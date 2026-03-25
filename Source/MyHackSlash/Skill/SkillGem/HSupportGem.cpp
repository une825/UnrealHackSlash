#include "Skill/SkillGem/HSupportGem.h"
#include "Skill/SkillGem/HMainGem.h"

bool UHSupportGem::CanAttach(const UHMainGem* InMainGem) const
{
	if (!InMainGem) return false;

	// 디자인 문서 규칙: 보조 젬 타입이 Common이면 무조건 가능
	if (GetGemType() == HEGemType::Common)
	{
		return true;
	}

	// 그 외에는 메인 젬과 타입이 일치해야 함 (Melee == Melee 등)
	return GetGemType() == InMainGem->GetGemType();
}
