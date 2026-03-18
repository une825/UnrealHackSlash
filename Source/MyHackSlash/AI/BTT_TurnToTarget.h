// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_TurnToTarget.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UBTT_TurnToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_TurnToTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
