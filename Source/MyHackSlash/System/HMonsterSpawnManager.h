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
    // ���� ��峪 UI���� ���̺긦 ������ �� ȣ��
    // 몬스터 웨이브 시작
    UFUNCTION(BlueprintCallable, Category = "MonsterSpawn")
    void StartMonsterWave(UHMonsterSpawnerDataAsset* InConfig);

    UFUNCTION(BlueprintCallable, Category = "MonsterSpawn")
    void StopMonsterWave();

private:
    UFUNCTION()
    void OnMonsterDied(AActor* InAttacker, class AHBaseMonster* InDeadMonster);

    void ExecuteSpawnTick();
    bool GetValidSpawnLocation(FVector& OutLocation);

    UPROPERTY()
    UHMonsterSpawnerDataAsset* CurrentConfig;

    FTimerHandle SpawnTimerHandle;
};
