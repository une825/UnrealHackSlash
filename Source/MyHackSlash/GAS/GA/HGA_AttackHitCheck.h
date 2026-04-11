// Source/MyHackSlash/GA/HGA_AttackHitCheck.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HGA_AttackHitCheck.generated.h"

/**
 * 애니메이션 노티파이 등에서 전달받은 이벤트를 기반으로 실제 공격 판정을 수행하는 능력 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_AttackHitCheck : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_AttackHitCheck();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** @brief 트레이스 결과를 처리하는 콜백 함수입니다. */
	UFUNCTION()
	void OnTraceResultCallback(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

protected:
	/** 사용할 타겟 액터 클래스 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<class AGameplayAbilityTargetActor> TargetActorClass;

	/** 공격 시 적용할 데미지 이펙트 클래스 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<class UGameplayEffect> AttackDamageEffect;

	/** 명중 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	TObjectPtr<USoundBase> HitSound;

	/** 공격을 시작한 캐릭터의 레벨 */
	float AttackerLevel;
};
