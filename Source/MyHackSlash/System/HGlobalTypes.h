#pragma once

#include "CoreMinimal.h"
#include "HGlobalTypes.generated.h"

/**
 * 젬의 대분류: 액티브 스킬(Main) 또는 패시브 보조(Support)
 */
UENUM(BlueprintType)
enum class HEGemCategory : uint8
{
	Main      UMETA(DisplayName = "Main"),
	Support   UMETA(DisplayName = "Support")
};

/**
 * 젬의 작동 방식 및 호환성 정의
 */
UENUM(BlueprintType)
enum class HEGemType : uint8
{
	Common     UMETA(DisplayName = "Common"),
	Melee      UMETA(DisplayName = "Melee"),
	Projectile UMETA(DisplayName = "Projectile")
};

/**
 * 젬의 속성 분류 (공명 시스템 및 데미지 타입용)
 */
UENUM(BlueprintType)
enum class HEElement : uint8
{
	None       UMETA(DisplayName = "None"),
	Physical   UMETA(DisplayName = "Physical"),
	Fire       UMETA(DisplayName = "Fire"),
	Ice        UMETA(DisplayName = "Ice"),
	Lightning  UMETA(DisplayName = "Lightning"),
	Magic      UMETA(DisplayName = "Magic")
};

/**
 * 공격의 형태적 분류
 */
UENUM(BlueprintType)
enum class HEShape : uint8
{
	Area       UMETA(DisplayName = "Area (AOE)"),
	Projectile UMETA(DisplayName = "Projectile")
};

/**
 * 웨이브의 성격
 */
UENUM(BlueprintType)
enum class EHWaveType : uint8
{
	Battle      UMETA(DisplayName = "Battle"),
	Boss        UMETA(DisplayName = "Boss"),
	Reward      UMETA(DisplayName = "Reward"),
	Shop        UMETA(DisplayName = "Shop")
};

/**
 * 웨이브 클리어 조건
 */
UENUM(BlueprintType)
enum class EHWaveClearType : uint8
{
	TimeSurvival UMETA(DisplayName = "TimeSurvival"),
	KillCount    UMETA(DisplayName = "KillCount"),
	ManualUI     UMETA(DisplayName = "ManualUI")
};

/**
 * 웨이브 진행 상태
 */
UENUM(BlueprintType)
enum class EHWaveState : uint8
{
	Ready        UMETA(DisplayName = "Ready"),
	InProgress   UMETA(DisplayName = "InProgress"),
	Completed    UMETA(DisplayName = "Completed")
};

/**
 * 개별 웨이브 데이터 구조체
 */
USTRUCT(BlueprintType)
struct FHWaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 WaveIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	EHWaveType WaveType = EHWaveType::Battle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	EHWaveClearType ClearType = EHWaveClearType::TimeSurvival;

	/** @brief 클리어 기준값 (초 또는 마리 수) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float ClearValue = 60.0f;

	/** @brief [전투/보스용] 몬스터 스폰 세팅 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TObjectPtr<class UHMonsterSpawnerDataAsset> MonsterSpawnerConfig;

	/** @brief [보상/상점용] 등장할 아이템/상점 세팅 데이터 (추후 확장) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TObjectPtr<class UPrimaryDataAsset> RewardData;

	/** @brief 엑셀(DataTable)에서 관리되는 상점 보상 데이터 참조 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FDataTableRowHandle ShopRewardRow;
	};

