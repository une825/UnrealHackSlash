// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTT_TurnToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"

UBTT_TurnToTarget::UBTT_TurnToTarget()
{
	NodeName = TEXT("Turn");
}

EBTNodeResult::Type UBTT_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}

	APawn* TargetPawn = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));
	if (nullptr == TargetPawn)
	{
		return EBTNodeResult::Failed;
	}

    AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
    if (nullptr == Character || nullptr == Character->GetUnitProfileData())
    {
        return EBTNodeResult::Failed;
    }

	const float TurnSpeed = Character->GetUnitProfileData()->TurnSpeed;
	FVector Direction = TargetPawn->GetActorLocation() - ControllingPawn->GetActorLocation();
	Direction.Z = 0.0f; // 높이 차이는 무시

	FRotator TargetRotation = Direction.Rotation();
	ControllingPawn->GetController()->SetControlRotation(FMath::RInterpTo(ControllingPawn->GetController()->GetControlRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), TurnSpeed));

	return EBTNodeResult::Succeeded;
}
