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

	/** 현재 인벤토리의 모든 젬 리스트를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Gem|Inventory")
	const TArray<UHGemBase*>& GetInventoryGems() const { return InventoryGems; }

public:
	/** 인벤토리 데이터가 변할 때 호출되는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Gem|Events")
	FOnGemInventoryUpdated OnGemInventoryUpdated;

private:
	/** 보유 중인 모든 젬 인스턴스 */
	UPROPERTY(VisibleAnywhere, Category = "Gem|Data")
	TArray<TObjectPtr<UHGemBase>> InventoryGems;
};
