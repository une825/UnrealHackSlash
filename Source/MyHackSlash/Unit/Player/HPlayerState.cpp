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

void AHPlayerState::AddGold(int32 InAmount)
{
	if (InAmount <= 0) return;

	CurrentGold += InAmount;
	OnGoldChanged.Broadcast(CurrentGold);
}

bool AHPlayerState::ConsumeGold(int32 InAmount)
{
	if (InAmount <= 0 || CurrentGold < InAmount) return false;

	CurrentGold -= InAmount;
	OnGoldChanged.Broadcast(CurrentGold);
	return true;
}
