# 🎮 Skill Gem System Design Document

본 문서는 스킬젬 시스템의 데이터 구조 및 로직을 정의합니다. 모든 클래스와 주요 타입에는 프로젝트 컨벤션에 따라 **H** 접두사를 사용합니다.

---

## 1. Core Enumerations & Types
시스템의 위계 구조를 정의하는 핵심 열거형입니다.

| Category | Type Name | Values | Description |
| :--- | :--- | :--- | :--- |
| **Gem Category** | `HEGemCategory` | `Main`, `Support` | 젬의 대분류 (액티브 스킬 vs 패시브 보조) |
| **HEGemType** | `HEGemType` | `Common`, `Melee`, `Projectile` | 젬의 작동 방식 및 호환성 정의 |
| **Element** | `HEElement` | `Physical`, `Fire`, `Ice`, `Lightning` | 속성 분류 (공명 시스템용) |
| **Shape** | `HEShape` | `Area` (AOE), `Projectile` | 공격의 형태적 분류 |

---

## 2. Main Skill Gems (Active)
플레이어가 직접 사용하는 스킬입니다. 모든 메인 젬은 `Melee` 혹은 `Projectile` 중 하나의 타입을 가집니다.

| Skill Name | **HEGemType** | HEElement | Description | Key Features |
| :--- | :--- | :--- | :--- | :--- |
| **주먹질** | **Melee** | Physical | 기본적인 근접 타격 | 빠른 공속, 표준 리치 |
| **거대 망치** | **Melee** | Physical | 묵직한 한 방 공격 | 느린 공속, 고데미지, 강력한 넉백 |
| **회전 톱날** | **Projectile** | Physical | 주변 회전 투사체 | 플레이어 중심 공전, 지속 데미지 |
| **파이어볼** | **Projectile** | Fire | 직선 발사 화염구 | 충돌 시 폭발 (HProjectile 기반) |
| **별사탕** | **Projectile** | Magic | 유도 미사일 | 적 추적 기능 (Homing) |
| **깜짝 상자** | **Projectile** | Physical | 설치형 함정 | 감지 범위 내 적 진입 시 폭발 |

---

## 3. Support Gems (Passive)
메인 젬의 성능을 변형하거나 강화합니다. 자신의 `HEGemType`에 따라 장착 가능한 메인 젬이 결정됩니다.

### 3.1 Common (공통 보조)
`HEGemType`에 상관없이 모든 메인 젬에 연결 가능합니다.

* **스탯 강화:** 공격 범위 증가, 공격 속도 증가, 데미지 증가, 넉백 증가
* **특수 효과:** 흡혈, 처형(저체력 즉시 처치)
* **하이리스크:** 광전사(공격력 증가/체력 감소), 광기(공속 폭발/조작 반전)
* **유틸리티:** 전염(상태이상 전이)

### 3.2 Type-Specific (타입 전용 보조)
특정 `HEGemType`을 가진 젬에만 링크할 수 있습니다.

| Target Type | Gem Name | Effect |
| :--- | :--- | :--- |
| **Projectile** | 투사체 갯수 증가 | 한 번에 발사되는 투사체 수 증가 |
| **Projectile** | 투사체 속도 증가 | 투사체의 탄속 및 유효 사거리 증가 |
| **Melee** | 연쇄 타격 | 공격 시 일정 확률로 판정 반복 발생 |
| **Melee** | 충격파 | 근접 타격 성공 시 주변 광역 데미지 |
| **Melee** | 잔혹한 상처 | 치명타 발생 시 지속 출혈 데미지 부여 |
| **Melee** | 파고들기 | 공격 시전과 동시에 짧은 거리 돌진 |

---

## 4. Systems & Logic

### 4.1 몽타주 매핑 시스템 (Animation Mapping)
유닛 프로필(`UHUnitProfileData`)은 단일 몽타주 대신 태그 기반의 맵을 사용하여 다양한 스킬 모션을 관리합니다.
*   **ActionMontageMap**: `TMap<FGameplayTag, UAnimMontage*>`를 통해 `Ability.Attack`, `Ability.FireBall` 등의 태그에 맞는 애니메이션을 출력합니다.
*   **Fallback**: 특정 태그에 해당하는 몽타주가 없을 경우 기본 `AttackMontage`를 사용합니다.

### 4.2 발사체 시스템 (Projectile System)
`Projectile` 타입의 메인 젬은 `AHProjectile` 클래스를 사용하여 투사체를 생성합니다.
*   **Faction Check**: 시전자의 `UnitType`을 투사체에 전달하여 아군(같은 타입)에게는 데미지를 입히지 않도록 판별합니다.
*   **VFX Integration**: 나이아가라 시스템을 사용하여 비행 효과 및 충돌 폭발 효과를 표현하며, 오브젝트 풀링을 통해 최적화합니다.

### 4.3 젬 공명 시스템 (Resonance)
슬롯에 장착된 젬들의 동일 속성(HEElement) 개수를 체크하여 패시브 효과를 발동합니다.

### 4.4 젬 업그레이드 시스템 (Gem Upgrade)
동일한 종류의 젬을 모아 성능이 강화된 상위 티어 젬으로 합성할 수 있습니다.
*   **합성 조건**: 인벤토리에 동일한 `GemID`를 가진 젬 인스턴스가 3개 모일 경우.
*   **자동 합성**: `UHGemInventoryComponent`에서 젬이 추가될 때마다 자동으로 체크하여 상위 젬 1개로 변환합니다.
*   **데이터 정의**: `FHGemData`의 `Tier`와 `NextTierGemID` 필드를 통해 업그레이드 경로를 정의합니다.

---

## 5. Implementation Notes for AI (AI 구현 참고사항)

1. **Data Management:**
     - `FHGemData` (USTRUCT): `SkillAbilityClass` 필드를 통해 해당 젬이 실행할 GAS 어빌리티를 지정합니다.
2. **Class Hierarchy:** 
     - `AHProjectile`: 모든 투사체의 기반 클래스.
     - `UHGA_FireBall`: 파이어볼 로직을 담당하는 Gameplay Ability.
     - `UHGemBase` (상위 클래스, UObject 상속): `FHGemData` 인스턴스를 보관하여 작동.
3. **Inventory Integration:**
     - `UHGemInventoryComponent::AddGem(const FHGemData&)`를 통해 젬을 생성 및 인벤토리에 추가.
4. **Logic Validation:** 보조젬 장착 시 `HSupportGem->TargetType`이 메인젬의 `HEGemType`과 일치하거나 'Common'인지 반드시 체크할 것.
