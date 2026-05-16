// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTS_Detect.h"
#include "AIController.h"
#include "Unit/HBaseCharacter.h" // 직접 유닛 클래스 포함
#include "DataAsset/HUnitProfileData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

UBTS_Detect::UBTS_Detect()
{
    NodeName = TEXT("HDetect");
    Interval = 1.0f;
}

void UBTS_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (nullptr == ControllingPawn) return;

    AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
    if (nullptr == Character) return;

    const UHUnitProfileData* Profile = Character->GetUnitProfileData();
    if (nullptr == Profile) return;

    // DataAsset에서 직접 탐색 범위를 가져옵니다.
    float DetectRadius = Profile->DetectRadius;

    FVector Center = ControllingPawn->GetActorLocation();
    UWorld* World = ControllingPawn->GetWorld();
    if (nullptr == World) return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (nullptr == BlackboardComp) return;

    APawn* CurrentTarget = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Target")));
    if (CurrentTarget && CurrentTarget->GetController() && CurrentTarget->GetController()->IsPlayerController())
    {
        const float CurrentDistanceSq = FVector::DistSquared(Center, CurrentTarget->GetActorLocation());
        if (CurrentDistanceSq <= FMath::Square(DetectRadius))
        {
            UE_LOG(LogTemp, Verbose, TEXT("[MonsterAI] Detect keep target. Pawn=%s Target=%s Dist=%.1f Radius=%.1f"),
                *GetNameSafe(ControllingPawn),
                *GetNameSafe(CurrentTarget),
                FMath::Sqrt(CurrentDistanceSq),
                DetectRadius);
            return;
        }
    }

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionQueryParam(SCENE_QUERY_STAT(Detect), false, ControllingPawn);
    bool bResult = World->OverlapMultiByChannel(
        OverlapResults,
        Center,
        FQuat::Identity,
        ECC_GameTraceChannel1,
        FCollisionShape::MakeSphere(DetectRadius),
        CollisionQueryParam
    );

    if (bResult)
    {
        APawn* BestTarget = nullptr;
        float BestDistanceSq = TNumericLimits<float>::Max();

        for (const auto& OverlapResult : OverlapResults)
        {
            APawn* TargetPawn = Cast<APawn>(OverlapResult.GetActor());
            if (TargetPawn && TargetPawn->GetController() && TargetPawn->GetController()->IsPlayerController())
            {
                const float TargetDistanceSq = FVector::DistSquared(Center, TargetPawn->GetActorLocation());
                if (TargetDistanceSq < BestDistanceSq)
                {
                    BestDistanceSq = TargetDistanceSq;
                    BestTarget = TargetPawn;
                }
            }
        }

        if (BestTarget)
        {
            UE_LOG(LogTemp, Log, TEXT("[MonsterAI] Detect set target. Pawn=%s Target=%s Dist=%.1f Radius=%.1f"),
                *GetNameSafe(ControllingPawn),
                *GetNameSafe(BestTarget),
                FMath::Sqrt(BestDistanceSq),
                DetectRadius);
            BlackboardComp->SetValueAsObject(TEXT("Target"), BestTarget);
            return;
        }
    }

    if (CurrentTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("[MonsterAI] Detect clear target. Pawn=%s OldTarget=%s Radius=%.1f"),
            *GetNameSafe(ControllingPawn),
            *GetNameSafe(CurrentTarget),
            DetectRadius);
    }

    BlackboardComp->SetValueAsObject(TEXT("Target"), nullptr);
}
