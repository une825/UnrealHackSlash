#include "System/HUIManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "DataAsset/HUIDataAsset.h"
#include "UObject/ConstructorHelpers.h"

UHUIManager::UHUIManager()
{
	// 생성자에서 UI 데이터 에셋을 자동으로 찾아서 할당합니다.
	// 경로: /Game/Blueprint/DataAsset/DA_UIConfig (없을 경우를 대비해 나중에 블루프린트에서 수동 할당하거나 경로를 맞춰주세요.)
	static ConstructorHelpers::FObjectFinder<UHUIDataAsset> UIDataAssetFinder(TEXT("/Game/Blueprint/DataAsset/BP_UIData"));
	if (UIDataAssetFinder.Succeeded())
	{
		UIDataAsset = UIDataAssetFinder.Object;
	}
}

void UHUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ActiveWidgets.Empty();
}

UUserWidget* UHUIManager::ShowWidgetByName(FName WidgetName, int32 ZOrder)
{
	if (!UIDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("UHUIManager: UIDataAsset is null! Please check the path."));
		return nullptr;
	}

	TSoftClassPtr<UUserWidget> SoftWidgetClass = UIDataAsset->FindWidgetClassByName(WidgetName);
	if (SoftWidgetClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUIManager: Cannot find widget class for name [%s]"), *WidgetName.ToString());
		return nullptr;
	}

	// SoftClassPtr이므로 로드해서 사용
	UClass* LoadedClass = SoftWidgetClass.LoadSynchronous();
	if (LoadedClass)
	{
		return ShowWidget(LoadedClass, ZOrder);
	}

	return nullptr;
}

UUserWidget* UHUIManager::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!WidgetClass) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(World, WidgetClass);
	if (NewWidget)
	{
		NewWidget->AddToViewport(ZOrder);
		ActiveWidgets.Add(NewWidget);
		return NewWidget;
	}

	return nullptr;
}

void UHUIManager::HideWidget(UUserWidget* Widget)
{
	if (!Widget) return;

	Widget->RemoveFromParent();
	ActiveWidgets.Remove(Widget);
}

void UHUIManager::HideAllWidgets()
{
	for (UUserWidget* Widget : ActiveWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}
	ActiveWidgets.Empty();
}
