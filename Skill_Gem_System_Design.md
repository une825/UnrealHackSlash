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
| **파이어볼** | **Projectile** | Fire | 직선 발사 화염구 | 충돌 시 폭발 |
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

### 4.1 젬 공명 시스템 (Resonance)
슬롯에 장착된 젬들의 동일 속성(HEElement) 개수를 체크하여 패시브 효과를 발동합니다.

* **화염 공명 (3개):** `화염 오라` - 플레이어 주변에 주기적 화염 데미지 방출.
* **번개 공명 (3개):** `천벌` - 화면 내 랜덤 위치에 주기적 낙뢰 발생.

### 4.2 젬 합성 시스템 (Synthesis)
불필요한 자원을 소모하여 상위 혹은 새로운 능력을 획득합니다.

* **Input:** 안 쓰는 젬 3개 (등급/종류 무관)
* **Output:** 새로운 무작위 젬 1개 생성 (확률에 따라 상위 등급 출현 가능)

### 4.3 장착 매칭 로직 (Gem Matching Logic)
보조 젬을 메인 젬에 장착할 때 다음의 논리를 따릅니다.
```cpp
bool CanAttach(HMainGem, HSupportGem) {
    if (HSupportGem.Type == HEGemType::Common) return true;
    return (HMainGem.Type == HSupportGem.Type);
}
```
---

## 5. Implementation Notes for AI (AI 구현 참고사항)

1. **Data Management:**
     - `FHGemData` (USTRUCT): 개별 젬의 스펙을 정의하는 구조체. `GemID (FName)`를 식별자로 사용.
     - `UHGemDataAsset` (UDataAsset): 모든 `FHGemData`를 리스트(`GemList`)로 관리하는 컬렉션 에셋.
2. **Class Hierarchy:** 
     - `UHGemBase` (상위 클래스, UObject 상속): `FHGemData` 인스턴스를 보관하여 작동.
     - `UHMainGem` (Melee, Projectile 로직 포함)
     - `UHSupportGem` (공통/전용 효과 포함)
3. **Inventory Integration:**
     - `UHGemInventoryComponent::AddGem(const FHGemData&)`를 통해 젬을 생성 및 인벤토리에 추가.
4. **Logic Validation:** 보조젬 장착 시 `HSupportGem->TargetType`이 메인젬의 `HEGemType`과 일치하거나 'Common'인지 반드시 체크할 것.
