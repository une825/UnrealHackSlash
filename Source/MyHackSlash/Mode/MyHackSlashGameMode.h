// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyHackSlashGameMode.generated.h"

class UHMonsterSpawnerDataAsset;

UCLASS(minimalapi)
class AMyHackSlashGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyHackSlashGameMode();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float InDeltaSeconds) override;

private:
	void SetMonsterSpawnManager();

private:
	UPROPERTY(EditAnywhere, Category = "Wave")
	TSoftObjectPtr<UHMonsterSpawnerDataAsset> MonsterSpawnConfigPtr;

	TSharedPtr<struct FStreamableHandle> LoadHandle;
};



