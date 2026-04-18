// Fill out your copyright notice in the Description page of Project Settings.

#include "System/HWaveManager.h"
#include "DataAsset/HWaveConfigDataAsset.h"
#include "System/HMonsterSpawnManager.h"
#include "Unit/Player/HPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "System/HUIManager.h"
#include "UI/HWaveResultUI.h"

void UHWaveManager::InitializeWaveSystem(UHWaveConfigDataAsset* InConfig)
{
	if (!InConfig) return;

	WaveConfig = InConfig;
	CurrentWaveIndex = 0;
	CurrentState = EHWaveState::Ready;
}

void UHWaveManager::StartWave()
{
	if (!WaveConfig || !WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex)) return;
	if (CurrentState == EHWaveState::InProgress) return;

	CurrentState = EHWaveState::InProgress;
	const FHWaveData& WaveData = WaveConfig->WaveList[CurrentWaveIndex];

	// 초기화
	CurrentTimer = 0.0f;
	CurrentKillCount = 0;

	// 상점 웨이브인 경우 상점 UI 오픈 및 일시정지
	if (WaveData.WaveType == EHWaveType::Shop)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		if (GetWorld() && GetWorld()->GetGameInstance())
		{
			if (UHUIManager* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UHUIManager>())
			{
				UIManager->ShowWidgetByName(TEXT("ShopUI"));
			}
		}
	}

	// 몬스터 스폰 시작 (전투/보스인 경우)
	if (WaveData.WaveType == EHWaveType::Battle || WaveData.WaveType == EHWaveType::Boss)
	{
		if (UHMonsterSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UHMonsterSpawnManager>())
		{
			SpawnManager->StartMonsterWave(WaveData.MonsterSpawnerConfig);
		}
	}

	// 상태 체크 타이머 시작 (1초 간격)
	GetWorld()->GetTimerManager().SetTimer(WaveUpdateTimerHandle, this, &UHWaveManager::UpdateWaveProgress, 1.0f, true);

	// 이벤트 호출
	OnWaveStarted.Broadcast(WaveData.WaveIndex, WaveData.WaveType, WaveData.ClearType);
	OnWaveProgressUpdated.Broadcast(0.0f, 0.0f, WaveData.ClearValue);

	UE_LOG(LogTemp, Log, TEXT("Wave %d Started! Type: %d, Clear: %d"), WaveData.WaveIndex, (int32)WaveData.WaveType, (int32)WaveData.ClearType);
}

void UHWaveManager::UpdateWaveProgress()
{
	if (CurrentState != EHWaveState::InProgress || !WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex)) return;

	const FHWaveData& WaveData = WaveConfig->WaveList[CurrentWaveIndex];
	float Progress = 0.0f;
	float CurrentValue = 0.0f;

	switch (WaveData.ClearType)
	{
	case EHWaveClearType::TimeSurvival:
		CurrentTimer += 1.0f;
		CurrentValue = CurrentTimer;
		Progress = FMath::Clamp(CurrentValue / WaveData.ClearValue, 0.0f, 1.0f);
		if (CurrentTimer >= WaveData.ClearValue)
		{
			EndWave();
		}
		break;

	case EHWaveClearType::KillCount:
		CurrentValue = static_cast<float>(CurrentKillCount);
		Progress = FMath::Clamp(CurrentValue / WaveData.ClearValue, 0.0f, 1.0f);
		if (CurrentKillCount >= static_cast<int32>(WaveData.ClearValue))
		{
			EndWave();
		}
		break;

	case EHWaveClearType::ManualUI:
		// ManualUI는 별도의 버튼 클릭 등으로 EndWave()를 호출할 때까지 유지됨
		CurrentValue = 0.0f;
		Progress = 0.5f; 
		break;
	}

	OnWaveProgressUpdated.Broadcast(Progress, CurrentValue, WaveData.ClearValue);
}

void UHWaveManager::EndWave()
{
	if (CurrentState != EHWaveState::InProgress) return;

	GetWorld()->GetTimerManager().ClearTimer(WaveUpdateTimerHandle);
	CurrentState = EHWaveState::Completed;

	// 몬스터 스폰 중지
	if (UHMonsterSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UHMonsterSpawnManager>())
	{
		SpawnManager->StopMonsterWave();
	}

	// 게임 일시정지 (UI가 뜬 동안 멈춤)
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 이자 정산 및 UI 표시
	CalculateInterest();

	// 이벤트 호출
	if (WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex))
	{
		OnWaveCompleted.Broadcast(WaveConfig->WaveList[CurrentWaveIndex].WaveIndex);
		UE_LOG(LogTemp, Log, TEXT("Wave %d Completed!"), WaveConfig->WaveList[CurrentWaveIndex].WaveIndex);
	}
}

void UHWaveManager::PrepareNextWave()
{
	if (CurrentState != EHWaveState::Completed) return;

	CurrentWaveIndex++;
	CurrentState = EHWaveState::Ready;
}

void UHWaveManager::ReportMonsterDeath()
{
	if (CurrentState != EHWaveState::InProgress) return;

	CurrentKillCount++;
	
	// KillCount 조건인 경우 즉시 업데이트 체크
	if (WaveConfig && WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex))
	{
		if (WaveConfig->WaveList[CurrentWaveIndex].ClearType == EHWaveClearType::KillCount)
		{
			UpdateWaveProgress();
		}
	}
}

int32 UHWaveManager::GetCurrentWaveDisplayIndex() const
{
	if (WaveConfig && WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex))
	{
		return WaveConfig->WaveList[CurrentWaveIndex].WaveIndex;
	}
	return 0;
}

EHWaveType UHWaveManager::GetCurrentWaveType() const
{
	if (WaveConfig && WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex))
	{
		return WaveConfig->WaveList[CurrentWaveIndex].WaveType;
	}
	return EHWaveType::Battle;
}

EHWaveClearType UHWaveManager::GetCurrentWaveClearType() const
{
	if (WaveConfig && WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex))
	{
		return WaveConfig->WaveList[CurrentWaveIndex].ClearType;
	}
	return EHWaveClearType::TimeSurvival;
}

void UHWaveManager::GetCurrentWaveProgress(float& OutCurrent, float& OutTarget) const
{
	OutCurrent = 0.0f;
	OutTarget = 0.0f;

	if (!WaveConfig || !WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex)) return;

	const FHWaveData& WaveData = WaveConfig->WaveList[CurrentWaveIndex];
	OutTarget = WaveData.ClearValue;

	switch (WaveData.ClearType)
	{
	case EHWaveClearType::TimeSurvival:
		OutCurrent = CurrentTimer;
		break;
	case EHWaveClearType::KillCount:
		OutCurrent = static_cast<float>(CurrentKillCount);
		break;
	case EHWaveClearType::ManualUI:
		OutCurrent = 0.0f;
		break;
	}
}

const FHWaveData& UHWaveManager::GetCurrentWaveData() const
{
	if (WaveConfig && WaveConfig->WaveList.IsValidIndex(CurrentWaveIndex))
	{
		return WaveConfig->WaveList[CurrentWaveIndex];
	}
	return DefaultWaveData;
}

void UHWaveManager::CalculateInterest()
{
	if (!WaveConfig) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	AHPlayerState* PS = PC->GetPlayerState<AHPlayerState>();
	if (!PS) return;

	int32 CurrentGold = PS->GetCurrentGold();
	int32 EarnedInterest = FMath::Min(FMath::FloorToInt(static_cast<float>(CurrentGold) * WaveConfig->DefaultInterestRate), WaveConfig->MaxInterest);

	if (EarnedInterest > 0)
	{
		PS->AddGold(EarnedInterest);
		UE_LOG(LogTemp, Log, TEXT("Interest Earned: %d (Current Gold: %d)"), EarnedInterest, CurrentGold);
	}

	// 결과 UI 표시
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		if (UHUIManager* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UHUIManager>())
		{
			if (UHWaveResultUI* ResultUI = Cast<UHWaveResultUI>(UIManager->ShowWidgetByName(TEXT("WaveResultUI"))))
			{
				ResultUI->SetResultData(GetCurrentWaveDisplayIndex(), GetCurrentWaveType(), EarnedInterest, PS->GetCurrentGold());
			}
		}
	}
}
