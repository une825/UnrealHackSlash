#include "UI/SelectAbilityPopupUI/HSelectAbilityPopupUI.h"
#include "UI/SelectAbilityPopupUI/HSelectAbilityEntryUI.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Kismet/GameplayStatics.h"

void UHSelectAbilityPopupUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 게임 시간 정지 (Global Time Dilation)
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
	
	// 2. 리스트 뷰 비우기 및 테스트 데이터 추가
	if (AbilityListView)
	{
		AbilityListView->ClearListItems();

		// 테스트 데이터 추가 (3개)
		for (int32 i = 1; i <= 3; ++i)
		{
			UHSelectAbilityData* NewData = NewObject<UHSelectAbilityData>(this);
			NewData->Title = FText::Format(NSLOCTEXT("UI", "AbilityTitle", "능력 {0}"), FText::AsNumber(i));
			NewData->Description = FText::Format(NSLOCTEXT("UI", "AbilityDesc", "공격력을 {0}% 증가시킵니다."), FText::AsNumber(i * 10));
			AbilityListView->AddItem(NewData);
		}
	}
}
