#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "HExecutionCalculation_Damage.generated.h"

/**
 * @brief 공격자의 스탯을 기반으로 최종 데미지를 산출하는 GAS ExecutionCalculation 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHExecutionCalculation_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UHExecutionCalculation_Damage();

	/** @brief 실제 데미지 계산 로직이 수행되는 함수입니다. */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& ExecutionOutputs) const override;
};
