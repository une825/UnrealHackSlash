// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTT_FindPatrolPos.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"

UBTT_FindPatrolPos::UBTT_FindPatrolPos()
{
	NodeName = TEXT("HFindPatrolPos");
}

EBTNodeResult::Type UBTT_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (nullptr == ControllingPawn) return EBTNodeResult::Failed;

    AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
    if (nullptr == Character || nullptr == Character->GetUnitProfileData()) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());
    if (nullptr == NavSystem) return EBTNodeResult::Failed;

    FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("HomePos"));
    
    // DataAsset에서 직접 순찰 반경을 가져옵니다.
    float PatrolRadius = Character->GetUnitProfileData()->PatrolRadius;
    
    FNavLocation NextPatrolPos;
    if (NavSystem->GetRandomPointInNavigableRadius(Origin, PatrolRadius, NextPatrolPos))
    {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPos"), NextPatrolPos.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
