// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HIntroUI.generated.h"

/**
 * @brief 인트로 화면의 UI를 담당하는 위젯 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHIntroUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** @brief 게임 시작 버튼 클릭 시 호출됩니다. */
	UFUNCTION()
	void OnStartButtonClicked();

protected:
	/** @brief 블루프린트 위젯의 버튼과 자동으로 바인딩됩니다. (이름이 같아야 함) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> StartButton;

	/** @brief 이동할 게임 레벨 이름 */
	UPROPERTY(EditAnywhere, Category = "H|Level")
	FName InGameLevelName = TEXT("TopDownMap");
};
