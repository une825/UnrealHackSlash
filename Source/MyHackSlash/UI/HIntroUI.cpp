// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HIntroUI.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UHIntroUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UHIntroUI::OnStartButtonClicked);
	}
}

void UHIntroUI::OnStartButtonClicked()
{
	// 인게임 레벨로 이동
	UGameplayStatics::OpenLevel(GetWorld(), InGameLevelName);

	UE_LOG(LogTemp, Log, TEXT("UHIntroUI: Transitioning to InGame Level (%s)"), *InGameLevelName.ToString());
}
