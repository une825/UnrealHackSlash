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

public:
	// 경험치 변경 시 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnExpChanged OnExpChanged;

public:
	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

public:
	virtual void PossessedBy(AController* NewController);
	virtual void SetupPlayerInputComponent(class UInputComponent* InPlayerInputComponent) override;

	// 플레이어 스탯 및 경험치 초기화
	virtual void InitializeStat(int32 InNewLevel) override;

	// 경험치 추가 및 레벨업 체크
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void AddExp(float InExp);

	float GetCurrentExp() const { return CurrentExp; }
	float GetMaxExp() const { return MaxExp; }

protected:
	void SetupGASInputComponent();
	void GASInputPressed(const int32 InInputID);
	void GASInputReleased(const int32 InInputID);

	// 레벨업 시 호출되는 이벤트
	void OnLevelUp();

	virtual void SetDead() override;

protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	TMap<int32, TSubclassOf<class UGameplayAbility>> StartInputAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	FPlayerStatRow CurrentPlayerStat;

	// 플레이어 경험치 정보
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	float CurrentExp = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	float MaxExp = 100.0f;

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
};
