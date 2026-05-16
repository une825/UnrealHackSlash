#include "Skill/SkillGem/HGemBase.h"

UHGemBase::UHGemBase()
{
}

void UHGemBase::Initialize(const FHGemData& InGemData)
{
	GemData = InGemData;
	if (!InstanceId.IsValid())
	{
		InstanceId = FGuid::NewGuid();
	}
}
