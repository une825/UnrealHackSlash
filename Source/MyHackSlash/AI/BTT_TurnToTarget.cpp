// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTT_TurnToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"

namespace
{
	bool TurnPawnToTarget(APawn* InPawn, APawn* InTargetPawn, float InTurnSpeed, float InAcceptanceAngle, float InDeltaSeconds)
	{
		if (nullptr == InPawn || nullptr == InTargetPawn)
		{
			return false;
		}

		FVector Direction = InTargetPawn->GetActorLocation() - InPawn->GetActorLocation();
		Direction.Z = 0.0f;
		if (Direction.IsNearlyZero())
		{
			return true;
		}

		const FRotator TargetRotation(0.0f, Direction.Rotation().Yaw, 0.0f);
		const FRotator CurrentRotation(0.0f, InPawn->GetActorRotation().Yaw, 0.0f);
		const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));
		if (DeltaYaw <= InAcceptanceAngle)
		{
			InPawn->SetActorRotation(TargetRotation);
			if (AController* Controller = InPawn->GetController())
			{
				Controller->SetControlRotation(TargetRotation);
			}
			return true;
		}

		const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, InDeltaSeconds, InTurnSpeed);
		InPawn->SetActorRotation(NewRotation);
		if (AController* Controller = InPawn->GetController())
		{
			Controller->SetControlRotation(NewRotation);
		}

		return false;
	}
}

UBTT_TurnToTarget::UBTT_TurnToTarget()
{
	NodeName = TEXT("Turn");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (nullptr == BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	APawn* TargetPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (nullptr == TargetPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (BlackboardComp->GetValueAsBool(TEXT("IsAttacking")))
	{
		return EBTNodeResult::Succeeded;
	}

	AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
	if (nullptr == Character || nullptr == Character->GetUnitProfileData())
	{
		return EBTNodeResult::Failed;
	}

	const float TurnSpeed = Character->GetUnitProfileData()->TurnSpeed;
	AIController->StopMovement();

	if (TurnPawnToTarget(ControllingPawn, TargetPawn, TurnSpeed, TurnAcceptanceAngle, GetWorld()->GetDeltaSeconds()))
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTT_TurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (nullptr == AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* ControllingPawn = AIController->GetPawn();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (nullptr == ControllingPawn || nullptr == BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* TargetPawn = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (nullptr == TargetPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (BlackboardComp->GetValueAsBool(TEXT("IsAttacking")))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AHBaseCharacter* Character = Cast<AHBaseCharacter>(ControllingPawn);
	if (nullptr == Character || nullptr == Character->GetUnitProfileData())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const float TurnSpeed = Character->GetUnitProfileData()->TurnSpeed;
	if (TurnPawnToTarget(ControllingPawn, TargetPawn, TurnSpeed, TurnAcceptanceAngle, DeltaSeconds))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
