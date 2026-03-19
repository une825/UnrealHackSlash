#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "HSelectAbilityEntryUI.generated.h"

class UTextBlock;
class UButton;

/**
 * 능력 선택 리스트 항목의 데이터를 담는 객체입니다.
 */
UCLASS(BlueprintType)
class MYHACKSLASH_API UHSelectAbilityData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FText Description;
};

/**
 * 능력 선택 팝업 내의 개별 항목을 담당하는 UI 클래스입니다.
 */
UCLASS()
class MYHACKSLASH_API UHSelectAbilityEntryUI : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* InListItemObject) override;

	UFUNCTION()
	void OnSelectButtonClicked();

protected:
	// 블루프린트 위젯 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;
};
