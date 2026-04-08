// Source/MyHackSlash/Skill/AnimNotifyState/HAnimNotifyState_AttackHitCheck.cpp

#include "Skill/AnimNotifyState/HAnimNotifyState_AttackHitCheck.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "System/HSoundManager.h"

UHAnimNotifyState_AttackHitCheck::UHAnimNotifyState_AttackHitCheck()
{
	// 기본 태그 설정
	TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Event.Hit"));
}

FString UHAnimNotifyState_AttackHitCheck::GetNotifyName_Implementation() const
{
	return TEXT("AttackHitCheck_Event");
}

void UHAnimNotifyState_AttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World || !World->IsGameWorld()) return; // 게임 월드(PIE 포함)가 아니면 무시

	AActor* Owner = MeshComp->GetOwner();

	// 1. GAS 이벤트 데이터 전송
	FGameplayEventData Payload;
	Payload.Instigator = Owner;
	Payload.EventTag = TriggerTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, TriggerTag, Payload);

	// 2. 사운드 재생
	if (!PlaySoundName.IsNone())
	{
		if (UHSoundManager* SoundManager = World->GetSubsystem<UHSoundManager>())
		{
			SoundManager->PlaySFXByKey(PlaySoundName, Owner->GetActorLocation());
		}
	}
}

