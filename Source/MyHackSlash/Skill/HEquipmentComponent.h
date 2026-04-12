#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpec.h"
#include "HEquipmentComponent.generated.h"

class UHMainGem;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

/**
 * 젬 장착 및 GAS 능력 부여를 전담하는 컴포넌트입니다.
 */
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class MYHACKSLASH_API UHEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHEquipmentComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** @brief 특정 슬롯에 메인 젬을 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|Equipment")
	bool EquipGem(int32 InSlotIndex, UHMainGem* InGem);

	/** @brief 특정 슬롯의 메인 젬에 보조 젬을 장착합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|Equipment")
	bool EquipSupportGem(int32 InSlotIndex, UHSupportGem* InSupportGem);

	/** @brief 특정 슬롯의 젬을 해제합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|Equipment")
	void UnequipGem(int32 InSlotIndex, bool bInReturnToInventory = true);

	/** @brief 특정 슬롯에 장착된 젬을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem|Equipment")
	UHMainGem* GetEquippedGem(int32 InSlotIndex) const;

protected:
	/** @brief 특정 슬롯의 어빌리티 스펙에 보조 젬 효과를 업데이트합니다. */
	void UpdateAbilitySpec(int32 InSlotIndex);

public:
	/** @brief 장착 상태가 변경될 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Gem|Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

protected:
	/** @brief 현재 장착된 메인 젬 리스트 (최대 4개) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gem|Equipment")
	TArray<TObjectPtr<UHMainGem>> EquippedMainGems;

	/** @brief 장착된 젬들의 어빌리티 핸들을 저장 */
	TMap<int32, FGameplayAbilitySpecHandle> EquippedAbilityHandles;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
};
