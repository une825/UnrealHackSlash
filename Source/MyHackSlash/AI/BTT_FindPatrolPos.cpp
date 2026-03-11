// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_FindPatrolPos.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/HMonsterAIInerface.h"

UBTT_FindPatrolPos::UBTT_FindPatrolPos()
{
	NodeName = TEXT("HFindPatrolPos");
}

EBTNodeResult::Type UBTT_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
    if (Result == EBTNodeResult::Failed)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (nullptr == ControllingPawn)
    {
        return EBTNodeResult::Failed;
    }

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());
    if (nullptr == NavSystem)
    {
        return EBTNodeResult::Failed;
    }

    IHMonsterAIInerface* AIPawn = Cast<IHMonsterAIInerface>(ControllingPawn);
    if (nullptr == AIPawn)
    {
        return EBTNodeResult::Failed;
    }

    FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("HomePos"));
    float PatrolRadius = AIPawn->GetAIPatrolRadius();
    FNavLocation NextPatrolPos;

    if (NavSystem->GetRandomPointInNavigableRadius(Origin, PatrolRadius, NextPatrolPos))
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPos"), NextPatrolPos.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}