#include "UI/SelectAbilityPopupUI/HSelectAbilityPopupUI.h"
#include "UI/SelectAbilityPopupUI/HSelectAbilityEntryUI.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Components/Button.h"
#include "Mode/HGameState.h"
#include "Mode/MyHackSlashPlayerController.h"
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

	if (UHSelectAbilityManager* Manager = GetGameInstance()->GetSubsystem<UHSelectAbilityManager>())
	{
		if (!Manager->IsRewardDataReady())
		{
			if (AHGameState* HGameState = GetWorld() ? GetWorld()->GetGameState<AHGameState>() : nullptr)
			{
				Manager->InitializeManager(
					HGameState->GetSelectAbilityGradeDataAsset(),
					HGameState->GetSelectAbilityRewardDataTable(),
					HGameState->GetGemCollectionDataAsset(),
					nullptr
				);
			}
		}
	}

	if (AMyHackSlashPlayerController* PC = Cast<AMyHackSlashPlayerController>(GetOwningPlayer()))
	{
		OptionsChangedHandle = PC->OnSelectAbilityOptionsChanged.AddUObject(this, &UHSelectAbilityPopupUI::HandleSelectAbilityOptionsChanged);
	}

	// 3. 보상 옵션 갱신
	RefreshOptions();
}

void UHSelectAbilityPopupUI::NativeDestruct()
{
	if (AMyHackSlashPlayerController* PC = Cast<AMyHackSlashPlayerController>(GetOwningPlayer()))
	{
		PC->OnSelectAbilityOptionsChanged.Remove(OptionsChangedHandle);
	}

	// 4. 게임 일시정지 해제
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	Super::NativeDestruct();
}

void UHSelectAbilityPopupUI::OnRefreshButtonClicked()
{
	if (AMyHackSlashPlayerController* PC = Cast<AMyHackSlashPlayerController>(GetOwningPlayer()))
	{
		PC->RequestRefreshSelectAbilityOptions();
	}
}

void UHSelectAbilityPopupUI::RefreshOptions()
{
	if (!AbilityListView) return;

	AbilityListView->ClearListItems();

	AMyHackSlashPlayerController* PC = Cast<AMyHackSlashPlayerController>(GetOwningPlayer());
	if (!PC) return;

	// 1. 보상 선택지 생성 및 리스트 갱신
	const TArray<FHRewardOptionEntry>& SelectedOptions = PC->GetSelectAbilityOptions();
	for (const FHRewardOptionEntry& OptionEntry : SelectedOptions)
	{
		UHSelectAbilityData* NewData = NewObject<UHSelectAbilityData>(this);
		if (NewData)
		{
			NewData->SetRewardOptionEntry(OptionEntry);
			AbilityListView->AddItem(NewData);
		}
	}

	// 2. 새로고침 관련 UI 갱신
	if (RefreshCountText)
	{
		RefreshCountText->SetText(FText::AsNumber(PC->GetSelectAbilityRefreshCount()));
	}

	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(PC->CanRefreshSelectAbilityOptions());
	}
}

void UHSelectAbilityPopupUI::HandleSelectAbilityOptionsChanged()
{
	RefreshOptions();
}
