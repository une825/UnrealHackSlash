#include "UI/HDamageTextWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UHDamageTextWidget::SetDamage(float InDamage, bool bInIsCritical)
{
	if (DamageText)
	{
		// 1. 데미지 텍스트 설정 (소수점 없이 정수로 표시)
		int32 DamageValue = FMath::RoundToInt(InDamage);
		DamageText->SetText(FText::AsNumber(DamageValue));

		// 2. 크리티컬 여부에 따라 색상 및 윤곽선 색상 변경
		FSlateFontInfo FontInfo = DamageText->GetFont();

		if (bInIsCritical)
		{
			DamageText->SetColorAndOpacity(FSlateColor(CriticalColor));
			FontInfo.OutlineSettings.OutlineColor = CriticalOutlineColor;
		}
		else
		{
			DamageText->SetColorAndOpacity(FSlateColor(NormalColor));
			FontInfo.OutlineSettings.OutlineColor = NormalOutlineColor;
		}

		DamageText->SetFont(FontInfo);
	}

	// 3. 애니메이션 재생 (블루프린트에서 'ShowAnimation'이라는 이름으로 애니메이션을 만들었다면 실행됨)
	if (ShowAnimation)
	{
		PlayAnimation(ShowAnimation);
	}
}
