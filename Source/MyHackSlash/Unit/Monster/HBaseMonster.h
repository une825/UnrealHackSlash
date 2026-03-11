// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/HBaseCharacter.h"
#include "AI/HMonsterAIInerface.h"
#include "HBaseMonster.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MYHACKSLASH_API AHBaseMonster : public AHBaseCharacter, public IHMonsterAIInerface
{
	GENERATED_BODY()
public:
	AHBaseMonster();

protected:
	virtual void SetDead() override;
	
	void EnableRagdoll();

protected:
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;

	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual void AttackByAI() override;

	FAICharacterAttackFinished OnAttackFinished;

};
