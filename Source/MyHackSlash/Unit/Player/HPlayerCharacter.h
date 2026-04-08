// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/HBaseCharacter.h"
#include "HPlayerCharacter.generated.h"

class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExpChanged, int, CurrentLevel, float, CurrentExp, float, MaxExp);

/**
 * 플레이어 전용 캐릭터 클래스입니다. 경험치 및 레벨업 시스템을 포함합니다.
 */
UCLASS()
class MYHACKSLASH_API AHPlayerCharacter : public AHBaseCharacter
{
	GENERATED_BODY()

public:
	AHPlayerCharacter();

	/** 플레이어의 젬 인벤토리를 관리하는 컴포넌트입니다. */
	UFUNCTION(BlueprintPure, Category = "Gem")
	class UHGemInventoryComponent* GetGemInventoryComponent() const { return GemInventoryComponent; }

	/** 플레이어의 젬 장착을 관리하는 컴포넌트입니다. */
	UFUNCTION(BlueprintPure, Category = "Gem")
	class UHEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

public:
	// 경험치 변경 시 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnExpChanged OnExpChanged;

	// 레벨업 시 호출되는 이벤트
	void OnLevelUp();

public:
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

public:
	virtual void BeginPlay() override;
	virtual void Tick(float InDeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InPlayerInputComponent) override;

	// 플레이어 스탯 및 경험치 초기화
	virtual void InitializeStat(int32 InNewLevel) override;

	// 경험치 추가 및 레벨업 체크
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void AddExp(float InExp);

	float GetCurrentExp() const;
	float GetMaxExp() const;

protected:
	void SetupGASInputComponent();
	void GASInputPressed(const int32 InInputID);
	void GASInputReleased(const int32 InInputID);

	virtual void SetDead() override;

protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TMap<int32, TSubclassOf<class UGameplayAbility>> StartInputAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** @brief 추가 스킬 슬롯용 입력 액션 배열 (Index 0은 이미 AttackAction이 담당하므로 Slot 1부터 매핑됨) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TArray<class UInputAction*> SkillActions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	FPlayerStatRow CurrentPlayerStat;

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** 젬 인벤토리 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UHGemInventoryComponent> GemInventoryComponent;

	/** 젬 장착 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UHEquipmentComponent> EquipmentComponent;

	/** 네비게이션 인보커 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNavigationInvokerComponent> NavInvokerComponent;
};
