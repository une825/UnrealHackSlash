#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HGA_Magnet.generated.h"

/**
 * 자력 상태일 때 주변의 아이템(골드 등)을 플레이어에게 끌어당기는 어빌리티입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_Magnet : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_Magnet();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** @brief 주기적으로 주변 아이템을 검사 */
	void TickMagnet();

	/** @brief 자력 범위 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float MagnetRadius = 1000.0f;

	/** @brief 끌어당기는 속도 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float PullSpeed = 1500.0f;

	/** @brief Tick 간격 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float TickInterval = 0.05f;

	FTimerHandle MagnetTimerHandle;
};
