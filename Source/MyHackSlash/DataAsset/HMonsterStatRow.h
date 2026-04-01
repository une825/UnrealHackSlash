// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/HUnitStatRow.h"
#include "HMonsterStatRow.generated.h"

/**
 * 몬스터 전용 스탯 구조체입니다. (기본 스탯 + 처치 경험치 보상)
 */
USTRUCT(BlueprintType)
struct FMonsterStatRow : public FUnitStatRow
{
	GENERATED_BODY()

public:
	FMonsterStatRow()
		: FUnitStatRow()
		, ExpReward(50.0f)
		, GoldReward(10)
	{}

	// 처치 시 플레이어에게 지급할 경험치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float ExpReward;

	// 처치 시 드랍할 골드 양
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	int32 GoldReward;
};
