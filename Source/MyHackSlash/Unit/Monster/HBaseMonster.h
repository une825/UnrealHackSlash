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

public:
	// 스탯 및 초기화
	virtual void InitializeStat(int32 NewLevel) override;

protected:
	virtual void SetDead() override;
	
	void EnableRagdoll();
	void ReturnToPool();

protected:
	virtual void PostInitializeComponents() override;

protected:
	virtual void SetAIAttackDelegate(const FAIMonsterAttackFinished& InOnAttackFinished) override;
	virtual void AttackByAI() override;
	virtual void NotifyAttackEnd() override;

	FAIMonsterAttackFinished OnAttackFinished;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	FMonsterStatRow CurrentMonsterStat;
};
