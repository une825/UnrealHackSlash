// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Skill/HCombatInterface.h"
#include "DataAsset/HUnitProfileData.h"
#include <Attribute/HCharacterAttributeSet.h>

#include "HBaseCharacter.generated.h"

class UInputAction;
class UNiagaraSystem;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHungerChanged, float, CurrentHunger, float, MaxHunger);

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

	// 배고픔 변경 시 호출되는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnHungerChanged OnHungerChanged;

public:
	/** @brief 유닛의 스탯을 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "Stat")
	virtual void InitializeStat(int32 InNewLevel);

	/** @brief 유닛의 상태를 리셋합니다. (오브젝트 풀링 등에서 재사용 시) */
	virtual void ResetCharacter();

	int32 GetLevel() const;
	float GetCurrentHP() const;
	float GetMaxHP() const;
	float GetCurrentHunger() const;
	float GetMaxHunger() const;

	/** @brief GAS Attribute Getters */
	float GetAttackDamage() const;
	float GetAttackSpeedRate() const;
	float GetAttackRange() const;
	float GetCriticalRate() const;
	float GetCriticalMultiplier() const;
	float GetMovementSpeed() const;

	const UHUnitProfileData* GetUnitProfileData() const { return UnitProfileData; }

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAnimMontage* GetAttackMontage() const;
	float GetAIPatrolRadius() const;
	float GetAIDetectRadius() const;

	/** @brief 마지막으로 데미지를 가한 액터를 설정합니다. */
	void SetLastDamageCauser(AActor* InCauser) { LastDamageCauser = InCauser; }

	/** @brief 공격 실행 및 종료 관리 */
	void Attack();
	void ProcessAttack();
	bool BeginAttackState(bool bInStopMovementImmediately = true);
	void EndAttackState();
	void AttackEnd(UAnimMontage* InAnimMontage, bool bInInterrupted);

	/** @brief 피격 및 연출 관련 */
	void HandleHitSound();
	void HandleHUDDamageEffect();
	void PlayHittedEffect();
	void ShowDamageText(float InActualDamage, bool bInIsCritical, AActor* InDamageCauser);
	virtual void HandleCameraShake(float InDamageAmount);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostInitializeComponents() override;

	/** @brief ASC가 유효해진 시점에 어트리뷰트 변경 콜백을 바인딩합니다. */
	virtual void BindAttributeCallbacks();

	virtual bool CanJumpInternal_Implementation() const override;
	virtual float TakeDamage(float InDamageAmount, FDamageEvent const& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser) override;

	/** @brief Attribute 변경 시 호출될 콜백 */
	virtual void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnHungerAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnMovementSpeedAttributeChanged(const FOnAttributeChangeData& Data);

protected:
	/** @brief 공격 로직 내부 처리 */
	void AttackBegin();
	virtual void NotifyAttackEnd();

	UFUNCTION(BlueprintCallable)
	void UpdateWalkSpeed(const float InNewWalkSpeed);

	virtual void SetDead();
	virtual void OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void AttackHitCheck() override;

protected:
	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag AttackAbilityTag;

	UPROPERTY(EditAnywhere, Category = "GAS")
	FGameplayTag DeadTag;

	void EnableRagdoll();
	void SetDeadImpulse();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> StartAbilities;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<class UGameplayEffect> InitStatEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHCharacterAttributeSet> AttributeSet;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DataAsset)
	TObjectPtr<class UHUnitProfileData> UnitProfileData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = "true"))
	bool Attackable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = "true"))
	float HitRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = "true"))
	FName WeaponSocketName = TEXT("Hand_R_Socket");

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
