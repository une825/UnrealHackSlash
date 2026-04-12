#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "System/HGlobalTypes.h"
#include "HGA_ProjectileBase.generated.h"

class AHProjectile;
class UHMainGem;

/**
 * @brief 투사체를 발사하는 모든 스킬의 기본이 되는 베이스 어빌리티 클래스입니다.
 * 보조 젬(Support Gem)의 효과(개수 증가, 속성 변경 등)를 공통으로 처리합니다.
 */
UCLASS()
class MYHACKSLASH_API UHGA_ProjectileBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHGA_ProjectileBase();

protected:
	/** @brief 보조 젬 효과를 포함한 최종 투사체 개수를 계산합니다. */
	int32 GetProjectileCount() const;

	/** @brief 투사체들을 부채꼴 모양으로 발사합니다. */
	virtual void SpawnProjectiles();

	/** @brief 개별 투사체를 생성하고 초기화합니다. (필요 시 하위 클래스에서 오버라이드) */
	virtual AHProjectile* CreateProjectileInstance(const FVector& InLocation, const FRotator& InRotation);

protected:
	/** @brief 발사할 투사체 클래스 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AHProjectile> ProjectileClass;

	/** @brief 기본 투사체 데미지 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float BaseDamage = 10.0f;

	/** @brief 투사체 간의 발사 간격 각도 (부채꼴) */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	float SpreadAngle = 15.0f;

	/** @brief 애니메이션 재생용 태그 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	FGameplayTag MontageTag;

	/** @brief 데미지 적용 시 사용할 GameplayEffect 클래스 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	/** @brief 현재 스킬의 속성 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	HEElement Element = HEElement::Physical;
};
