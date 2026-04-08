#include "System/HMonsterSpawnManager.h"
#include "DataAsset/HMonsterSpawnerDataAsset.h"
#include "HObjectPoolManager.h"
#include "System/HQuestManager.h"
#include "System/HWaveManager.h"
#include "Unit/Monster/HBaseMonster.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

void UHMonsterSpawnManager::StartMonsterWave(UHMonsterSpawnerDataAsset* InConfig)
{
    if (!InConfig) return;

    CurrentConfig = InConfig;

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
    if (!CurrentConfig || !CurrentConfig->MonsterClass) return;

    UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
    if (!Pool) return;

    // 스폰할 몬스터의 기본 높이값 가져오기
    float SpawnZOffset = 0.0f;
    if (const ACharacter* DefaultChar = Cast<ACharacter>(CurrentConfig->MonsterClass->GetDefaultObject()))
    {
        if (UCapsuleComponent* Capsule = DefaultChar->GetCapsuleComponent())
        {
            SpawnZOffset = Capsule->GetScaledCapsuleHalfHeight();
        }
    }

    for (int32 i = 0; i < CurrentConfig->SpawnsPerTick; ++i)
    {
        FVector SpawnPos;
        if (GetValidSpawnLocation(SpawnPos))
        {
            // 실제 몬스터의 캡슐 높이만큼 위로 올려서 정확히 지면에 발이 닿게 설정
            SpawnPos.Z += SpawnZOffset;

            AActor* SpawnedActor = Pool->SpawnFromPool(CurrentConfig->MonsterClass, SpawnPos, FRotator::ZeroRotator);

            if (AHBaseMonster* Monster = Cast<AHBaseMonster>(SpawnedActor))
            {
                // 중복 바인딩 방지를 위해 먼저 제거 후 추가 (오브젝트 풀링 대응)
                Monster->OnMonsterDead.RemoveDynamic(this, &UHMonsterSpawnManager::OnMonsterDied);
                Monster->OnMonsterDead.AddDynamic(this, &UHMonsterSpawnManager::OnMonsterDied);
            }
        }
    }
}

void UHMonsterSpawnManager::OnMonsterDied(AActor* InAttacker, AHBaseMonster* InDeadMonster)
{
    // 1. 퀘스트 매니저 알림
    if (UHQuestManager* QuestManager = GetWorld()->GetGameInstance()->GetSubsystem<UHQuestManager>())
    {
        QuestManager->ProcessMonsterDeath(InAttacker, InDeadMonster);
    }

    // 2. 웨이브 매니저 알림
    if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
    {
        WaveManager->ReportMonsterDeath();
    }
}


bool UHMonsterSpawnManager::GetValidSpawnLocation(FVector& OutLocation)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Player || !CurrentConfig) return false;

    FVector Origin = Player->GetActorLocation();

    // ���� ���� ���� ��ǥ ���
    FVector RandomDir = FMath::VRand();
    RandomDir.Z = 0;
    RandomDir.Normalize();

    float Dist = FMath::FRandRange(CurrentConfig->MinRadius, CurrentConfig->MaxRadius);
    FVector TestPos = Origin + (RandomDir * Dist);

    // NavMesh ���� �ִ��� Ȯ��
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