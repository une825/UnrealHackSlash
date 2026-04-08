#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HGA_Attack.generated.h"

/**
 * 캐릭터의 공격 애니메이션(몽타주) 재생을 담당하는 상위 어빌리티 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_Attack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** 몽타주 종료 시 호출될 콜백 */
	UFUNCTION()
	void OnMontageEnded();
};
