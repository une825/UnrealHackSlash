// Source/MyHackSlash/UI/ShopUI/HShopEntryUI.cpp

#include "UI/ShopUI/HShopEntryUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Unit/Player/HPlayerState.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Skill/HGemInventoryComponent.h"
#include "DataAsset/HGemDataAsset.h"
#include "Mode/MyHackSlashGameMode.h"
#include "AbilitySystemComponent.h"
#include "Attribute/HCharacterAttributeSet.h"

void UHShopEntryUI::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	CachedData = Cast<UHShopEntryData>(ListItemObject);
	if (!CachedData.IsValid()) return;

	const FHShopRow& Row = CachedData->ItemRowData;

	if (TitleText) TitleText->SetText(Row.Title);
	if (DescText) DescText->SetText(Row.Description);
	if (CurrencyText) CurrencyText->SetText(FText::AsNumber(Row.Price));

	if (IconImage)
	{
		IconImage->SetBrushFromSoftTexture(Row.Icon);
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.Clear();
		SelectButton->OnClicked.AddDynamic(this, &UHShopEntryUI::OnSelectButtonClicked);
	}
}

void UHShopEntryUI::OnSelectButtonClicked()
{
	if (!CachedData.IsValid()) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AHPlayerState* PS = PC->GetPlayerState<AHPlayerState>();
	if (!PS) return;

	const FHShopRow& Row = CachedData->ItemRowData;

	// 1. 골드 체크 및 소비
	if (!PS->ConsumeGold(Row.Price))
	{
		// TODO: 골드 부족 UI 알림 (예: 텍스트 흔들기 등)
		UE_LOG(LogTemp, Warning, TEXT("Not enough gold to buy %s"), *Row.Title.ToString());
		return;
	}

	// 2. 상품 타입에 따른 효과 적용
	AHPlayerCharacter* PlayerChar = Cast<AHPlayerCharacter>(PC->GetPawn());
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();

	switch (Row.ItemType)
	{
	case EHShopItemType::Ability:
		if (ASC)
		{
			// 태그에 매칭되는 Attribute를 찾아 무한 지속 Gameplay Effect 부여
			FGameplayAttribute Attribute;
			FString TagName = Row.ItemTag.ToString();

			if (TagName.Contains(TEXT("AttackDamage"))) Attribute = UHCharacterAttributeSet::GetAttackDamageAttribute();
			else if (TagName.Contains(TEXT("MaxHealth"))) Attribute = UHCharacterAttributeSet::GetMaxHealthAttribute();
			else if (TagName.Contains(TEXT("MovementSpeed"))) Attribute = UHCharacterAttributeSet::GetMovementSpeedAttribute();
			else if (TagName.Contains(TEXT("CriticalRate"))) Attribute = UHCharacterAttributeSet::GetCriticalRateAttribute();

			if (Attribute.IsValid())
			{
				// 무한 지속 Gameplay Effect 동적 생성 및 설정
				UGameplayEffect* GE_Bonus = NewObject<UGameplayEffect>(GetTransientPackage());
				GE_Bonus->DurationPolicy = EGameplayEffectDurationType::Infinite;

				// Modifier 추가 (능력치 가산)
				int32 ModIdx = GE_Bonus->Modifiers.Num();
				GE_Bonus->Modifiers.Add(FGameplayModifierInfo());
				FGameplayModifierInfo& ModInfo = GE_Bonus->Modifiers[ModIdx];
				ModInfo.Attribute = Attribute;
				ModInfo.ModifierOp = EGameplayModOp::Additive;
				ModInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Row.EffectValue));

				// 자신에게 적용
				ASC->ApplyGameplayEffectToSelf(GE_Bonus, 1.0f, ASC->MakeEffectContext());
			}
		}
		break;

	case EHShopItemType::InstantEffect:
		if (ASC)
		{
			// 즉시 회복 효과 처리
			if (Row.ItemTag.ToString().Contains(TEXT("RestoreHealth")))
			{
				float CurrentHP = ASC->GetNumericAttribute(UHCharacterAttributeSet::GetHealthAttribute());
				float MaxHP = ASC->GetNumericAttribute(UHCharacterAttributeSet::GetMaxHealthAttribute());
				ASC->SetNumericAttributeBase(UHCharacterAttributeSet::GetHealthAttribute(), FMath::Min(CurrentHP + Row.EffectValue, MaxHP));
			}
		}
		break;

	case EHShopItemType::SkillGem:
		if (PlayerChar)
		{
			if (UHGemInventoryComponent* Inventory = PlayerChar->GetGemInventoryComponent())
			{
				if (AMyHackSlashGameMode* GM = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()))
				{
					if (UHGemDataAsset* GemCollection = GM->GetGemCollectionDataAsset())
					{
						// ItemTag의 마지막 부분을 GemID로 사용하여 1티어 젬 데이터 찾기
						FString TagName = Row.ItemTag.ToString();
						int32 LastDotIndex;
						TagName.FindLastChar('.', LastDotIndex);
						FString GemIDStr = (LastDotIndex != INDEX_NONE) ? TagName.RightChop(LastDotIndex + 1) : TagName;
						
						FName FullID = FName(*FString::Printf(TEXT("%s_T1"), *GemIDStr));
						FHGemData GemData;
						if (GemCollection->FindGemData(FullID, GemData))
						{
							Inventory->AddGem(GemData);
						}
					}
				}
			}
		}
		break;

	default:
		break;
	}

	// TODO: 구매 완료 사운드 및 이펙트 호출
	UE_LOG(LogTemp, Log, TEXT("Successfully purchased %s"), *Row.Title.ToString());
}
