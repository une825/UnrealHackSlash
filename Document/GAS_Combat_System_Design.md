# ⚔️ GAS & Combat System Design Document

본 문서는 프로젝트의 **Gameplay Ability System (GAS)** 연동 방식과 전투 로직의 설계 구조를 정의합니다.

---

## 1. GAS Core Components

### 1.1 Ability System Component (ASC)
*   **Owner**: `AHBaseCharacter`가 `AbilitySystemComponent`를 소유합니다.
*   **Initialization**: 하위 클래스(Player, Monster)에서 필요에 따라 초기 어빌리티(`StartAbilities`)를 부여받습니다.
*   **Input Binding**: `UHEquipmentComponent`를 통해 장착된 젬의 슬롯 인덱스에 따라 `InputID`가 동적으로 매핑됩니다.
*   **Multiplayer Replication**: 플레이어 ASC는 `AHPlayerState`에서 복제되며 `Mixed` replication mode를 사용합니다. 몬스터 ASC는 `AHBaseMonster`에서 복제되며 `Minimal` replication mode를 사용합니다.

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

이 프로젝트는 **GAS 표준 `UAttributeSet`**을 사용하여 유닛의 핵심 스탯을 실시간으로 관리하며, 초기값은 **DataTable**을 통해 설정할 수 있습니다.

### 2.1 Character Attribute Set (`UHCharacterAttributeSet`)
유닛의 핵심 스탯을 정의하고 GAS와 연동합니다. 각 스탯은 대응하는 **Max** 속성을 통해 상한선이 제한됩니다.
*   **Health / MaxHealth**: 현재 체력 및 최대 체력
*   **Hunger / MaxHunger**: 현재 배고픔 및 최대 배고픔 (플레이어 전용)
*   **AttackDamage / MaxAttackDamage**: 기본 공격력 및 최대 공격력
*   **AttackRange / MaxAttackRange**: 공격 유효 거리 및 최대 공격 거리
*   **AttackRadius / MaxAttackRadius**: 공격 반경 및 최대 공격 반경
*   **AttackSpeedRate / MaxAttackSpeedRate**: 애니메이션 재생 속도 배율 및 최대 배율
*   **MovementSpeed / MaxMovementSpeed**: 이동 속도 및 최대 이동 속도

멀티플레이에서 UI와 상태 동기화를 위해 주요 Attribute는 `ReplicatedUsing`과 `GAMEPLAYATTRIBUTE_REPNOTIFY`를 사용해 복제됩니다. `Health`, `Hunger`, `Experience`, `Level`, `Gold`와 전투 계산에 필요한 공격 관련 Attribute는 서버 값을 기준으로 클라이언트에 전파됩니다.

### 2.2 Hunger System (배고픔 시스템)
플레이어의 생존을 위협하는 핵심 기믹으로, 시간에 따라 감소하며 전투를 통해 유지해야 합니다.
*   **지속 감소**: `GE_Hunger_Decay`를 통해 일정 시간(Period)마다 `Hunger` 어트리뷰트가 감소합니다.
*   **전투 시 회복**: 몬스터 처치 시 즉각적인(Instant) GE가 적용되어 `Hunger`를 일정량 회복합니다.
*   **허기 상태 효과**: `Hunger`가 0이 되면 `Character.State.IsStarving` 태그가 부여되며, 지속적으로 `Health`가 감소하는 디버프가 발생합니다.

### 2.2.1 Multiplayer Experience Policy

멀티플레이에서는 플레이어별 경험치를 따로 올리지 않고 파티 공유 경험치를 사용합니다.

*   **공유 경험치**: 몬스터 처치 등으로 획득한 경험치는 서버가 공용 진행도로 누적합니다.
*   **필요 경험치 배율**: 멀티플레이 레벨업에 필요한 경험치는 솔로 플레이 기준의 2배를 요구합니다.
*   **동시 레벨업 처리**: 공유 경험치가 레벨업 기준에 도달하면 모든 플레이어에게 능력 선택 UI를 동시에 표시합니다.
*   **선택 중 일시정지**: 멀티플레이에서도 능력 선택 단계는 예외적으로 서버가 게임을 일시정지합니다. 선택 제한 시간은 10초이며, 시간 초과 시 서버가 미선택 플레이어의 보상을 자동 선택합니다.
*   **재개 조건**: 모든 플레이어가 선택을 완료하거나 10초 타임아웃 처리가 끝나면 서버가 일시정지를 해제합니다.

### 2.3 Damage Workflow
1.  **Hit Detection**: `IHCombatInterface::AttackHitCheck()`에서 `SweepMulti`를 통해 타겟을 감지합니다.
2.  **Faction Check**: 공격자와 피격자의 `EHUnitType`(Player/Monster)을 비교하여 아군 오사를 방지합니다.
3.  **Execution**: `AActor::TakeDamage`를 통해 데미지를 전달합니다.
4.  **Reaction**: `AHBaseCharacter::TakeDamage`에서 `CurrentHP`를 차감하고 다음 피드백을 실행합니다.
    *   **VFX**: 피격 이펙트 및 나이아가라 효과 재생.
    *   **Camera Shake**: 플레이어 캐릭터인 경우 `HitCameraShakeClass`를 통한 화면 흔들림 실행.
    *   **Death**: 사망 시 래그돌 및 임펄스 적용.

멀티플레이에서는 실제 히트 판정 결과 처리와 `GameplayEffect` 적용은 서버에서만 실행합니다. 클라이언트는 입력과 예측/연출을 담당하고, 체력 변화는 Attribute 복제로 동기화합니다. 사망 판정은 서버의 DeadTag 변경에서 `AHBaseCharacter::SetDead()`로 확정하고, `IsDead` 복제와 `OnRep_IsDead()`를 통해 클라이언트가 래그돌/임펄스 연출을 적용합니다. 풀에서 재사용되는 몬스터는 `IsDead=false` 복제 시 클라이언트도 캡슐, 무브먼트, 메쉬 물리/가시성 상태를 생존 상태로 복구합니다.

### 2.4 Skill Damage Calculation
*   **UHGA_FireBall**: 스킬 데미지는 본체의 기본 스탯과 어빌리티 고유 데미지를 합산하여 계산합니다.
    *   `FinalDamage = UHGA_FireBall::ProjectileDamage + UHCharacterAttributeSet::AttackDamage`
    *   이를 통해 캐릭터의 레벨업이나 스탯 변화가 스킬 데미지에 즉각 반영되도록 설계되었습니다.
*   **Projectile Authority**: 투사체 생성과 폭발 데미지 처리는 서버에서만 실행합니다. `AHProjectile`은 actor/movement replication을 사용해 클라이언트에 위치를 전파합니다.

---

## 3. Animation & VFX Integration

### 3.1 Tag-based Montage Mapping
`UHUnitProfileData` 에셋을 통해 어빌리티 태그별 몽타주를 관리합니다.
*   **ActionMontageMap**: `TMap<FGameplayTag, UAnimMontage*>`를 사용하여 어빌리티 실행 시 적절한 애니메이션을 동적으로 선택합니다.
*   **Fallback**: 특정 태그에 대한 설정이 없을 경우 기본 `AttackMontage`를 사용합니다.
*   **Primary Slot Only**: 플레이어 장착 스킬은 `FGameplayAbilitySpec::InputID == 1`일 때만 몽타주를 재생합니다. 슬롯 1~3의 자동 발동 스킬은 몽타주 없이 효과만 처리합니다.
*   **AI Attack Flow**: 몬스터 공격 중 AI 경로 이동은 `BTT_Attack`에서 중단하고, 공격 상태는 `AHBaseCharacter::BeginAttackState(false)`/`EndAttackState()`로 관리합니다. 공격 몽타주 재생과 종료 판정은 `UHGA_MonsterMeleeAttack`의 `UAbilityTask_PlayMontageAndWait`가 단독으로 담당합니다.
*   **AI Rotation Policy**: 이동 중 회전은 `CharacterMovement`의 `bOrientRotationToMovement`에 맡깁니다. `BTT_TurnToTarget`은 공격 직전에 AI 이동을 멈추고 Actor Yaw와 컨트롤러 회전을 타겟 방향으로 보간하며, 허용 각도 안에 들어온 뒤 공격 단계로 넘어갑니다. `BTT_Attack`은 공격 시작 직전에 최종 방향을 한 번 더 보정해 전방 기반 Sweep 판정과 몽타주 방향이 어긋나지 않게 합니다.
*   **AI Attack Range Policy**: 공격 가능 판정은 실제 melee Sweep과 유사하게 2D 거리, 공격 거리, 공격 반경, 공격자/타겟 캡슐 반경을 함께 고려합니다.
*   **AI Path Rotation Smoothing**: 몬스터는 MoveTo 경로 추종 중 속도 방향이 즉시 꺾이는 것을 줄이기 위해 `bRequestedMoveUseAcceleration`을 사용하고, 플레이어보다 낮은 `RotationRate`를 적용합니다.
*   **Multiplayer Target Stability**: `BTS_Detect`는 현재 타겟이 탐지 반경 안에 있으면 유지하고, 타겟이 없거나 범위를 벗어났을 때만 가장 가까운 플레이어를 새 타겟으로 선택합니다. Overlap 결과 순서에 따라 타겟이 흔들리는 것을 방지합니다.

### 3.2 Object Pooling (VFX)
성능 최적화를 위해 `UHObjectPoolManager`를 통해 나이아가라 시스템을 관리합니다.
*   공격 시 소환되는 이펙트나 피격 시 발생하는 효과는 모두 풀링된 인스턴스를 재사용합니다.
*   NiagaraActor 풀링 시 액터를 다시 보이게 만들면 Niagara가 아닌 보조 표시 컴포넌트가 함께 드러날 수 있으므로, 풀에서 꺼낼 때 NiagaraComponent만 표시하고 나머지 PrimitiveComponent는 숨깁니다. 풀 반납 시에는 이전 부착 대상에서 분리합니다.

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
    * 풀에서 재사용되는 몬스터는 사망 시 AIController를 `UnPossess()`하지 않고 이동, 포커스, BT를 즉시 정지합니다. 재활성화 시 이동 속도와 생존 컴포넌트 상태를 복구하고, 기존 컨트롤러가 같은 Pawn을 유지한 상태에서 Blackboard의 `HomePos`, `Target`, `PatrolPos`를 초기화한 뒤 BT를 다시 실행합니다. RVO avoidance, 동적 navigation 영향, 메쉬 Pawn 충돌은 생존 복구에서 임의로 켜지 않고 기존 충돌/이동 정책을 따릅니다.
    * 몬스터는 서버 권한 AI 이동을 클라이언트에 부드럽게 표시하기 위해 플레이어보다 높은 `NetUpdateFrequency`를 사용합니다.
3.  **Homing Projectiles**: `AHProjectile` 클래스를 확장하여 유도 기능을 추가할 수 있습니다.
