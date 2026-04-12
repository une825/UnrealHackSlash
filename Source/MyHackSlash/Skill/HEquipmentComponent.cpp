#include "Skill/HEquipmentComponent.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Skill/HGemInventoryComponent.h"
#include "Unit/Player/HPlayerCharacter.h"
#include "Mode/MyHackSlashGameMode.h"

UHEquipmentComponent::UHEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	EquippedMainGems.Init(nullptr, 4); // 4개 슬롯 초기화
}

void UHEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

}

bool UHEquipmentComponent::EquipGem(int32 InSlotIndex, UHMainGem* InGem)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !InGem) return false;

	// ASC가 유효하지 않으면 다시 시도
	if (nullptr == ASC)
	{
		ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}

	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->RemoveGemInstance(InGem);
		}
	}

	UnequipGem(InSlotIndex);

	EquippedMainGems[InSlotIndex] = InGem;

	if (AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UHGemDataAsset* GemCollection = GameMode->GetGemCollectionDataAsset())
		{
			TSubclassOf<UGameplayAbility> FindAbility = GemCollection->FindAbilityClassByTagName(InGem->GetGemData().AbilityTagName);
			if (ASC && FindAbility)
			{
				FGameplayAbilitySpec Spec(FindAbility);
				Spec.InputID = InSlotIndex + 1;
				
				// 핵심: 어빌리티의 소스 오브젝트를 메인 젬 인스턴스로 설정
				Spec.SourceObject = InGem;

				FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
				EquippedAbilityHandles.Add(InSlotIndex, Handle);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
	return true;
}

bool UHEquipmentComponent::EquipSupportGem(int32 InSlotIndex, UHSupportGem* InSupportGem)
{
	if (!InSupportGem) return false;

	UHMainGem* TargetMainGem = GetEquippedGem(InSlotIndex);
	if (!TargetMainGem) return false;

	if (TargetMainGem->AddSupportGem(InSupportGem))
	{
		if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
		{
			if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
			{
				InvComp->RemoveGemInstance(InSupportGem);
			}
		}

		// SourceObject(메인젬)의 내부 데이터가 변경되었으므로 별도의 Spec 업데이트 로직 없이도 
		// GA가 실행될 때 변경된 값을 참조하게 됩니다.
		OnEquipmentChanged.Broadcast();
		return true;
	}

	return false;
}

void UHEquipmentComponent::UpdateAbilitySpec(int32 InSlotIndex)
{
    // SourceObject 방식을 사용하므로 더 이상 필요하지 않습니다. 제거하거나 비워둡니다.
}

void UHEquipmentComponent::UnequipGem(int32 InSlotIndex, bool bInReturnToInventory)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !EquippedMainGems[InSlotIndex]) return;

	UHMainGem* RemovedGem = EquippedMainGems[InSlotIndex];

	// GAS 연동: Ability 제거
	if (ASC && EquippedAbilityHandles.Contains(InSlotIndex))
	{
		ASC->ClearAbility(EquippedAbilityHandles[InSlotIndex]);
		EquippedAbilityHandles.Remove(InSlotIndex);
	}

	EquippedMainGems[InSlotIndex] = nullptr;

	// 해제된 젬을 다시 인벤토리 가방으로 돌려보냄 (파라미터가 true일 때만)
	if (bInReturnToInventory)
	{
		if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
		{
			if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
			{
				InvComp->AddGemInstance(RemovedGem);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
}

UHMainGem* UHEquipmentComponent::GetEquippedGem(int32 InSlotIndex) const
{
	if (EquippedMainGems.IsValidIndex(InSlotIndex))
	{
		return EquippedMainGems[InSlotIndex];
	}
	return nullptr;
}
