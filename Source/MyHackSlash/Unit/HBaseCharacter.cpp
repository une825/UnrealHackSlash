// Fill out your copyright notice in the Description page of Project Settings.


#include "Unit/HBaseCharacter.h"

#include "UObject/ConstructorHelpers.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"


// Sets default values
AHBaseCharacter::AHBaseCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("HCapsule"));

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AHBaseCharacter::Attack()
{
	ProcessAttack();
}


UAbilitySystemComponent* AHBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AHBaseCharacter::CanJumpInternal_Implementation() const
{
	// 기본 조건 (공중에 떠있는지 등) 확인
	bool bCanJump = Super::CanJumpInternal_Implementation();

	return bCanJump;
}

float AHBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	LastDamageCauser = DamageCauser;
	SetDead();
	return DamageAmount;
}

void AHBaseCharacter::ProcessAttack()
{
	if (Attackable)
	{
		Attackable = false;
		AttackBegin();
	}
}

void AHBaseCharacter::AttackBegin()
{
	//// 이동 기능 홀드
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	const float AttackSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr)
		AnimInstance->Montage_Play(AttackMontage, AttackSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AHBaseCharacter::AttackEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

}

void AHBaseCharacter::AttackEnd(UAnimMontage* InAnimMontage, bool bInInterrupted)
{
	Attackable = true;
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AHBaseCharacter::UpdateWalkSpeed(const float InNewWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = InNewWalkSpeed;
}

void AHBaseCharacter::SetDead()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(DeadMontage, 1.0f);
	}
}

void AHBaseCharacter::AttackHitCheck()
{
	const float AttackRange = 40.0f;
	const float AttackDamage = 30.0f;

	const FVector Start = GetMesh()->GetSocketLocation(WeaponSocketName);
	// const FVector End = Start; // 정적인 위치 체크라면 동일하게, 이전 프레임 위치를 저장해 선형 체크도 가능
	//const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack));
	Params.AddIgnoredActor(this); // 나 자신은 제외
	
	// 구체 형태(Sphere)로 충돌 검사
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start, End,
		FQuat::Identity,
		ECC_GameTraceChannel1, // 전용 공격 채널 설정 권장
		FCollisionShape::MakeSphere(HitRadius),
		Params
	);
	
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* TargetActor = Hit.GetActor();
			if (TargetActor && !WasAlreadyHit(TargetActor))
			{
				AddHitActor(TargetActor); // 한 번의 스윙에 중복 타격 방지
	
				FDamageEvent DamageEvent;
				TargetActor->TakeDamage(AttackDamage, DamageEvent, GetController(), this);

				//// GAS를 사용한다면 여기서 GameplayEffect 적용
				//ApplyDamageToTarget(TargetActor);
			}
		}
	}

#if ENABLE_DRAW_DEBUG

	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = bHit ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, HitRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 1.0f);

#endif
}
