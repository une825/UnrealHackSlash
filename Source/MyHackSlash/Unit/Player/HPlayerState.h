// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include <Attribute/HCharacterAttributeSet.h>

#include "HPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MYHACKSLASH_API AHPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHPlayerState();

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** @brief 골드를 획득합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Currency")
	void AddGold(int32 InAmount);

	/** @brief 골드를 소비합니다. 성공 시 true를 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "H|Currency")
	bool ConsumeGold(int32 InAmount);

	/** @brief 현재 보유 중인 골드를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "H|Currency")
	int32 GetCurrentGold() const { return CurrentGold; }

	/** @brief 골드 변경 시 알림을 위한 델리게이트 (UI 등에서 바인딩) */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32 /*NewGold*/);
	FOnGoldChanged OnGoldChanged;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHCharacterAttributeSet> AttributeSet;

	/** @brief 현재 보유 중인 골드 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "H|Currency")
	int32 CurrentGold = 0;
};
