#include "GAS/GA/HGA_SpinningBlades.h"
#include "Skill/HProjectile_SpinningBlade.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"

UHGA_SpinningBlades::UHGA_SpinningBlades()
	: BladeCount(3), OrbitRadius(200.0f), RotationSpeed(180.0f), Duration(5.0f)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_SpinningBlades::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (AvatarActor && ProjectileClass && DamageEffectClass)
	{
		// 1. 데미지 적용을 위한 SpecHandle 생성
		FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
		
		// 필요한 경우 여기서 SetByCaller로 데미지 배율 등 추가 데이터 전달 가능
		// DamageSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.DamageMultiplier")), 1.0f);

		// 2. 균등한 간격으로 칼날 생성
		float AngleStep = 360.0f / BladeCount;

		for (int32 i = 0; i < BladeCount; ++i)
		{
			float InitialAngle = i * AngleStep;
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = AvatarActor;
			SpawnParams.Instigator = Cast<APawn>(AvatarActor);

			AHProjectile_SpinningBlade* Projectile = GetWorld()->SpawnActor<AHProjectile_SpinningBlade>(
				ProjectileClass, 
				AvatarActor->GetActorLocation(), 
				FRotator::ZeroRotator, 
				SpawnParams);

			if (Projectile)
			{
				Projectile->Initialize(AvatarActor, OrbitRadius, RotationSpeed, InitialAngle, DamageSpecHandle);
				// 수명 설정 (어빌리티 지속 시간과 동기화)
				Projectile->SetLifeSpan(Duration);
			}
		}
	}

	// 3. 지속 시간 후 어빌리티 종료 처리
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Handle, ActorInfo, ActivationInfo]()
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}, Duration, false);
}
