#include "Skill/HEquipmentComponent.h"
#include "Skill/SkillGem/HMainGem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Skill/HGemInventoryComponent.h"
#include "Unit/Player/HPlayerCharacter.h"

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

	// ASC가 유효하지 않으면 다시 시도 (PossessedBy 등 초기화 시점 대응)
	if (nullptr == ASC)
	{
		ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}

	// 기존 슬롯 해제
	UnequipGem(InSlotIndex);

	// 새로운 젬 장착
	EquippedMainGems[InSlotIndex] = InGem;

	// GAS 연동: Ability 부여
	if (ASC && InGem->GetGemData().SkillAbilityClass)
	{
		FGameplayAbilitySpec Spec(InGem->GetGemData().SkillAbilityClass);
		// 슬롯 인덱스 0은 InputID 1 (기본 공격), 1은 InputID 2 ...
		Spec.InputID = InSlotIndex + 1; 
		
		EquippedAbilityHandles.Add(InSlotIndex, ASC->GiveAbility(Spec));
	}

	OnEquipmentChanged.Broadcast();
	return true;
}

void UHEquipmentComponent::UnequipGem(int32 InSlotIndex)
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

	// 해제된 젬을 다시 인벤토리 가방으로 돌려보냄
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->AddGemInstance(RemovedGem);
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
