#include "UI/MainHud/HMainHudUI.h"

void UHMainHudUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 나중에 UI가 화면에 나타날 때 필요한 초기화(버튼 바인딩 등)를 수행합니다.
	UE_LOG(LogTemp, Log, TEXT("HMainHudUI: Main HUD Widget Constructed!"));
}
