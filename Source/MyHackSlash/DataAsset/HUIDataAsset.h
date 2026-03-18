#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blueprint/UserWidget.h"
#include "HUIDataAsset.generated.h"

/**
 * 개별 UI 위젯의 정보를 담는 구조체
 */
USTRUCT(BlueprintType)
struct FUIWidgetInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FName WidgetName;

	// 메모리 절약을 위해 SoftClassPtr 사용 (필요할 때 로드)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UUserWidget> WidgetClass;
};

/**
 * 모든 UI 위젯 정보를 관리하는 데이터 에셋
 */
UCLASS()
class MYHACKSLASH_API UHUIDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TArray<FUIWidgetInfo> WidgetList;

	// 이름으로 위젯 클래스를 찾아 반환합니다.
	TSoftClassPtr<UUserWidget> FindWidgetClassByName(FName Name) const
	{
		for (const FUIWidgetInfo& Info : WidgetList)
		{
			if (Info.WidgetName == Name)
			{
				return Info.WidgetClass;
			}
		}
		return nullptr;
	}
};
