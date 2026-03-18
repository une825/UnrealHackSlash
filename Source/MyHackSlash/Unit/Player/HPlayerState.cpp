// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/Player/HPlayerState.h"
#include "AbilitySystemComponent.h"

AHPlayerState::AHPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AHPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
