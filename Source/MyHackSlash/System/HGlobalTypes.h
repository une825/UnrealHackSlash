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
