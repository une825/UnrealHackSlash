// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTS_Detect.h"

#include "AIController.h"
#include "AI/HMonsterAIInerface.h"
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
    if (nullptr == ControllingPawn)
    {
        return;
    }

    FVector Center = ControllingPawn->GetActorLocation();
    UWorld* World = ControllingPawn->GetWorld();
    if (nullptr == World)
    {
        return;
    }

    IHMonsterAIInerface* AIPawn = Cast<IHMonsterAIInerface>(ControllingPawn);
    if (nullptr == AIPawn)
    {
        return;
    }

    float DetectRadius = AIPawn->GetAIDetectRange();

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
            APawn* Pawn = Cast<APawn>(OverlapResult.GetActor());
            if (Pawn && Pawn->GetController()->IsPlayerController())
            {
                OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Player"), Pawn);
                DrawDebugSphere(World, Center, DetectRadius, 16, FColor::Green, false, 0.2f);

                DrawDebugPoint(World, Pawn->GetActorLocation(), 10.0f, FColor::Green, false, 0.2f);
                DrawDebugLine(World, ControllingPawn->GetActorLocation(), Pawn->GetActorLocation(), FColor::Green, false, 0.27f);
                return;
            }
        }
    }

    OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Player"), nullptr);
    DrawDebugSphere(World, Center, DetectRadius, 16, FColor::Red, false, 0.2f);
}