// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HMonsterSpawnManager.generated.h"

class UHMonsterSpawnerDataAsset;

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UHMonsterSpawnManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    // 게임 모드나 UI에서 웨이브를 시작할 때 호출
    UFUNCTION(BlueprintCallable, Category = "MonsterSpawn")
    void StartMonsterWave(UHMonsterSpawnerDataAsset* Config);

    UFUNCTION(BlueprintCallable, Category = "MonsterSpawn")
    void StopMonsterWave();

private:
    void ExecuteSpawnTick();
    bool GetValidSpawnLocation(FVector& OutLocation);

    UPROPERTY()
    UHMonsterSpawnerDataAsset* CurrentConfig;

    FTimerHandle SpawnTimerHandle;
};
