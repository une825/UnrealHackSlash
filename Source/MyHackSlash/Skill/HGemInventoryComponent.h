#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/HGlobalTypes.h"
#include "GameplayAbilitySpec.h"
#include "DataAsset/HGemDataAsset.h"
#include "HGemInventoryComponent.generated.h"

class UHGemBase;
class UHMainGem;
class UHSupportGem;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGemInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGemUpgraded, UHGemBase*, NewGem);

/**
 * 캐릭터의 젬 인벤토리와 장착 슬롯을 관리하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYHACKSLASH_API UHGemInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHGemInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 젬 데이터를 기반으로 새로운 젬을 생성하여 인벤토리에 추가합니다. */
	UFUNCTION(BlueprintCallable, Category = "Gem|Inventory")
	UHGemBase* AddGem(const FHGemData& InGemData);

	/** @brief 이미 생성된 젬 인스턴스를 인벤토리에 다시 추가합니다. (장착 해제 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "Gem|Inventory")
	void AddGemInstance(UHGemBase* InGemInstance);

	/** @brief 인벤토리에서 특정 젬 인스턴스를 제거합니다. (장착 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "Gem|Inventory")
	void RemoveGemInstance(UHGemBase* InGemInstance);

	/** 현재 인벤토리의 모든 젬 리스트를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem|Inventory")
	const TArray<UHGemBase*>& GetInventoryGems() const { return InventoryGems; }

protected:
	/** @brief 동일한 젬 3개가 모였는지 확인하고 자동 업그레이드를 수행합니다. */
	void CheckAndUpgradeGems();

private:
	/** @brief 모든 젬(인벤토리 + 장착)을 수집하여 그룹화합니다. */
	void GatherAllGems(TMap<FName, TArray<UHGemBase*>>& OutGemGroups, TMap<UHGemBase*, int32>& OutMainSlotMap, TMap<UHGemBase*, int32>& OutSupportSlotMap) const;

	/** @brief 단일 그룹에 대해 업그레이드 가능 여부를 판단하고 실행합니다. */
	bool TryUpgradeSingleGroup(UHGemDataAsset* GemCollection, class UHEquipmentComponent* EquipComp);

	/** @brief 업그레이드에 사용될 재료 젬 3개를 제거합니다. */
	void ConsumeGems(const TArray<UHGemBase*>& InGems, const TMap<UHGemBase*, int32>& InSupportSlotMap, class UHEquipmentComponent* EquipComp);

	/** @brief 생성된 상위 티어 젬을 적절한 위치(인벤토리 또는 기존 장착 슬롯)에 배치합니다. */
	void DistributeUpgradedGem(UHGemBase* NewGem, UHGemBase* SourceGem, class UHEquipmentComponent* EquipComp, int32 MainSlot, int32 SupportSlot);

public:
	/** 인벤토리 데이터가 변할 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Gem|Events")
	FOnGemInventoryUpdated OnGemInventoryUpdated;

	/** 젬이 업그레이드되었을 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Gem|Events")
	FOnGemUpgraded OnGemUpgraded;

private:
	/** 보유 중인 모든 젬 인스턴스 */
	UPROPERTY(VisibleAnywhere, Category = "Gem|Data")
	TArray<TObjectPtr<UHGemBase>> InventoryGems;
};
