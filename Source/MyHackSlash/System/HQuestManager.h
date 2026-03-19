#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HQuestManager.generated.h"

class AHBaseMonster;

/**
 * 퀘스트 진행 및 몬스터 처치 보상을 통합 관리하는 시스템입니다.
 */
UCLASS()
class MYHACKSLASH_API UHQuestManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 몬스터 사망 시 호출되어 보상 및 퀘스트 처리를 진행합니다.
	UFUNCTION()
	void ProcessMonsterDeath(AActor* InAttacker, AHBaseMonster* InDeadMonster);

};
