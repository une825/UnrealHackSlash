// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyHackSlashGameMode.generated.h"

class UHMonsterSpawnerDataAsset;
class UHSelectAbilityGradeDataAsset;
class UHGemDataAsset;
class UHSoundDataAsset;
class UDataTable;
struct FMapPropData;

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

	/** @brief Wave 시스템을 초기화합니다. */
	void SetWaveManager();

private:
	UPROPERTY(EditAnywhere, Category = "Wave")
	TObjectPtr<class UHWaveConfigDataAsset> WaveConfig;

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

	// --- Infinite Map Settings ---
	UPROPERTY(EditAnywhere, Category = "Map")
	TObjectPtr<class UHMapConfigDataAsset> MapConfig;

	// --- Sound Settings ---
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<UHSoundDataAsset> SoundConfig;

	TSharedPtr<struct FStreamableHandle> LoadHandle;
};
