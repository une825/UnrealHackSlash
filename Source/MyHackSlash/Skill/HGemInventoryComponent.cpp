#include "Skill/HGemInventoryComponent.h"
#include "Skill/SkillGem/HGemBase.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "DataAsset/HGemDataAsset.h"

UHGemInventoryComponent::UHGemInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHGemInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventoryGems.Empty();
}

UHGemBase* UHGemInventoryComponent::AddGem(const FHGemData& InGemData)
{
	if (InGemData.GemID == NAME_None) return nullptr;

	// 카테고리에 맞는 클래스 결정
	TSubclassOf<UHGemBase> GemClass = (InGemData.GemCategory == HEGemCategory::Main) 
		? UHMainGem::StaticClass() : UHSupportGem::StaticClass();

	// 인스턴스 생성 및 초기화
	UHGemBase* NewGem = NewObject<UHGemBase>(this, GemClass);
	NewGem->Initialize(InGemData);

	// 인벤토리에 새 젬 추가
	InventoryGems.Add(NewGem);

	OnGemInventoryUpdated.Broadcast();

	return NewGem;
}

void UHGemInventoryComponent::AddGemInstance(UHGemBase* InGemInstance)
{
	if (InGemInstance && !InventoryGems.Contains(InGemInstance))
	{
		InventoryGems.Add(InGemInstance);
		OnGemInventoryUpdated.Broadcast();
	}
}

void UHGemInventoryComponent::RemoveGemInstance(UHGemBase* InGemInstance)
{
	if (InGemInstance && InventoryGems.Contains(InGemInstance))
	{
		InventoryGems.Remove(InGemInstance);
		OnGemInventoryUpdated.Broadcast();
	}
}
