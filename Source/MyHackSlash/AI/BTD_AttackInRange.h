// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_AttackInRange.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UBTD_AttackInRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTD_AttackInRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

};
