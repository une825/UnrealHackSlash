// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HIntroUI.h"
#include "Components/Button.h"
#include "Engine/World.h"
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
	UWorld* World = GetWorld();
	if (!World) return;

	if (World->GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(World, InGameLevelName);
		UE_LOG(LogTemp, Log, TEXT("UHIntroUI: Transitioning to InGame Level (%s)"), *InGameLevelName.ToString());
		return;
	}

	if (World->GetAuthGameMode())
	{
		const FString TravelURL = FString::Printf(TEXT("%s?listen"), *InGameLevelName.ToString());
		World->ServerTravel(TravelURL);
		UE_LOG(LogTemp, Log, TEXT("UHIntroUI: ServerTravel to InGame Level (%s)"), *TravelURL);
	}
}
