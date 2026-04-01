# ⚔️ GAS & Combat System Design Document

본 문서는 프로젝트의 **Gameplay Ability System (GAS)** 연동 방식과 전투 로직의 설계 구조를 정의합니다.

---

## 1. GAS Core Components

### 1.1 Ability System Component (ASC)
*   **Owner**: `AHBaseCharacter`가 `AbilitySystemComponent`를 소유합니다.
*   **Initialization**: 하위 클래스(Player, Monster)에서 필요에 따라 초기 어빌리티(`StartAbilities`)를 부여받습니다.
*   **Input Binding**: `UHEquipmentComponent`를 통해 장착된 젬의 슬롯 인덱스에 따라 `InputID`가 동적으로 매핑됩니다.

### 1.2 Gameplay Abilities (GA)
모든 액티브 스킬과 공격 동작은 `UGameplayAbility`를 상속받아 구현됩니다.

| Class | Base Tag | Description |
| :--- | :--- | :--- |
| **UHGA_Attack** | `Ability.Attack` | 근접 공격의 기반 클래스. 몽타주 재생 및 이펙트 소환 담당. |
| **UHGA_FireBall** | `Ability.FireBall` | 투사체 발사형 스킬. `AHProjectile`을 생성하여 발사. |

### 1.3 Gameplay Tags
시스템의 상태와 액션을 식별하는 데 사용됩니다.

*   **Ability Tags**: `Ability.FireBall`, `Ability.FistAttack` (어빌리티 식별 및 애니메이션 매핑용)
*   **State Tags**: `Character.State.IsAttacking`, `Character.State.IsJumping` (상태 제어용)

---

## 2. Stat & Damage System

이 프로젝트는 표준 `UAttributeSet` 대신 **DataTable 기반의 커스텀 스탯 시스템**을 사용합니다.

### 2.1 Unit Stats (`FUnitStatRow`)
모든 유닛은 `FUnitStatRow` 구조체를 통해 다음 핵심 스탯을 관리합니다.
*   **MaxHP**: 최대 체력
*   **AttackDamage**: 기본 공격력
*   **AttackRange**: 공격 유효 거리
*   **AttackSpeedRate**: 애니메이션 재생 속도 배율
*   **MovementSpeed**: 이동 속도

### 2.2 Damage Workflow
1.  **Hit Detection**: `IHCombatInterface::AttackHitCheck()`에서 `SweepMulti`를 통해 타겟을 감지합니다.
2.  **Faction Check**: 공격자와 피격자의 `EHUnitType`(Player/Monster)을 비교하여 아군 오사를 방지합니다.
3.  **Execution**: `AActor::TakeDamage`를 통해 데미지를 전달합니다.
4.  **Reaction**: `AHBaseCharacter::TakeDamage`에서 `CurrentHP`를 차감하고 다음 피드백을 실행합니다.
    *   **VFX**: 피격 이펙트 및 나이아가라 효과 재생.
    *   **Camera Shake**: 플레이어 캐릭터인 경우 `HitCameraShakeClass`를 통한 화면 흔들림 실행.
    *   **Death**: 사망 시 래그돌 및 임펄스 적용.

### 2.3 Skill Damage Calculation
*   **UHGA_FireBall**: 스킬 데미지는 본체의 기본 스탯과 어빌리티 고유 데미지를 합산하여 계산합니다.
    *   `FinalDamage = UHGA_FireBall::ProjectileDamage + FUnitStatRow::AttackDamage`
    *   이를 통해 캐릭터의 레벨업이나 스탯 변화가 스킬 데미지에 즉각 반영되도록 설계되었습니다.

---

## 3. Animation & VFX Integration

### 3.1 Tag-based Montage Mapping
`UHUnitProfileData` 에셋을 통해 어빌리티 태그별 몽타주를 관리합니다.
*   **ActionMontageMap**: `TMap<FGameplayTag, UAnimMontage*>`를 사용하여 어빌리티 실행 시 적절한 애니메이션을 동적으로 선택합니다.
*   **Fallback**: 특정 태그에 대한 설정이 없을 경우 기본 `AttackMontage`를 사용합니다.

### 3.2 Object Pooling (VFX)
성능 최적화를 위해 `UHObjectPoolManager`를 통해 나이아가라 시스템을 관리합니다.
*   공격 시 소환되는 이펙트나 피격 시 발생하는 효과는 모두 풀링된 인스턴스를 재사용합니다.

---

## 4. Combat Interface (`IHCombatInterface`)

전투 관련 공통 기능을 인터페이스로 추상화하여 관리합니다.
*   **AttackHitCheck**: 물리적 타격 판정 로직.
*   **AlreadyHitActors**: 다중 타격 시 동일 대상 중복 피격 방지를 위한 캐싱 리스트.
*   **WeaponSocket**: 무기 또는 타격 지점의 소켓 이름 (`hand_r` 등).

---

## 5. Implementation Notes for AI

1.  **Ability Activation**: 몬스터 AI는 비헤이비어 트리에서 태그를 기반으로 어빌리티를 트리거해야 합니다.
2.  **Death Logic**: 사망 시 `EnableRagdoll`과 `SetDeadImpulse`를 통해 물리 기반의 사망 연출을 수행합니다. 또한, 설정된 확률 및 양에 따라 **코인(`AHCoin`)**을 드랍합니다.
3.  **Homing Projectiles**: `AHProjectile` 클래스를 확장하여 유도 기능을 추가할 수 있습니다.
