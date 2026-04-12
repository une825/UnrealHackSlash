#include "GAS/GA/HGA_SpinningBlades.h"
#include "Skill/HProjectile_SpinningBlade.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "System/HObjectPoolManager.h"

UHGA_SpinningBlades::UHGA_SpinningBlades()
	: BladeCount(3), OrbitRadius(200.0f), RotationSpeed(180.0f), Duration(5.0f)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UHGA_SpinningBlades::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("UHGA_SpinningBlades: ActivateAbility Started."));

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHGA_SpinningBlades: CommitAbility Failed (Cooldown or Cost)."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (!AvatarActor || !ProjectileClass || !DamageEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UHGA_SpinningBlades: Missing required data! Avatar: %s, ProjectileClass: %s, DamageEffectClass: %s"),
			AvatarActor ? *AvatarActor->GetName() : TEXT("Null"),
			ProjectileClass ? *ProjectileClass->GetName() : TEXT("Null"),
			DamageEffectClass ? *DamageEffectClass->GetName() : TEXT("Null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	float AngleStep = 360.0f / BladeCount;

	UE_LOG(LogTemp, Log, TEXT("UHGA_SpinningBlades: Spawning %d blades with AngleStep %f."), BladeCount, AngleStep);

	for (int32 i = 0; i < BladeCount; ++i)
	{
		SpawnProjectile(i, AngleStep, DamageSpecHandle);
	}

	FTimerHandle EndTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(EndTimerHandle, [this, Handle, ActorInfo, ActivationInfo]()
	{
		UE_LOG(LogTemp, Log, TEXT("UHGA_SpinningBlades: Duration ended, calling EndAbility."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}, Duration, false);
}

void UHGA_SpinningBlades::SpawnProjectile(int32 Index, float AngleStep, const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	if (ProjectileClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UHGA_SpinningBlades: SpawnProjectile[%d] failed - ProjectileClass is Null."), Index);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (AvatarActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UHGA_SpinningBlades: SpawnProjectile[%d] failed - AvatarActor is Null."), Index);
		return;
	}

	UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
	if (Pool == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UHGA_SpinningBlades: SpawnProjectile[%d] failed - Pool Manager is Null."), Index);
		return;
	}

	float InitialAngle = Index * AngleStep;
	float RadAngle = FMath::DegreesToRadians(InitialAngle);
	FVector SpawnLoc = AvatarActor->GetActorLocation() + FVector(FMath::Cos(RadAngle) * OrbitRadius, FMath::Sin(RadAngle) * OrbitRadius, 0.0f);

	UE_LOG(LogTemp, Log, TEXT("UHGA_SpinningBlades: Attempting to spawn blade[%d] at %s (InitialAngle: %f)"), Index, *SpawnLoc.ToString(), InitialAngle);

	AActor* PooledActor = Pool->SpawnFromPool(ProjectileClass, SpawnLoc, FRotator::ZeroRotator);
	if (PooledActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UHGA_SpinningBlades: SpawnProjectile[%d] failed - SpawnFromPool returned Null."), Index);
		return;
	}

	AHProjectile_SpinningBlade* Projectile = Cast<AHProjectile_SpinningBlade>(PooledActor);
	if (Projectile)
	{
		Projectile->SetOwner(AvatarActor);
		Projectile->SetInstigator(Cast<APawn>(AvatarActor));
		Projectile->Initialize(AvatarActor, OrbitRadius, RotationSpeed, InitialAngle, DamageSpecHandle);
		Projectile->SetProjectileLifeSpan(Duration);
		Projectile->ResetProjectile(SpawnLoc, FRotator::ZeroRotator);

		UE_LOG(LogTemp, Log, TEXT("UHGA_SpinningBlades: Successfully initialized blade[%d]."), Index);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UHGA_SpinningBlades: SpawnProjectile[%d] failed - Cast to AHProjectile_SpinningBlade failed (Class: %s)."), 
			Index, *PooledActor->GetClass()->GetName());
	}
}
