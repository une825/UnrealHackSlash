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
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (nullptr == AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControllingPawn = AIController->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 디버그 로그 추가
	UE_LOG(LogTemp, Log, TEXT("[BTT_Attack] ExecuteTask Started for %s"), *ControllingPawn->GetName());

	// 블랙보드에 공격 중임을 기록
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(TEXT("IsAttacking"), true);
	}

	// 공격 시작 시 AI 이동을 즉시 중단
	AIController->StopMovement();

	IHMonsterAIInerface* AIPawn = Cast<IHMonsterAIInerface>(ControllingPawn);
	if (nullptr == AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	FAIMonsterAttackFinished OnAttackFinished;
	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp(&OwnerComp);
	OnAttackFinished.BindLambda([this, WeakOwnerComp, ControllingPawn]()
	{
		if (WeakOwnerComp.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[BTT_Attack] Attack Finished Delegate Called for %s"), *ControllingPawn->GetName());
			
			// 블랙보드 공격 상태 해제
			if (UBlackboardComponent* BBComp = WeakOwnerComp->GetBlackboardComponent())
			{
				BBComp->SetValueAsBool(TEXT("IsAttacking"), false);
			}

			FinishLatentTask(*WeakOwnerComp, EBTNodeResult::Succeeded);
		}
	});
	AIPawn->SetAIAttackDelegate(OnAttackFinished);

	AIPawn->AttackByAI();
	return EBTNodeResult::InProgress;
}
