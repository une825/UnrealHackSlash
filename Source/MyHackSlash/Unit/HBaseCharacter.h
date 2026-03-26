// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Skill/HCombatInterface.h"
#include "DataAsset/HUnitStatRow.h"
#include "DataAsset/HUnitProfileData.h"

#include "HBaseCharacter.generated.h"

class UInputAction;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);

UCLASS()
class MYHACKSLASH_API AHBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IHCombatInterface
{
	GENERATED_BODY()

public:
	AHBaseCharacter();

public:
	// HP 변경 시 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnHPChanged OnHPChanged;

public:
	// 유닛의 스탯을 초기화합니다. 하위 클래스에서 각자의 타입에 맞게 구현합니다.
	UFUNCTION(BlueprintCallable, Category = "Stat")
	virtual void InitializeStat(int32 InNewLevel);

	virtual void ResetCharacter();

	const int32 GetLevel() const { return Level; }
	const int32 GetCurrentHP() const { return CurrentHP; }
	const UHUnitProfileData* GetUnitProfileData() const { return UnitProfileData; }
	const FUnitStatRow& GetCurrentStat() const { return CurrentStat; }

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAnimMontage* GetAttackMontage() const;
	float GetAIPatrolRadius() const;
	float GetAIDetectRadius() const;

public:
	void Attack();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual float TakeDamage(float InDamageAmount, FDamageEvent const& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser) override;

protected:
	void ProcessAttack();
	void AttackBegin();
	void AttackEnd(UAnimMontage* InAnimMontage, bool bInInterrupted);
	virtual void NotifyAttackEnd();

	UFUNCTION(BlueprintCallable)
	void UpdateWalkSpeed(const float InNewWalkSpeed);

	virtual void SetDead();
	virtual void AttackHitCheck() override;

	void PlayHittedEffect();
	void EnableRagdoll();
	void SetDeadImpulse();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> StartAbilities;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DataAsset)
	TObjectPtr<class UHUnitProfileData> UnitProfileData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	int32 Level = 1;

	// 공통적으로 사용되는 런타임 스탯 데이터
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	FUnitStatRow CurrentStat;

	// 실시간 체력 정보
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	float CurrentHP = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	float MaxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = "true"))
	bool Attackable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dead, Meta = (AllowPrivateAccess = "true"))
	bool IsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dead, Meta = (AllowPrivateAccess = "true"))
	float DeadEventDelayTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dead, Meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> LastDamageCauser;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> HittedEffect;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UNiagaraSystem> HittedBodyEffect;

	/** @brief 피격 시 실행할 카메라 쉐이크 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;

	/** @brief 데미지 수치를 카메라 쉐이크 강도(Scale)로 변환하는 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float DamageToShakeScale = 0.1f;
};
