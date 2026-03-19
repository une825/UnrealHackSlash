#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "HUIManager.generated.h"

class UHUIDataAsset;

/**
 * 게임의 UI를 전역적으로 관리하는 매니저 클래스입니다.
 * 게임 인스턴스 서브시스템을 상속받아 전역적으로 유지됩니다.
 */
UCLASS()
class MYHACKSLASH_API UHUIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHUIManager();

	// 서브시스템 초기화 시 호출됩니다.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// UI 이름을 기반으로 위젯을 띄웁니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* ShowWidgetByName(FName InWidgetName, int32 InZOrder = 0);

	// 활성화된 위젯 중 특정 이름을 가진 위젯을 찾아 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* GetWidgetByName(FName InWidgetName) const;

	// 기존 ShowWidget (클래스를 직접 넘기는 경우)
	UFUNCTION(BlueprintCallable, Category = "UI")
	UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0);

	// 현재 활성화된 특정 위젯을 닫습니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideWidget(UUserWidget* Widget);

	// 모든 위젯을 닫습니다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllWidgets();

	// 특정 타입의 위젯을 찾아서 반환합니다.
	template <typename T>
	T* GetWidget() const
	{
		for (auto& Pair : ActiveWidgets)
		{
			if (T* TypedWidget = Cast<T>(Pair.Value))
			{
				return TypedWidget;
			}
		}
		return nullptr;
	}

private:
	// UI 정보를 담고 있는 데이터 에셋
	UPROPERTY()
	TObjectPtr<UHUIDataAsset> UIDataAsset;

	// 현재 화면에 출력 중인 위젯 목록을 이름(Key)과 함께 관리합니다.
	UPROPERTY()
	TMap<FName, TObjectPtr<UUserWidget>> ActiveWidgets;
};
