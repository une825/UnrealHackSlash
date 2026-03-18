// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_Attack.h"
#include "AIController.h"
#include "HMonsterAIInerface.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_Attack::UBTT_Attack()
{
}

EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}

	IHMonsterAIInerface* AIPawn = Cast<IHMonsterAIInerface>(ControllingPawn);
	if (nullptr == AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	FAIMonsterAttackFinished OnAttackFinished;
	OnAttackFinished.BindLambda([&]()
	{
		//OwnerComp.GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	});
	AIPawn->SetAIAttackDelegate(OnAttackFinished);

	AIPawn->AttackByAI();
	return EBTNodeResult::InProgress;
}
