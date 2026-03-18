// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/HUnitStatRow.h"
#include "HPlayerStatRow.generated.h"

/**
 * 플레이어 전용 스탯 구조체입니다. (기본 스탯 + 필요 경험치)
 */
USTRUCT(BlueprintType)
struct FPlayerStatRow : public FUnitStatRow
{
	GENERATED_BODY()

public:
	FPlayerStatRow()
		: FUnitStatRow()
		, MaxExp(100.0f)
	{}

	// 다음 레벨로 가기 위한 필요 경험치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxExp;
};
