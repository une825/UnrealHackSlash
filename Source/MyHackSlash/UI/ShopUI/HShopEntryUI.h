// Source/MyHackSlash/UI/ShopUI/HShopEntryUI.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "HShopEntryUI.generated.h"

class UImage;
class UTextBlock;
class UButton;

/**
 * 상점의 개별 상품 항목을 표시하는 위젯 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHShopEntryUI : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	// IUserObjectListEntry 인터페이스 구현 (TileView 아이템 데이터 바인딩)
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CurrencyIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrencyText; // 상품 가격

protected:
	UFUNCTION()
	void OnSelectButtonClicked();
};
