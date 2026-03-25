#include "DataAsset/HUnitProfileData.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"

UAnimMontage* UHUnitProfileData::GetActionMontage(const FGameplayTag& InActionTag) const
{
	if (const TObjectPtr<UAnimMontage>* FoundMontage = ActionMontageMap.Find(InActionTag))
	{
		return *FoundMontage;
	}

	return AttackMontage;
}
