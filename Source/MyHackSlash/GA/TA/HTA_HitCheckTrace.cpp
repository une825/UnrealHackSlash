// Source/MyHackSlash/GA/TA/HTA_HitCheckTrace.cpp

#include "GA/TA/HTA_HitCheckTrace.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attribute/HCharacterAttributeSet.h"

AHTA_HitCheckTrace::AHTA_HitCheckTrace()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHTA_HitCheckTrace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	
	SourceActor = Ability->GetAvatarActorFromActorInfo();
}

void AHTA_HitCheckTrace::ConfirmTargetingAndContinue()
{
	if (SourceActor)
	{
		FGameplayAbilityTargetDataHandle DataHandle = MakeTargetData();
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
}

FGameplayAbilityTargetDataHandle AHTA_HitCheckTrace::MakeTargetData() const
{
	FGameplayAbilityTargetDataHandle DataHandle;

	if (!SourceActor) return DataHandle;

	// AbilitySystemComponent를 통해 AttributeSet에서 스탯 가져오기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	if (!ASC) return DataHandle;

	const UHCharacterAttributeSet* AttributeSet = ASC->GetSet<UHCharacterAttributeSet>();
	if (!AttributeSet) return DataHandle;

	const float TraceDistance = AttributeSet->GetAttackRange();
	const float TraceRadius = AttributeSet->GetAttackRadius();

	float CapsuleRadius = 0.0f;
	if (const ACharacter* Character = Cast<ACharacter>(SourceActor))
	{
		CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}

	const FVector Forward = SourceActor->GetActorForwardVector();
	const FVector Start = SourceActor->GetActorLocation() + (Forward * CapsuleRadius);
	const FVector End = Start + (Forward * TraceDistance);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HitCheckTA));
	Params.AddIgnoredActor(SourceActor);

	// Sphere Sweep을 통한 타겟 감지
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit();
			NewData->HitResult = Hit;
			DataHandle.Add(NewData);
		}
	}

#if ENABLE_DRAW_DEBUG
	//const FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	//const float CapsuleHalfLength = (TraceDistance * 0.5f);
	//const FColor DrawColor = bHit ? FColor::Green : FColor::Red;

	//DrawDebugCapsule(
	//	GetWorld(),
	//	CapsuleOrigin,
	//	CapsuleHalfLength + TraceRadius,
	//	TraceRadius,
	//	FRotationMatrix::MakeFromZ(Forward).ToQuat(),
	//	DrawColor,
	//	false,
	//	1.0f
	//);
#endif

	return DataHandle;
}
