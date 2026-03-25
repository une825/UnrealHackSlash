#include "UI/MainHud/HEquipGemSlotEntryUI.h"
#include "Components/Image.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"

void UHEquipGemSlotEntryUI::NativeOnListItemObjectSet(UObject* InListItemObject)
{
	UHEquipGemSlotEntryData* EntryData = Cast<UHEquipGemSlotEntryData>(InListItemObject);
	if (!EntryData || !EntryData->SupportGem) return;

	// 보조 젬 데이터에서 아이콘을 가져와 설정
	const FHGemData& GemData = EntryData->SupportGem->GetGemData();
	if (IconImage && GemData.GemIcon)
	{
		IconImage->SetBrushFromTexture(GemData.GemIcon);
	}
}
