// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HIntroGameMode.generated.h"

/**
 * @brief 인트로 레벨 전용 게임모드입니다.
 */
UCLASS()
class MYHACKSLASH_API AHIntroGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHIntroGameMode();

protected:
	virtual void BeginPlay() override;

protected:
	/** @brief 인트로에서 띄울 UI 클래스 (에디터에서 할당) */
	UPROPERTY(EditAnywhere, Category = "H|UI")
	TSubclassOf<class UUserWidget> IntroUIClass;
};
