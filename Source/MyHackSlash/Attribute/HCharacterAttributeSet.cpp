#include "Attribute/HCharacterAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Unit/HBaseCharacter.h"

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
	, Gold(0.0f)
	, MaxGold(999999.0f)
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
	else if (Attribute == GetGoldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxGold());
	}
	else if (Attribute == GetExpRewardAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetGoldRewardAttribute())
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
			// 레벨업 조건 충족 (경험치 이월 및 레벨 증가)
			float RemainingExp = CurrentExp - MaxExp;
			
			// 캐릭터 클래스를 직접 호출하는 대신 어트리뷰트 값만 변경합니다.
			// 캐릭터는 이 변화를 스스로 감지하여 필요한 로직(OnLevelUp 등)을 수행합니다.
			SetExperience(RemainingExp);
			SetLevel(GetLevel() + 1.0f);
		}
	}
}

bool UHCharacterAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	return true;
}

void UHCharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = FMath::Abs(GetDamage());
		SetDamage(0.0f);

		if (LocalDamageDone > 0.0f)
		{
			// 가해자 정보 추출
			FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
			AActor* Instigator = Context.GetInstigator();
			AActor* EffectCauser = Context.GetEffectCauser();

			// 1. 실제 체력 차감 로직
			const float NewHealth = FMath::Clamp(GetHealth() - LocalDamageDone, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
			if (TargetASC)
			{
				// 타겟 캐릭터에게 가해자 정보 전달
				AHBaseCharacter* TargetCharacter = Cast<AHBaseCharacter>(TargetASC->GetAvatarActor());
				if (TargetCharacter)
				{
					TargetCharacter->SetLastDamageCauser(Instigator ? Instigator : EffectCauser);

					// 데미지 로그 추가
					FString InstigatorName = Instigator ? Instigator->GetName() : (EffectCauser ? EffectCauser->GetName() : TEXT("Unknown"));
					UE_LOG(LogTemp, Warning, TEXT("[DAMAGE] %s -> %s : %f Damage (Remaining HP: %f/%f)"), 
						*InstigatorName, *TargetCharacter->GetName(), LocalDamageDone, GetHealth(), GetMaxHealth());
				}

				// 2. 피드백 실행 (데미지 텍스트 등 시각적 효과는 GameplayCue를 통해 처리)
				FGameplayCueParameters CueParams;
				CueParams.RawMagnitude = LocalDamageDone;
				CueParams.EffectContext = Context;
				TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Character.Hitted")), CueParams);

				// 3. 사망 판정 (태그만 추가하여 캐릭터가 스스로 인지하게 함)
				if (GetHealth() <= 0.0f)
				{
					// 사망 상태 태그 추가 (HBaseCharacter 등이 이를 감지하여 SetDead 등을 호출함)
					TargetASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Character.State.IsDead")));
					
					// Breakable Actor 등의 경우를 위해 공통적인 파괴/사망 태그 처리
					TargetASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Object.State.Broken")));
				}
			}
		}
	}

	// 나머지 모든 속성들에 대한 클램핑 처리 (부동소수점 오차 방지 등)
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}
