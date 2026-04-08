// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mode/MyHackSlashGameMode.h"
#include "MyHackSlashPlayerController.h"
#include "Unit/Player/HPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "DataAsset/HMonsterSpawnerDataAsset.h"
#include "System/HMonsterSpawnManager.h"
#include "System/HSelectAbilityManager.h"
#include "System/HInfiniteMapManager.h"
#include "System/HWaveManager.h"
#include <System/HSoundManager.h>
#include "DataAsset/HSelectAbilityData.h"
#include "DataAsset/HMapConfigDataAsset.h"
#include "DataAsset/HWaveConfigDataAsset.h"
#include "DataAsset/HSoundDataAsset.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

AMyHackSlashGameMode::AMyHackSlashGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AMyHackSlashPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Unit/Player/Blueprint/BP_PlayerCharacter_01"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Unit/Player/Blueprint/BP_PlayerController"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	PlayerStateClass = AHPlayerState::StaticClass();
}

void AMyHackSlashGameMode::BeginPlay()
{
	Super::BeginPlay();

	// SetMonsterSpawnManager(); // 이제 WaveManager가 담당하므로 직접 호출하지 않음
	SetWaveManager();
	SetSelectAbilityManager();

	// Infinite Map 초기화 (DataAsset 사용)
	if (MapConfig)
	{
		if (UHInfiniteMapManager* MapManager = GetWorld()->GetSubsystem<UHInfiniteMapManager>())
		{
			MapManager->SetMapSettings(
				MapConfig->MapTileClass, 
				MapConfig->MapTileSize, 
				MapConfig->MapViewDistance, 
				MapConfig->MapPropPool, 
				MapConfig->MinPropCount, 
				MapConfig->MaxPropCount
			);
		}
	}

	// BGM 시작
	if (SoundConfig)
	{
		if (UHSoundManager* SoundManager = GetWorld()->GetSubsystem<UHSoundManager>())
		{
			SoundManager->InitializeManager(SoundConfig);
			SoundManager->PlayBGMByKey(TEXT("BGM_01"));
		}
	}
}

void AMyHackSlashGameMode::Tick(float InDeltaSeconds)
{
}

void AMyHackSlashGameMode::SetMonsterSpawnManager()
{
	// 현재는 WaveManager가 StartMonsterWave를 직접 호출하므로 이 함수는 사용되지 않거나
	// 초기 설정이 필요한 경우에만 사용합니다.
}

void AMyHackSlashGameMode::SetWaveManager()
{
	if (WaveConfig)
	{
		if (UHWaveManager* WaveManager = GetWorld()->GetSubsystem<UHWaveManager>())
		{
			WaveManager->InitializeWaveSystem(WaveConfig);
			WaveManager->StartWave();
		}
	}
}

void AMyHackSlashGameMode::SetSelectAbilityManager()
{
	if (UHSelectAbilityManager* SelectAbilityManager = GetGameInstance()->GetSubsystem<UHSelectAbilityManager>())
	{
		SelectAbilityManager->InitializeManager(SelectAbilityGradeDataAsset, SelectAbilityRewardTable, GemCollectionDataAsset);
	}
}
