#include "UI/SelectAbilityPopupUI/HSelectAbilityPopupUI.h"
#include "UI/SelectAbilityPopupUI/HSelectAbilityEntryUI.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Components/Button.h"
#include "System/HSelectAbilityManager.h"
#include "Kismet/GameplayStatics.h"

void UHSelectAbilityPopupUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 게임 일시정지 (뱀서류 필수 로직)
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// 2. 새로고침 버튼 이벤트 바인딩
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UHSelectAbilityPopupUI::OnRefreshButtonClicked);
	}

	// 3. 보상 옵션 갱신
	RefreshOptions();
}

void UHSelectAbilityPopupUI::NativeDestruct()
{
	// 4. 게임 일시정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	Super::NativeDestruct();
}

void UHSelectAbilityPopupUI::OnRefreshButtonClicked()
{
	if (UHSelectAbilityManager* Manager = GetGameInstance()->GetSubsystem<UHSelectAbilityManager>())
	{
		if (Manager->CanRefresh())
		{
			Manager->ConsumeRefresh();
			RefreshOptions();
		}
	}
}

void UHSelectAbilityPopupUI::RefreshOptions()
{
	if (!AbilityListView) return;

	AbilityListView->ClearListItems();

	UHSelectAbilityManager* Manager = GetGameInstance()->GetSubsystem<UHSelectAbilityManager>();
	if (!Manager) return;

	// 1. 보상 선택지 생성 및 리스트 갱신
	TArray<FHRewardOptionData> SelectedOptions;
	if (Manager->GetRandomRewardOptions(SelectedOptions))
	{
		for (const FHRewardOptionData& OptionData : SelectedOptions)
		{
			UHSelectAbilityData* NewData = NewObject<UHSelectAbilityData>(this);
			if (NewData)
			{
				NewData->SetRewardOptionData(OptionData);
				AbilityListView->AddItem(NewData);
			}
		}
	}

	// 2. 새로고침 관련 UI 갱신
	if (RefreshCountText)
	{
		RefreshCountText->SetText(FText::AsNumber(Manager->GetCurrentRefreshCount()));
	}

	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(Manager->CanRefresh());
	}
}
