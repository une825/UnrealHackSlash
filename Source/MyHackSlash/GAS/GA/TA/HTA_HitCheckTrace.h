// Source/MyHackSlash/GA/TA/HTA_HitCheckTrace.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "HTA_HitCheckTrace.generated.h"

/**
 * 트레이스를 통해 타겟(적) 정보를 수집하는 타겟 액터 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API AHTA_HitCheckTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AHTA_HitCheckTrace();

	/** 타겟팅을 시작할 때 호출됩니다. */
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	/** 타겟팅이 확정되었을 때 호출됩니다. */
	virtual void ConfirmTargetingAndContinue() override;

protected:
	/** 실제 트레이스를 수행하여 타겟 데이터를 생성합니다. */
	virtual FGameplayAbilityTargetDataHandle MakeTargetData() const;
};
