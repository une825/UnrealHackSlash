#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"

bool UHMainGem::AddSupportGem(UHSupportGem* InSupportGem)
{
	if (!InSupportGem || !InSupportGem->CanAttach(this))
	{
		return false;
	}

	// 최대 3개 보조 젬 슬롯 제한
	if (SupportGems.Num() >= 3)
	{
		return false;
	}

	SupportGems.AddUnique(InSupportGem);
	return true;
}

void UHMainGem::RemoveSupportGem(UHSupportGem* InSupportGem)
{
	if (InSupportGem)
	{
		SupportGems.Remove(InSupportGem);
	}
}
