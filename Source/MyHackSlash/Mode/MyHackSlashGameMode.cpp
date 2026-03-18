// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mode/MyHackSlashGameMode.h"
#include "MyHackSlashPlayerController.h"
#include "Unit/Player/HPlayerState.h"
#include "UObject/ConstructorHelpers.h"
#include "System/HMonsterSpawnerDataAsset.h"
#include <System/HMonsterSpawnManager.h>
#include <Engine/StreamableManager.h>
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

	SetMonsterSpawnManager();
}

void AMyHackSlashGameMode::Tick(float InDeltaSeconds)
{
}

void AMyHackSlashGameMode::SetMonsterSpawnManager()
{
	// 1. 변수 자체가 유효한지 먼저 체크
	if (MonsterSpawnConfigPtr.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("MonsterSpawnConfigPtr 가 할당되지 않았습니다!"));
		return;
	}

	// 2. 경로 추출 (ToSoftObjectPath 사용)
	FSoftObjectPath AssetPath = MonsterSpawnConfigPtr.ToSoftObjectPath();
	if (!AssetPath.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AssetPath 가 유효하지 않습니다!"));
		return;
	}

	if (MonsterSpawnConfigPtr.IsValid())
	{
		// 이미 로드되어 있다면 바로 사용
		if (UHMonsterSpawnManager* MonsterSpawnManager = GetWorld()->GetSubsystem<UHMonsterSpawnManager>())
		{
			MonsterSpawnManager->StartMonsterWave(MonsterSpawnConfigPtr.Get());
		}
	}
	else
	{
		// 비동기 로드 시작
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		LoadHandle = Streamable.RequestAsyncLoad(MonsterSpawnConfigPtr.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this]()
			{
				UE_LOG(LogTemp, Log, TEXT("Loading Complete!"));

				if (UHMonsterSpawnerDataAsset* LoadedConfig = MonsterSpawnConfigPtr.Get())
				{
					if (UHMonsterSpawnManager* MonsterSpawnManager = GetWorld()->GetSubsystem<UHMonsterSpawnManager>())
					{
						MonsterSpawnManager->StartMonsterWave(LoadedConfig);
					}
				}
			}));
	}
}
