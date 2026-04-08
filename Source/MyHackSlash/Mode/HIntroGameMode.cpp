// Fill out your copyright notice in the Description page of Project Settings.

#include "Mode/HIntroGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AHIntroGameMode::AHIntroGameMode()
{
	// 인트로에서는 단순한 PlayerController를 사용해도 무방합니다.
	DefaultPawnClass = nullptr;
}

void AHIntroGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 1. 인트로 UI 위젯 생성 및 표시
	if (IntroUIClass)
	{
		if (UUserWidget* IntroWidget = CreateWidget<UUserWidget>(GetWorld(), IntroUIClass))
		{
			IntroWidget->AddToViewport();
		}
	}

	// 2. 마우스 커서 활성화 및 입력 모드 설정
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->bShowMouseCursor = true;

		// UI 입력만 받도록 설정
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	UE_LOG(LogTemp, Log, TEXT("AHIntroGameMode: Intro Screen Initialized"));
}
