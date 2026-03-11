// Fill out your copyright notice in the Description page of Project Settings.


#include "System/HMonsterSpawnManager.h"
#include "HMonsterSpawnerDataAsset.h"
#include "HObjectPoolManager.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

void UHMonsterSpawnManager::StartMonsterWave(UHMonsterSpawnerDataAsset* Config)
{
    if (!Config) return;

    CurrentConfig = Config;

    // 기존 타이머가 있다면 초기화 후 재시작
    GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &UHMonsterSpawnManager::ExecuteSpawnTick, CurrentConfig->SpawnInterval, true);
}

void UHMonsterSpawnManager::StopMonsterWave()
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    CurrentConfig = nullptr;
}

void UHMonsterSpawnManager::ExecuteSpawnTick()
{
    if (!CurrentConfig) return;

    UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
    if (!Pool) return;

    for (int32 i = 0; i < CurrentConfig->SpawnsPerTick; ++i)
    {
        FVector SpawnPos;
        if (GetValidSpawnLocation(SpawnPos))
        {
            Pool->SpawnFromPool(CurrentConfig->MonsterClass, SpawnPos, FRotator::ZeroRotator);
        }
    }
}

bool UHMonsterSpawnManager::GetValidSpawnLocation(FVector& OutLocation)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player || !CurrentConfig) return false;

    FVector Origin = Player->GetActorLocation();

    // 도넛 형태 랜덤 좌표 계산
    FVector RandomDir = FMath::VRand();
    RandomDir.Z = 0;
    RandomDir.Normalize();

    float Dist = FMath::FRandRange(CurrentConfig->MinRadius, CurrentConfig->MaxRadius);
    FVector TestPos = Origin + (RandomDir * Dist);

    // NavMesh 위에 있는지 확인
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLoc;
        if (NavSys->ProjectPointToNavigation(TestPos, NavLoc, FVector(500.f, 500.f, 500.f)))
        {
            OutLocation = NavLoc.Location;
            return true;
        }
    }
    return false;
}