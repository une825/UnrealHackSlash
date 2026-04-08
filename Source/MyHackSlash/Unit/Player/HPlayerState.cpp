// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Player/HPlayerState.h"
#include "AbilitySystemComponent.h"

AHPlayerState::AHPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AttributeSet = CreateDefaultSubobject<UHCharacterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AHPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AbilitySystemComponent)
	{
		// Gold Attribute 변경 시 OnGoldAttributeChanged 호출되도록 바인딩
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UHCharacterAttributeSet::GetGoldAttribute())
			.AddUObject(this, &AHPlayerState::OnGoldAttributeChanged);
	}
}

void AHPlayerState::OnGoldAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnGoldChanged.Broadcast(static_cast<int32>(Data.NewValue));
}

int32 AHPlayerState::GetCurrentGold() const
{
	return AttributeSet ? static_cast<int32>(AttributeSet->GetGold()) : 0;
}

void AHPlayerState::AddGold(int32 InAmount)
{
	if (InAmount <= 0 || !AttributeSet) return;

	float CurrentGold = AttributeSet->GetGold();
	AttributeSet->SetGold(CurrentGold + InAmount);
	// Note: OnGoldChanged는 OnGoldAttributeChanged 콜백에 의해 자동으로 브로드캐스트됩니다.
}

bool AHPlayerState::ConsumeGold(int32 InAmount)
{
	if (InAmount <= 0 || !AttributeSet) return false;

	float CurrentGold = AttributeSet->GetGold();
	if (CurrentGold < InAmount) return false;

	AttributeSet->SetGold(CurrentGold - InAmount);
	return true;
}
