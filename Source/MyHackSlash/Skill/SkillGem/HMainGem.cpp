#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"

bool UHMainGem::AddSupportGem(UHSupportGem* InSupportGem)
{
	if (!InSupportGem || !InSupportGem->CanAttach(this))
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
