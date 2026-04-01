// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/HBaseCharacter.h"
#include "AI/HMonsterAIInerface.h"
#include "HBaseMonster.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterDead, AActor*, InAttacker, AHBaseMonster*, InDeadMonster);

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
	// 사망 시 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMonsterDead OnMonsterDead;

	float GetExpReward() const;

public:
	// 스탯 및 초기화
	virtual void InitializeStat(int32 InNewLevel) override;

protected:
	virtual void SetDead() override;
	
	void ReturnToPool();

protected:
	virtual void PostInitializeComponents() override;

protected:
	virtual void SetAIAttackDelegate(const FAIMonsterAttackFinished& InOnAttackFinished) override;
	virtual void AttackByAI() override;
	virtual void NotifyAttackEnd() override;

	FAIMonsterAttackFinished OnAttackFinished;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<class AHCoin> CoinClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	FMonsterStatRow CurrentMonsterStat;
};
