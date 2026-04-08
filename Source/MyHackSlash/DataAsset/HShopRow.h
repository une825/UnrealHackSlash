// Source/MyHackSlash/DataAsset/HShopRow.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HShopRow.generated.h"

/**
 * 상점에서 판매할 상품의 종류를 정의합니다.
 */
UENUM(BlueprintType)
enum class EHShopItemType : uint8
{
	Ability,    // 능력치 강화
	SkillGem,   // 스킬 젬
	Consumable, // 소모품
	Equipment   // 장비
};

/**
 * 상점 데이터 테이블의 행 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FHShopRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FHShopRow() : ItemType(EHShopItemType::Ability), Price(100) {}

	// 상품 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	EHShopItemType ItemType;

	// 상품 고유 식별 태그 (구매 대상 아이템)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FGameplayTag ItemTag;

	// --- 구매 비용 설정 ---
	// 구매에 필요한 아이템(재화)의 ID 태그 (예: Currency.Gold, Item.Gem.Red 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FGameplayTag CostItemTag;

	// 지불해야 하는 아이템의 개수 (가격)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Price;
	// ----------------------

	// 상점에 표시될 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText Title;

	// 상점에 표시될 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText Description;

	// 상품 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TSoftObjectPtr<UTexture2D> Icon;

	// 상품의 가중치 (랜덤 상점 등장 확률용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	float Weight = 1.0f;
};
