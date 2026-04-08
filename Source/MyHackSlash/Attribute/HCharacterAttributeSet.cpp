#include "Attribute/HCharacterAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Unit/HBaseCharacter.h"
#include <Unit/Player/HPlayerCharacter.h>
#include <Item/HBreakableActor.h>

UHCharacterAttributeSet::UHCharacterAttributeSet()
	: Level(1.0f)
	, MaxLevel(100.0f)
	, Health(100.0f)
	, MaxHealth(100.0f)
	, AttackDamage(100.0f)
	, MaxAttackDamage(1000.0f)
	, AttackRange(150.0f)
	, MaxAttackRange(2000.0f)
	, AttackRadius(50.0f)
	, MaxAttackRadius(1000.0f)
	, AttackSpeedRate(1.0f)
	, MaxAttackSpeedRate(5.0f)
	, MovementSpeed(400.0f)
	, MaxMovementSpeed(2000.0f)
	, CriticalRate(5.0f)
	, MaxCriticalRate(100.0f)
	, CriticalMultiplier(1.5f)
	, MaxCriticalMultiplier(10.0f)
	, Experience(0.0f)
	, MaxExperience(100.0f)
{
}

void UHCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetLevelAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.0f, GetMaxLevel());
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetAttackDamageAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAttackDamage());
	}
	else if (Attribute == GetAttackRangeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAttackRange());
	}
	else if (Attribute == GetAttackRadiusAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAttackRadius());
	}
	else if (Attribute == GetAttackSpeedRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAttackSpeedRate());
	}
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMovementSpeed());
	}
	else if (Attribute == GetCriticalRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxCriticalRate());
	}
	else if (Attribute == GetCriticalMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.0f, GetMaxCriticalMultiplier());
	}
	else if (Attribute == GetExperienceAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}

void UHCharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetExperienceAttribute())
	{
		float CurrentExp = NewValue;
		float MaxExp = GetMaxExperience();

		if (MaxExp > 0.0f && CurrentExp >= MaxExp)
		{
			// 레벨업 조건 충족
			float RemainingExp = CurrentExp - MaxExp;
			
			// 1. 경험치 초기화 (남은 경험치 이월) 및 레벨 증가
			// 주의: 내부적으로 다시 PostAttributeChange를 트리거할 수 있으므로 설계에 유의
			SetExperience(RemainingExp);
			SetLevel(GetLevel() + 1.0f);

			// 2. 캐릭터 클래스에 레벨업 알림
			if (AActor* TargetActor = GetOwningAbilitySystemComponent()->GetAvatarActor())
			{
				if (AHPlayerCharacter* Player = Cast<AHPlayerCharacter>(TargetActor))
				{
					Player->OnLevelUp();
				}
			}
		}
	}
}

bool UHCharacterAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0.f)
		{
			// TODO: 방어력 계산이나 무적 상태 확인 등 데미지가 최종 적용되기 전 처리를 수행합니다.
		}
	}

	return true;
}

void UHCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 로그 : 어떤 속성이든 변화가 생기면 무조건 출력
	UE_LOG(LogTemp, Warning, TEXT("Attribute Changed: %s, Magnitude: %f"),
		*Data.EvaluatedData.Attribute.GetName(), Data.EvaluatedData.Magnitude);

	AActor* TargetActor = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = FMath::Abs(GetDamage());
		SetDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			// 1. 실제 체력 차감 로직 (공통)
			const float NewHealth = FMath::Clamp(GetHealth() - LocalDamageDone, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
			if (TargetASC)
			{
				// 2. 공통 피드백 실행 (데미지 텍스트 및 타격 이펙트)
				FGameplayCueParameters CueParams;
				CueParams.RawMagnitude = LocalDamageDone;
				CueParams.EffectContext = Data.EffectSpec.GetContext();
				TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Character.Hitted")), CueParams);

				// 3. 타입별 특수 처리 (사망/파괴 태그 등)
				if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(TargetActor))
				{
					// 가해자(Instigator) 정보 기록
					AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
					if (Instigator)
					{
						Character->SetLastDamageCauser(Instigator);
					}

					// UI 업데이트 브로드캐스트
					Character->OnHPChanged.Broadcast(GetHealth(), GetMaxHealth());

					// 캐릭터 사망 판정
					if (GetHealth() <= 0.0f)
					{
						TargetASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Character.State.IsDead")));
					}
				}
				else if (AHBreakableActor* Breakable = Cast<AHBreakableActor>(TargetActor))
				{
					// 오브젝트 파괴 판정
					if (GetHealth() <= 0.0f)
					{
						TargetASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Object.State.Broken")));
					}
				}
			}
		}
	}

	// 나머지 속성들에 대한 클램핑 및 업데이트 처리
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		if (AHBaseCharacter* Character = Cast<AHBaseCharacter>(TargetActor))
		{
			Character->OnHPChanged.Broadcast(GetHealth(), GetMaxHealth());
		}
	}
	else if (Data.EvaluatedData.Attribute == GetAttackDamageAttribute())
	{
		SetAttackDamage(FMath::Clamp(GetAttackDamage(), 0.0f, GetMaxAttackDamage()));
	}
	else if (Data.EvaluatedData.Attribute == GetAttackRangeAttribute())
	{
		SetAttackRange(FMath::Clamp(GetAttackRange(), 0.0f, GetMaxAttackRange()));
	}
	else if (Data.EvaluatedData.Attribute == GetAttackRadiusAttribute())
	{
		SetAttackRadius(FMath::Clamp(GetAttackRadius(), 0.0f, GetMaxAttackRadius()));
	}
	else if (Data.EvaluatedData.Attribute == GetAttackSpeedRateAttribute())
	{
		SetAttackSpeedRate(FMath::Clamp(GetAttackSpeedRate(), 0.0f, GetMaxAttackSpeedRate()));
	}
	else if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		SetMovementSpeed(FMath::Clamp(GetMovementSpeed(), 0.0f, GetMaxMovementSpeed()));
	}
	else if (Data.EvaluatedData.Attribute == GetCriticalRateAttribute())
	{
		SetCriticalRate(FMath::Clamp(GetCriticalRate(), 0.0f, GetMaxCriticalRate()));
	}
	else if (Data.EvaluatedData.Attribute == GetCriticalMultiplierAttribute())
	{
		SetCriticalMultiplier(FMath::Clamp(GetCriticalMultiplier(), 1.0f, GetMaxCriticalMultiplier()));
	}
}
