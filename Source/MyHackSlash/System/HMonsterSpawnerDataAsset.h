// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HMonsterSpawnerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UHMonsterSpawnerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> MonsterClass;

    UPROPERTY(EditAnywhere)
    float SpawnInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Rules")
    int32 SpawnsPerTick = 1; // 한 번에 소환할 마릿수 (최적화용)

    UPROPERTY(EditAnywhere)
    float MinRadius = 500.f;

    UPROPERTY(EditAnywhere)
    float MaxRadius = 1500.f;
};
