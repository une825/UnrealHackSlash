// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Skill/HCombatInterface.h"
#include "DataAsset/HMonsterStatRow.h"

#include "HBaseCharacter.generated.h"

class UInputAction;

UCLASS()
class MYHACKSLASH_API AHBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IHCombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHBaseCharacter();

public:
	// 레벨을 설정하고 해당 레벨에 맞는 스탯을 적용합니다.
	UFUNCTION(BlueprintCallable, Category = "Stat")
	virtual void InitializeStat(int32 NewLevel);

	// 캐릭터 상태를 초기화합니다 (오브젝트 풀용)
	virtual void ResetCharacter();

	int32 GetLevel() const { return Level; }

	class UHUnitProfileData* GetUnitProfileData() const { return UnitProfileData; }

	const FMonsterStatRow& GetCurrentStat() const { return CurrentStat; }

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	void Attack();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	virtual bool CanJumpInternal_Implementation() const override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	void ProcessAttack();
	void AttackBegin();
	void AttackEnd(UAnimMontage* InAnimMontage, bool bInInterrupted);
	virtual void NotifyAttackEnd();

	UFUNCTION(BlueprintCallable)
	void UpdateWalkSpeed(const float InNewWalkSpeed);

	virtual void SetDead();

	// Attack Hit Section
protected:
	virtual void AttackHitCheck() override;

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

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stat")
	FMonsterStatRow CurrentStat;

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

};
