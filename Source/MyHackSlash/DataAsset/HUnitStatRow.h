// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "HUnitStatRow.generated.h"

/**
 * 모든 유닛이 공통으로 가지는 기본 스탯 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FUnitStatRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FUnitStatRow()
		: MaxHP(100.0f)
		, AttackDamage(30.0f)
		, AttackRange(40.0f)
		, AttackSpeedRate(1.0f)
		, MovementSpeed(400.0f)
		, CriticalRate(5.0f)
		, CriticalMultiplier(1.5f)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackSpeedRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MovementSpeed;

	/** @brief 치명타 확률 (0~100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float CriticalRate;

	/** @brief 치명타 데미지 배율 (예: 1.5 = 150%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float CriticalMultiplier;
};
