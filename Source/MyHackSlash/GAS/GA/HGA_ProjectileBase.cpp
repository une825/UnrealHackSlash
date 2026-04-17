#include "GAS/GA/HGA_ProjectileBase.h"
#include "Skill/HProjectile.h"
#include "Skill/SkillGem/HMainGem.h"
#include "Skill/SkillGem/HSupportGem.h"
#include "System/HObjectPoolManager.h"
#include "Unit/HBaseCharacter.h"
#include "DataAsset/HUnitProfileData.h"

UHGA_ProjectileBase::UHGA_ProjectileBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

int32 UHGA_ProjectileBase::GetProjectileCount() const
{
	int32 TotalCount = 1;
	
	// SourceObject에서 MainGem 추출
	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (Spec)
	{
		if (const UHMainGem* MainGem = Cast<UHMainGem>(Spec->SourceObject))
		{
			for (const UHSupportGem* SupportGem : MainGem->GetSupportGems())
			{
				if (!SupportGem) continue;
				
				for (const FHSupportEffect& Effect : SupportGem->GetGemData().SupportEffects)
				{
					// 프로젝트 표준 태그 사용: Support.Projectile.Count
					if (Effect.SupportTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Support.Projectile.Count"))))
					{
						TotalCount += FMath::FloorToInt(Effect.ModifierValue);
					}
				}
			}
		}
	}
	
	return TotalCount;
}

void UHGA_ProjectileBase::SpawnProjectiles()
{
	if (!ProjectileClass) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	int32 ProjectileCount = GetProjectileCount();
	FVector BaseLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 50.0f;
	FRotator BaseRotation = Avatar->GetActorRotation();

	float StartAngle = -((ProjectileCount - 1) * SpreadAngle) / 2.0f;

	for (int32 i = 0; i < ProjectileCount; ++i)
	{
		FRotator SpawnRotation = BaseRotation;
		SpawnRotation.Yaw += (StartAngle + (i * SpreadAngle));

		CreateProjectileInstance(BaseLocation, SpawnRotation);
	}
}

AHProjectile* UHGA_ProjectileBase::CreateProjectileInstance(const FVector& InLocation, const FRotator& InRotation)
{
	UHObjectPoolManager* Pool = GetWorld()->GetSubsystem<UHObjectPoolManager>();
	if (!Pool) return nullptr;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	AHProjectile* Projectile = Cast<AHProjectile>(Pool->SpawnFromPool(ProjectileClass, InLocation, InRotation));

	if (Projectile)
	{
		Projectile->SetOwner(Avatar);
		Projectile->SetInstigator(Cast<APawn>(Avatar));
		Projectile->SetElement(Element);
		Projectile->DamageEffectClass = DamageEffectClass;

		float FinalDamage = BaseDamage;
		if (AHBaseCharacter* BaseChar = Cast<AHBaseCharacter>(Avatar))
		{
			FinalDamage += BaseChar->GetAttackDamage();
			if (BaseChar->GetUnitProfileData())
			{
				Projectile->SetOwningUnitType(BaseChar->GetUnitProfileData()->UnitType);
			}
		}

		Projectile->SetDamage(FinalDamage);

		// 보조 젬 정보 전달 (관통 등 처리용) - ResetProjectile 이전에 설정해야 Reset 내부 로직에서 참조 가능
		if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
		{
			Projectile->SetSourceObject(Spec->SourceObject.Get());
		}

		Projectile->ResetProjectile(InLocation, InRotation);
	}

	return Projectile;
}
