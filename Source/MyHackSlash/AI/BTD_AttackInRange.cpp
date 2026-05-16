// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTD_AttackInRange.h"
#include "AIController.h"
#include "Unit/HBaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

UBTD_AttackInRange::UBTD_AttackInRange()
{
	NodeName = TEXT("CanAttack");
}

bool UBTD_AttackInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (nullptr == AIController) return false;

	APawn* ControllingPawn = AIController->GetPawn();
	if (nullptr == ControllingPawn) return false;

    AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
    if (nullptr == Character) return false;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (nullptr == BlackboardComp) return false;

	APawn* TargetPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (TargetPawn)
	{
		float AttackRadius = 0.0f;
		if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
		{
			if (const UHCharacterAttributeSet* AttributeSet = ASC->GetSet<UHCharacterAttributeSet>())
			{
				AttackRadius = AttributeSet->GetAttackRadius();
			}
		}

		float SourceCapsuleRadius = 0.0f;
		if (const ACharacter* SourceCharacter = Cast<ACharacter>(ControllingPawn))
		{
			SourceCapsuleRadius = SourceCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		}

		float TargetCapsuleRadius = 0.0f;
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn))
		{
			TargetCapsuleRadius = TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		}

		const float AttackRange = Character->GetAttackRange();
		const float EffectiveAttackDistance = AttackRange + AttackRadius + SourceCapsuleRadius + TargetCapsuleRadius;
		const float Distance2D = FVector::Dist2D(ControllingPawn->GetActorLocation(), TargetPawn->GetActorLocation());
		return Distance2D <= EffectiveAttackDistance;
	}

	return false;
}
