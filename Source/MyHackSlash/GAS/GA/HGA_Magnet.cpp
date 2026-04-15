#include "GAS/GA/HGA_Magnet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UHGA_Magnet::UHGA_Magnet()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 'State.MagnetActive' 태그가 있을 때만 활성화하거나 작동하도록 설정할 수 있습니다.
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Magnet")));
}

void UHGA_Magnet::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		GetWorld()->GetTimerManager().SetTimer(MagnetTimerHandle, this, &UHGA_Magnet::TickMagnet, TickInterval, true);
	}
}

void UHGA_Magnet::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(MagnetTimerHandle);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHGA_Magnet::TickMagnet()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	// 태그가 없으면 즉시 종료
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.MagnetActive"))))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FVector Origin = AvatarActor->GetActorLocation();
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(AvatarActor);

	TArray<AActor*> OutActors;
	// PhysicsActor(보통 4번 또는 5번 채널)를 포함하도록 오브젝트 타입 확장
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Origin,
		MagnetRadius,
		{ EObjectTypeQuery::ObjectTypeQuery1, EObjectTypeQuery::ObjectTypeQuery2, EObjectTypeQuery::ObjectTypeQuery4 }, 
		AActor::StaticClass(),
		IgnoreActors,
		OutActors
	);

	for (AActor* Item : OutActors)
	{
		// 아이템에 'Item.Currency' 태그가 있거나 특정 클래스인 경우만 끌어당김
		if (Item && Item->ActorHasTag(TEXT("Item.Currency")))
		{
			FVector Direction = (Origin - Item->GetActorLocation()).GetSafeNormal();
			float Distance = FVector::Dist(Origin, Item->GetActorLocation());

			// 플레이어에게 가까워질수록 더 빠르게 끌어당김 (선택 사항)
			FVector NewLocation = Item->GetActorLocation() + (Direction * PullSpeed * TickInterval);
			Item->SetActorLocation(NewLocation, true);
		}
	}
}
