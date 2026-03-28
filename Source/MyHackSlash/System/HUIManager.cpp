#include "System/HUIManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "DataAsset/HUIDataAsset.h"
#include "UObject/ConstructorHelpers.h"
#include "UI/HDamageTextActor.h"

UHUIManager::UHUIManager()
{
	// 생성자에서 UI 데이터 에셋을 자동으로 찾아서 할당합니다.
	static ConstructorHelpers::FObjectFinder<UHUIDataAsset> UIDataAssetFinder(TEXT("/Game/System/BP_UIData"));
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

UUserWidget* UHUIManager::ShowWidgetByName(FName InWidgetName, int32 InZOrder)
{
	// 1. 이미 떠 있는 위젯이 있는지 맵에서 확인 (O(1))
	if (TObjectPtr<UUserWidget>* FoundWidget = ActiveWidgets.Find(InWidgetName))
	{
		return *FoundWidget;
	}

	if (!UIDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("UHUIManager: UIDataAsset is null!"));
		return nullptr;
	}

	TSoftClassPtr<UUserWidget> SoftWidgetClass = UIDataAsset->FindWidgetClassByName(InWidgetName);
	if (SoftWidgetClass.IsNull()) return nullptr;

	UClass* LoadedClass = SoftWidgetClass.LoadSynchronous();
	if (LoadedClass)
	{
		// 2. 위젯 생성 및 맵에 등록
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), LoadedClass);
		if (NewWidget)
		{
			NewWidget->AddToViewport(InZOrder);
			ActiveWidgets.Add(InWidgetName, NewWidget);
			return NewWidget;
		}
	}

	return nullptr;
}

UUserWidget* UHUIManager::GetWidgetByName(FName InWidgetName) const
{
	// 3. 맵에서 이름으로 즉시 찾아서 반환 (O(1))
	return ActiveWidgets.FindRef(InWidgetName);
}

UUserWidget* UHUIManager::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder)
{
	if (!WidgetClass) return nullptr;

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
	if (NewWidget)
	{
		NewWidget->AddToViewport(ZOrder);
		// 클래스 이름이나 고유 이름을 키로 등록 (직접 클래스로 띄울 때의 폴백)
		ActiveWidgets.Add(WidgetClass->GetFName(), NewWidget);
		return NewWidget;
	}

	return nullptr;
}

void UHUIManager::HideWidget(UUserWidget* Widget)
{
	if (!Widget) return;

	Widget->RemoveFromParent();

	// 4. 값(포인터)으로 키를 찾아 맵에서 제거
	const FName* KeyFound = ActiveWidgets.FindKey(Widget);
	if (KeyFound)
	{
		ActiveWidgets.Remove(*KeyFound);
	}
}

void UHUIManager::HideAllWidgets()
{
	for (auto& Pair : ActiveWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	ActiveWidgets.Empty();
}

TSubclassOf<class AHDamageTextActor> UHUIManager::GetDamageTextActorClass() const
{
	return UIDataAsset ? UIDataAsset->DamageTextActorClass : nullptr;
}
