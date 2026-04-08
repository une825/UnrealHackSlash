#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HCharacterAttributeSet.generated.h"

// 속성 접근을 위한 매크로 정의
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 프로젝트 내 캐릭터들의 핵심 스탯을 관리하는 AttributeSet 클래스
 */
UCLASS()
class MYHACKSLASH_API UHCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHCharacterAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;

public:
	// --- 핵심 스탯 (GAS_Combat_System_Design.md 참조) ---

	/** 레벨 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, Level);

	/** 최대 레벨 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxLevel;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxLevel);

	/** 데미지 (Meta-Attribute: 체력 차감에 사용됨) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, Damage);

	/** 현재 체력 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, Health);

	/** 최대 체력 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxHealth);

	/** 기본 공격력 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, AttackDamage);

	/** 최대 공격력 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAttackDamage;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxAttackDamage);

	/** 공격 유효 거리 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AttackRange;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, AttackRange);

	/** 최대 공격 유효 거리 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAttackRange;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxAttackRange);

	/** 공격 반경 (범위 공격용) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AttackRadius;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, AttackRadius);

	/** 최대 공격 반경 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAttackRadius;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxAttackRadius);

	/** 애니메이션 재생 속도 배율 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AttackSpeedRate;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, AttackSpeedRate);

	/** 최대 애니메이션 재생 속도 배율 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAttackSpeedRate;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxAttackSpeedRate);

	/** 이동 속도 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MovementSpeed);

	/** 최대 이동 속도 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxMovementSpeed;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxMovementSpeed);

	/** 치명타 확률 (0~100) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CriticalRate;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, CriticalRate);

	/** 최대 치명타 확률 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxCriticalRate;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxCriticalRate);

	/** 치명타 데미지 배율 (예: 1.5 = 150%) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CriticalMultiplier;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, CriticalMultiplier);

	/** 최대 치명타 데미지 배율 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxCriticalMultiplier;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxCriticalMultiplier);

	/** 현재 경험치 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, Experience);

	/** 다음 레벨에 필요한 경험치 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxExperience;
	ATTRIBUTE_ACCESSORS(UHCharacterAttributeSet, MaxExperience);
};
