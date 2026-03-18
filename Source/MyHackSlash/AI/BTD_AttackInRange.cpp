// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTD_AttackInRange.h"
#include "AIController.h"
#include "Unit/HBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTD_AttackInRange::UBTD_AttackInRange()
{
	NodeName = TEXT("CanAttack");
}

bool UBTD_AttackInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn) return false;

    AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
    if (nullptr == Character) return false;

	APawn* TargetPawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (TargetPawn)
	{
        // 실시간 공격 범위는 캐릭터의 스탯에서 가져옵니다.
		float AttackRange = Character->GetCurrentStat().AttackRange;
		float Distance = FVector::Distance(ControllingPawn->GetActorLocation(), TargetPawn->GetActorLocation());
		return Distance <= AttackRange;
	}

	return false;
}
