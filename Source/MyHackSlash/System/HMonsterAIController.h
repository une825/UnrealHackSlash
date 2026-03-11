// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HMonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API AHMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHMonsterAIController();

public:
	void RunAI();
	void StopAI();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UBlackboardData> BBAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UBehaviorTree> BTAsset;
};
