#include "UI/MainHud/HGemInventoryUI.h"
#include "UI/MainHud/HGemInventoryEntryUI.h"
#include "Components/ListView.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HGemInventoryComponent.h"
#include "Skill/SkillGem/HGemBase.h"

void UHGemInventoryUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 10개의 슬롯 데이터 객체 미리 생성 (한 번만 수행)
	InventorySlotDatas.Empty();
	for (int32 i = 0; i < 10; ++i)
	{
		InventorySlotDatas.Add(NewObject<UHGemInventoryEntryData>(this));
	}

	if (AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetOwningPlayerPawn()))
	{
		if (UHGemInventoryComponent* GemInventory = PlayerCharacter->GetGemInventoryComponent())
		{
			// 인벤토리 변경 시 UI 갱신 바인딩
			GemInventory->OnGemInventoryUpdated.AddDynamic(this, &UHGemInventoryUI::RefreshInventory);

			// 초기 리스트 갱신
			RefreshInventory();
		}
	}
}

void UHGemInventoryUI::RefreshInventory()
{
	if (!InventoryListView) return;

	InventoryListView->ClearListItems();

	if (AHPlayerCharacter* PlayerCharacter = Cast<AHPlayerCharacter>(GetOwningPlayerPawn()))
	{
		if (UHGemInventoryComponent* GemInventory = PlayerCharacter->GetGemInventoryComponent())
		{
			const TArray<UHGemBase*>& Gems = GemInventory->GetInventoryGems();
			
			// 인벤토리 컴포넌트의 젬 정보를 10개의 슬롯 데이터에 매핑하여 리스트 채우기
			for (int32 i = 0; i < 10; ++i)
			{
				if (Gems.IsValidIndex(i))
				{
					InventorySlotDatas[i]->GemBase = Gems[i];
					InventorySlotDatas[i]->bIsEmpty = false;
				}
				else
				{
					InventorySlotDatas[i]->GemBase = nullptr;
					InventorySlotDatas[i]->bIsEmpty = true;
				}
				
				InventoryListView->AddItem(InventorySlotDatas[i]);
			}
		}
	}

	InventoryListView->RegenerateAllEntries();
}
