// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HMonsterSpawnManager.generated.h"

class UHMonsterSpawnerDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterSpawned, class AHBaseMonster*, Monster);

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API UHMonsterSpawnManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    // ... 기존 코드
    UFUNCTION(BlueprintCallable, Category = "MonsterSpawn")
    void StartMonsterWave(UHMonsterSpawnerDataAsset* InConfig);

    UFUNCTION(BlueprintCallable, Category = "MonsterSpawn")
    void StopMonsterWave();

public:
    UPROPERTY(BlueprintAssignable, Category = "MonsterSpawn")
    FOnMonsterSpawned OnMonsterSpawned;

private:

    UFUNCTION()
    void OnMonsterDied(AActor* InAttacker, class AHBaseMonster* InDeadMonster);

    void ExecuteSpawnTick();
    bool GetValidSpawnLocation(FVector& OutLocation);

    UPROPERTY()
    UHMonsterSpawnerDataAsset* CurrentConfig;

    FTimerHandle SpawnTimerHandle;
};
