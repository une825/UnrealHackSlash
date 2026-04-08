// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HGlobalTypes.h"
#include "HWaveConfigDataAsset.generated.h"

/**
 * @brief 전체 웨이브 시퀀스를 정의하는 데이터 에셋입니다.
 */
UCLASS()
class MYHACKSLASH_API UHWaveConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 웨이브 리스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Wave")
	TArray<FHWaveData> WaveList;

	/** @brief 기본 이자율 (0.1 = 10%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Economy")
	float DefaultInterestRate = 0.1f;

	/** @brief 웨이브 클리어 시 최대 이자 한도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "H|Economy")
	int32 MaxInterest = 50;
};
