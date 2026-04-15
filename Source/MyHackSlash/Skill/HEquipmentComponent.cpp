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
	SlotSupportGems.SetNum(4);         // 4개 슬롯 보조 젬 리스트 초기화
}

void UHEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

bool UHEquipmentComponent::EquipGem(int32 InSlotIndex, UHMainGem* InGem)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !InGem) return false;

	if (nullptr == ASC)
	{
		ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}

	// 1. 다른 슬롯에 이미 장착되어 있었다면 해제 (이동 처리)
	for (int32 i = 0; i < EquippedMainGems.Num(); ++i)
	{
		if (EquippedMainGems[i] == InGem)
		{
			UnequipGem(i, false);
			break;
		}
	}

	// 2. 인벤토리에서 제거
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->RemoveGemInstance(InGem);
		}
	}

	// 3. 현재 타겟 슬롯의 기존 메인 젬 해제
	UnequipGem(InSlotIndex, true);

	// 4. 새 메인 젬 장착
	EquippedMainGems[InSlotIndex] = InGem;

	// 5. 슬롯에 남아있던 보조 젬들을 새 메인 젬에 연결 시도
	// 호환되지 않는 보조 젬은 자동으로 인벤토리로 반환 처리
	TArray<UHSupportGem*> IncompatibleGems;
	for (UHSupportGem* SupportGem : SlotSupportGems[InSlotIndex].SupportGems)
	{
		if (SupportGem)
		{
			// 메인 젬에 연결 시도 (내부적으로 CanAttach 체크 수행)
			if (!InGem->AddSupportGem(SupportGem))
			{
				IncompatibleGems.Add(SupportGem);
			}
		}
	}

	// 호환되지 않는 젬들을 슬롯에서 제거하고 인벤토리로 반환
	for (UHSupportGem* IncompatibleGem : IncompatibleGems)
	{
		UnequipSupportGem(InSlotIndex, IncompatibleGem, true);
	}

	// 6. GAS 어빌리티 부여
	if (AMyHackSlashGameMode* GameMode = Cast<AMyHackSlashGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (UHGemDataAsset* GemCollection = GameMode->GetGemCollectionDataAsset())
		{
			TSubclassOf<UGameplayAbility> FindAbility = GemCollection->FindAbilityClassByTagName(InGem->GetGemData().AbilityTagName);
			if (ASC && FindAbility)
			{
				FGameplayAbilitySpec Spec(FindAbility);
				Spec.InputID = InSlotIndex + 1;
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
	if (!SlotSupportGems.IsValidIndex(InSlotIndex) || !InSupportGem) return false;

	// 1. 이미 다른 슬롯에 장착되어 있었다면 해제 (이동 처리)
	for (int32 i = 0; i < SlotSupportGems.Num(); ++i)
	{
		if (SlotSupportGems[i].SupportGems.Contains(InSupportGem))
		{
			UnequipSupportGem(i, InSupportGem, false);
			break;
		}
	}

	// 2. 최대 3개 제한 체크
	if (SlotSupportGems[InSlotIndex].SupportGems.Num() >= 3) return false;

	// 3. 슬롯 보조 젬 리스트에 추가
	SlotSupportGems[InSlotIndex].SupportGems.AddUnique(InSupportGem);

	// 4. 인벤토리에서 제거
	if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
	{
		if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
		{
			InvComp->RemoveGemInstance(InSupportGem);
		}
	}

	// 5. 현재 장착된 메인 젬이 있다면 연결 시도
	if (UHMainGem* MainGem = EquippedMainGems[InSlotIndex])
	{
		MainGem->AddSupportGem(InSupportGem);
	}

	OnEquipmentChanged.Broadcast();
	return true;
}

void UHEquipmentComponent::UpdateAbilitySpec(int32 InSlotIndex)
{
}

void UHEquipmentComponent::UnequipSupportGem(int32 InSlotIndex, UHSupportGem* InSupportGem, bool bInReturnToInventory)
{
	if (!SlotSupportGems.IsValidIndex(InSlotIndex) || !InSupportGem) return;

	// 1. 슬롯 보조 젬 리스트에서 제거
	SlotSupportGems[InSlotIndex].SupportGems.Remove(InSupportGem);

	// 2. 메인 젬이 있다면 거기서도 제거
	if (UHMainGem* MainGem = EquippedMainGems[InSlotIndex])
	{
		MainGem->RemoveSupportGem(InSupportGem);
	}

	// 3. 인벤토리로 반환
	if (bInReturnToInventory)
	{
		if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(GetOwner()))
		{
			if (UHGemInventoryComponent* InvComp = Player->GetGemInventoryComponent())
			{
				InvComp->AddGemInstance(InSupportGem);
			}
		}
	}

	OnEquipmentChanged.Broadcast();
}

void UHEquipmentComponent::UnequipGem(int32 InSlotIndex, bool bInReturnToInventory)
{
	if (!EquippedMainGems.IsValidIndex(InSlotIndex) || !EquippedMainGems[InSlotIndex]) return;

	UHMainGem* RemovedGem = EquippedMainGems[InSlotIndex];

	// 1. 보조 젬들은 슬롯에 남기고 메인 젬 내부에서만 제거
	for (UHSupportGem* SupportGem : SlotSupportGems[InSlotIndex].SupportGems)
	{
		RemovedGem->RemoveSupportGem(SupportGem);
	}

	// 2. GAS 연동: Ability 제거
	if (ASC && EquippedAbilityHandles.Contains(InSlotIndex))
	{
		ASC->ClearAbility(EquippedAbilityHandles[InSlotIndex]);
		EquippedAbilityHandles.Remove(InSlotIndex);
	}

	// 3. 장착 해제
	EquippedMainGems[InSlotIndex] = nullptr;

	// 4. 인벤토리로 반환
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

TArray<UHSupportGem*> UHEquipmentComponent::GetEquippedSupportGems(int32 InSlotIndex) const
{
	TArray<UHSupportGem*> Result;
	if (SlotSupportGems.IsValidIndex(InSlotIndex))
	{
		for (auto& Gem : SlotSupportGems[InSlotIndex].SupportGems)
		{
			Result.Add(Gem);
		}
	}
	return Result;
}
