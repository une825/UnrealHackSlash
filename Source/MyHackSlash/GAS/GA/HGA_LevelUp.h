#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HGA_LevelUp.generated.h"

/**
 * @brief 레벨업 이벤트 발생 시 자동으로 실행되어 체력 회복 및 스탯 재초기화를 담당하는 어빌리티입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_LevelUp : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_LevelUp();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** @brief 체력 전회복을 위한 GameplayEffect 클래스 */
	UPROPERTY(EditAnywhere, Category = "LevelUp")
	TSubclassOf<class UGameplayEffect> HealEffectClass;

	/** @brief 레벨업 시 실행할 GameplayCue 태그 */
	UPROPERTY(EditAnywhere, Category = "LevelUp")
	FGameplayTag LevelUpCueTag;
};
