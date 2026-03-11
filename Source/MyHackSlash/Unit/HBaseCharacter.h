// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Skill/HCombatInterface.h"

#include "HBaseCharacter.generated.h"

UCLASS()
class MYHACKSLASH_API AHBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IHCombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHBaseCharacter();

public:
	void Attack();

public:
	//~ IAbilitySystemInterface 시작
	/** 어빌리티 시스템 컴포넌트를 반환합니다. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

protected:
	virtual bool CanJumpInternal_Implementation() const override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	void ProcessAttack();
	void AttackBegin();
	void AttackEnd(UAnimMontage* InAnimMontage, bool bInInterrupted);

	UFUNCTION(BlueprintCallable)
	void UpdateWalkSpeed(const float InNewWalkSpeed);

	virtual void SetDead();

	// Attack Hit Section
protected:
	virtual void AttackHitCheck() override;

protected:
	/** 어빌리티 시스템 컴포넌트입니다. 게임플레이 어트리뷰트 및 게임플레이 어빌리티를 사용하려면 필요합니다. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	UAbilitySystemComponent* AbilitySystemComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> DeadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = "true"))
	bool Attackable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dead, Meta = (AllowPrivateAccess = "true"))
	float DeadEventDelayTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dead, Meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> LastDamageCauser;

};
