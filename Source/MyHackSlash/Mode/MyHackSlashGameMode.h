// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyHackSlashGameMode.generated.h"

class UHMonsterSpawnerDataAsset;
class UHSelectAbilityGradeDataAsset;
class UHGemDataAsset;
class UDataTable;

UCLASS(minimalapi)
class AMyHackSlashGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyHackSlashGameMode();

	UHGemDataAsset* GetGemCollectionDataAsset() const { return GemCollectionDataAsset; }

public:
	virtual void BeginPlay() override;
	virtual void Tick(float InDeltaSeconds) override;

private:
	void SetMonsterSpawnManager();

	/** @brief Select Ability 시스템을 초기화합니다. */
	void SetSelectAbilityManager();

private:
	UPROPERTY(EditAnywhere, Category = "Wave")
	TSoftObjectPtr<UHMonsterSpawnerDataAsset> MonsterSpawnConfigPtr;

	/** @brief Select Ability 등급별 확률 데이터 에셋 */
	UPROPERTY(EditAnywhere, Category = "Select Ability")
	TObjectPtr<UHSelectAbilityGradeDataAsset> SelectAbilityGradeDataAsset;

	/** @brief Select Ability 보상 데이터 테이블 */
	UPROPERTY(EditAnywhere, Category = "Select Ability")
	TObjectPtr<UDataTable> SelectAbilityRewardTable;

	/** @brief 스킬 젬 데이터 컬렉션 에셋 */
	UPROPERTY(EditAnywhere, Category = "Select Ability")
	TObjectPtr<UHGemDataAsset> GemCollectionDataAsset;

	TSharedPtr<struct FStreamableHandle> LoadHandle;
};



