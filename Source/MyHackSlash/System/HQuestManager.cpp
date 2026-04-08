#include "System/HQuestManager.h"
#include "Unit/Monster/HBaseMonster.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Unit/Player/HPlayerState.h"

void UHQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHQuestManager::ProcessMonsterDeath(AActor* InAttacker, AHBaseMonster* InDeadMonster)
{
	if (!InAttacker || !InDeadMonster) return;

	// 이제 경험치 지급은 GAS GameplayEvent를 통해 플레이어 ASC에서 직접 처리됩니다.
	// QuestManager에서는 퀘스트 처치 카운트 등 다른 로직만 수행합니다.

	UE_LOG(LogTemp, Log, TEXT("QuestManager: Processing monster death for quest tracking..."));

	// 여기에 퀘스트 카운트 증가 등 추가 로직 구현 가능
}
