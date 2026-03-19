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
        for (const auto& OverlapResult : OverlapResults)
        {
            APawn* TargetPawn = Cast<APawn>(OverlapResult.GetActor());
            if (TargetPawn && TargetPawn->GetController() && TargetPawn->GetController()->IsPlayerController())
            {
                OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), TargetPawn);
                return;
            }
        }
    }

    OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);
}
