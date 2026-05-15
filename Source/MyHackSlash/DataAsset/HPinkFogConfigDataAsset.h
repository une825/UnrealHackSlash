#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HPinkFogConfigDataAsset.generated.h"

class UGameplayEffect;
class UNiagaraSystem;

/**
 * 핑크 안개 시스템 설정을 관리하는 데이터 에셋입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHPinkFogConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 전투 웨이브 중 핑크 안개가 발생할 확률 (0~1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OccurrenceProbability = 0.1f;

	/** @brief 발생 여부를 체크할 시간 간격 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog")
	float OccurrenceCheckInterval = 5.0f;

	/** @brief 안개 활성 시간 (페이드 인/아웃 제외) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog")
	float ActiveDuration = 30.0f;

	/** @brief 페이드 인/아웃에 걸리는 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog")
	float FadeDuration = 5.0f;

	/** @brief 몬스터 강화에 사용할 GameplayEffect 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog|GAS")
	TSubclassOf<UGameplayEffect> MonsterBuffEffect;

	/** @brief 안개 중 보상(골드, 경험치) 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog", meta = (ClampMin = "1.0"))
	float RewardMultiplier = 2.0f;

	/** @brief 안개 색상 (Fog 및 PostProcess용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog|Visual")
	FLinearColor FogColor = FLinearColor(1.0f, 0.41f, 0.7f, 1.0f); // Hot Pink

	/** @brief 안개 발생 시 소환할 나이아가라 시스템 (예: 꽃잎, 먼지 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PinkFog|Visual")
	TObjectPtr<UNiagaraSystem> FogNiagaraSystem;
};
