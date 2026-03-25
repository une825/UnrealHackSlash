#include "UI/SelectAbilityPopupUI/HSelectAbilityPopupUI.h"
#include "UI/SelectAbilityPopupUI/HSelectAbilityEntryUI.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "System/HSelectAbilityManager.h"
#include "Kismet/GameplayStatics.h"

void UHSelectAbilityPopupUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 게임 일시정지 (뱀서류 필수 로직)
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 2. 보상 옵션 갱신
	RefreshOptions();
}

void UHSelectAbilityPopupUI::NativeDestruct()
{
	// 3. 게임 일시정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	Super::NativeDestruct();
}

void UHSelectAbilityPopupUI::RefreshOptions()
{
	if (!AbilityListView) return;

	AbilityListView->ClearListItems();

	if (UHSelectAbilityManager* Manager = GetGameInstance()->GetSubsystem<UHSelectAbilityManager>())
	{
		TArray<FHRewardOptionData> SelectedOptions;
		if (Manager->GetRandomRewardOptions(SelectedOptions))
		{
			for (const FHRewardOptionData& OptionData : SelectedOptions)
			{
				// ListView에 추가하기 위해 UObject 래퍼 생성
				UHSelectAbilityData* NewData = NewObject<UHSelectAbilityData>(this);
				if (NewData)
				{
					NewData->SetRewardOptionData(OptionData);
					AbilityListView->AddItem(NewData);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UHSelectAbilityPopupUI: Failed to get reward options from manager."));
		}
	}
}
