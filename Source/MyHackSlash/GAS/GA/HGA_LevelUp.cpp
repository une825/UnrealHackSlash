#include "GAS/GA/HGA_LevelUp.h"
#include "AbilitySystemComponent.h"
#include "Unit/HBaseCharacter.h"
#include "Attribute/HCharacterAttributeSet.h"
#include "System/HUIManager.h"
#include "System/HSoundManager.h"

UHGA_LevelUp::UHGA_LevelUp()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 1. 어빌리티 자체 식별 태그 설정
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.LevelUp")));
	SetAssetTags(TagContainer);

	// 2. 특정 이벤트 발생 시 자동 활성화되도록 트리거 설정
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Character.LevelUp"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UHGA_LevelUp::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AHBaseCharacter* Character = Cast<AHBaseCharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		int32 NewLevel = 1;
		if (TriggerEventData)
		{
			NewLevel = static_cast<int32>(TriggerEventData->EventMagnitude);
		}
		else
		{
			NewLevel = Character->GetLevel();
		}

		// 1. 스탯 재초기화 (MaxHP 증가 등 반영)
		Character->InitializeStat(NewLevel);

		// 2. 체력 전회복 (HealEffectClass가 있으면 GE로, 없으면 직접 Set)
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (HealEffectClass && ASC)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			ASC->ApplyGameplayEffectToSelf(HealEffectClass.GetDefaultObject(), 1.0f, Context);
		}
		else if (UHCharacterAttributeSet* Set = const_cast<UHCharacterAttributeSet*>(ASC->GetSet<UHCharacterAttributeSet>()))
		{
			Set->SetHealth(Set->GetMaxHealth());
		}

		// 3. 시각적/청각적 피드백 (GameplayCue)
		if (LevelUpCueTag.IsValid())
		{
			FGameplayCueParameters CueParams;
			CueParams.RawMagnitude = static_cast<float>(NewLevel);
			ASC->ExecuteGameplayCue(LevelUpCueTag, CueParams);
		}

		// 4. UI 시스템 연동 (레벨업 팝업)
		if (Character->IsPlayerControlled())
		{
			if (UHUIManager* UIManager = Character->GetGameInstance()->GetSubsystem<UHUIManager>())
			{
				UIManager->ShowWidgetByName(TEXT("SelectAbilityPopupUI"));
			}

			if (UHSoundManager* SoundManager = Character->GetWorld()->GetSubsystem<UHSoundManager>())
			{
				SoundManager->PlaySFXByKey(TEXT("LevelUpSound"), Character->GetActorLocation(), 1.0, true);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("GA_LevelUp: Character %s Leveled Up to %d!"), *Character->GetName(), NewLevel);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
