// Fill out your copyright notice in the Description page of Project Settings.


#include "System/HMonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AHMonsterAIController::AHMonsterAIController()
{
}

void AHMonsterAIController::RunAI()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !BBAsset || !BTAsset)
	{
		return;
	}

	UBlackboardComponent* BlackboardPtr = Blackboard.Get();
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		StopAI();

		Blackboard->SetValueAsVector(TEXT("HomePos"), ControlledPawn->GetActorLocation());
		Blackboard->ClearValue(TEXT("Target"));
		Blackboard->ClearValue(TEXT("PatrolPos"));

		bool RunResult = RunBehaviorTree(BTAsset);
		ensure(RunResult);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterAI] UseBlackboard failed. Controller=%s Pawn=%s BB=%s"),
			*GetNameSafe(this),
			*GetNameSafe(ControlledPawn),
			*GetNameSafe(BBAsset));
	}
}

void AHMonsterAIController::StopAI()
{
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}

void AHMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunAI();
}
