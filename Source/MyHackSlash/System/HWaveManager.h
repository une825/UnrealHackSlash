// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "System/HGlobalTypes.h"
#include "HWaveManager.generated.h"

class UHWaveConfigDataAsset;

/** @brief 웨이브 시작 시 호출되는 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWaveStarted, int32, WaveIndex, EHWaveType, WaveType, EHWaveClearType, ClearType);
/** @brief 웨이브 진행률 업데이트 시 호출되는 델리게이트 (진행률 %, 현재 값, 목표 값) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWaveProgressUpdated, float, ProgressPercent, float, CurrentValue, float, TargetValue);
/** @brief 웨이브 완료 시 호출되는 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveIndex);

/**
 * @brief 게임의 웨이브 흐름과 경제 시스템(이자)을 총괄하는 매니저 서브시스템입니다.
 */
UCLASS()
class MYHACKSLASH_API UHWaveManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** @brief 웨이브 설정을 초기화하고 첫 웨이브를 준비합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Wave")
	void InitializeWaveSystem(UHWaveConfigDataAsset* InConfig);

	/** @brief 현재 설정된 인덱스의 웨이브를 시작합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Wave")
	void StartWave();

	/** @brief 현재 웨이브를 강제로 종료하거나 클리어 처리합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Wave")
	void EndWave();

	/** @brief 다음 웨이브로 인덱스를 넘깁니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Wave")
	void PrepareNextWave();

	/** @brief 몬스터 처치 시 호출되어 KillCount 조건을 체크합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Wave")
	void ReportMonsterDeath();

	/** @brief 현재 웨이브가 진행 중인지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "H|Wave")
	bool IsWaveInProgress() const { return CurrentState == EHWaveState::InProgress; }

	/** @brief 현재 웨이브의 표시용 인덱스(1부터 시작)를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "H|Wave")
	int32 GetCurrentWaveDisplayIndex() const;

	/** @brief 현재 웨이브의 타입을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "H|Wave")
	EHWaveType GetCurrentWaveType() const;

	/** @brief 현재 웨이브의 클리어 조건을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "H|Wave")
	EHWaveClearType GetCurrentWaveClearType() const;

	/** @brief 현재 웨이브의 진행 수치들을 반환합니다. (Current, Target) */
	UFUNCTION(BlueprintPure, Category = "H|Wave")
	void GetCurrentWaveProgress(float& OutCurrent, float& OutTarget) const;

	/** @brief 현재 웨이브의 상세 데이터를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "H|Wave")
	const FHWaveData& GetCurrentWaveData() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "H|Wave|Event")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "H|Wave|Event")
	FOnWaveProgressUpdated OnWaveProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "H|Wave|Event")
	FOnWaveCompleted OnWaveCompleted;

protected:
	/** @brief 매 초마다 클리어 조건을 체크하기 위한 타이머 함수 */
	void UpdateWaveProgress();

	/** @brief 웨이브 종료 시 이자를 계산하여 지급합니다. */
	void CalculateInterest();

private:
	UPROPERTY()
	TObjectPtr<UHWaveConfigDataAsset> WaveConfig;

	/** @brief 유효하지 않은 인덱스 접근 시 반환할 기본 데이터 */
	FHWaveData DefaultWaveData;

	UPROPERTY()
	EHWaveState CurrentState = EHWaveState::Ready;

	int32 CurrentWaveIndex = 0;
	float CurrentTimer = 0.0f;
	int32 CurrentKillCount = 0;

	FTimerHandle WaveUpdateTimerHandle;
};
