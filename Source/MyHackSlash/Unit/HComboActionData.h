// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HComboActionData.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UHComboActionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UHComboActionData();

public:
	UPROPERTY(EditAnywhere, Category = Name)
	FString MontageSectionNamePrefix;

	UPROPERTY(EditAnywhere, Category = Name)
	uint8 MaxComboCount = 0;

	UPROPERTY(EditAnywhere, Category = Name)
	float FrameRate = 0.0f;

	UPROPERTY(EditAnywhere, Category = Name)
	TArray<float> EffectiveFrameCount = {};

};
