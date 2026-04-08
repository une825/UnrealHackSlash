// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HMonsterDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UHMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = Monster, meta = (AssetBundles = "Monster"))
    TMap<FName, TSoftClassPtr<class AHBaseMonster>> MonsterAssets;
};
