#include "System/HQuestManager.h"
#include "Unit/Monster/HBaseMonster.h"
#include "Unit/Player/HPlayerCharacter.h"

void UHQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHQuestManager::ProcessMonsterDeath(AActor* InAttacker, AHBaseMonster* InDeadMonster)
{
	if (!InAttacker || !InDeadMonster) return;

	// 가해자가 플레이어인 경우 경험치 지급
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(InAttacker))
	{
		float RewardExp = InDeadMonster->GetExpReward();
		Player->AddExp(RewardExp);
		
		UE_LOG(LogTemp, Log, TEXT("QuestManager: Player rewarded with %f EXP for killing monster."), RewardExp);
	}

	// 여기에 퀘스트 카운트 증가 등 추가 로직 구현 가능
}
