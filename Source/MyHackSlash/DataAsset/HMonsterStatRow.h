// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "HMonsterStatRow.generated.h"

/**
 * DataTable의 각 행에 해당하는 몬스터 스탯 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FMonsterStatRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMonsterStatRow()
		: MaxHP(100.0f)
		, AttackDamage(30.0f)
		, AttackRange(40.0f)
		, AttackSpeedRate(1.0f)
		, MovementSpeed(400.0f)
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
};
