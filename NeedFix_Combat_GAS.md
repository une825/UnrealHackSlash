# 🛠️ 전투 시스템 GAS 전환 관련 수정 필요 사항 (Need Fix) - Phase 3

본 문서는 GAS 전환의 최종 단계로, 시스템의 순수성을 높이고 불필요한 레거시 코드를 제거하여 유지보수성을 극대화하기 위한 작업 목록을 정의합니다.

---

## 1. 핵심 구조 개선 (Architectural Refactoring)

### 1.1 데미지 계산의 캡슐화 (Execution Calculation 도입)
*   **현상**: `AHBaseCharacter::CalculateActualDamage`에서 치명타 확률/배율을 수동으로 계산하고, 이를 `GA`나 `Projectile`에서 직접 호출함.
*   **문제점**: 데미지 공식이 분산되어 있고, GAS의 표준 방식(Execution)을 활용하지 못함.
*   **해결 방안**: **`UGameplayEffectExecutionCalculation`**을 상속받은 `HDamageExecutionCalculation` 클래스를 생성.
    *   공격자의 `AttackDamage`, `CriticalRate`, `CriticalMultiplier` 캡처.
    *   방어자의 `Defense` (필요 시 추가) 캡처.
    *   최종 데미지 산출 로직을 이 클래스 하나로 통합.

> **💡 치명타 구현 방식 메모 (Implementation Note):**
> *   **확률 판정 (Chance Roll):** 어빌리티(`GA`)나 투사체(`Projectile`) 등 데미지 발생 시점에 직접 수행합니다. 성공 시 `GameplayEffectSpec`에 `Effect.Critical` 태그를 동적으로 추가합니다.
> *   **배율 적용 (Multiplier):** `ExecutionCalculation`에서 `Effect.Critical` 태그 존재 여부를 확인하여 공격자의 `CriticalMultiplier` 속성을 최종 데미지에 곱해줍니다.
> *   **시각 효과 (Visuals):** 동일한 태그(`Effect.Critical`)를 `GameplayCue`가 감지하여 치명타 전용 데미지 텍스트나 이펙트를 출력하도록 일원화했습니다.

### 1.2 레벨업 처리의 완전 순수 GAS화
*   **현상**: `AHPlayerCharacter::OnLevelUp`에서 `SetHealth`를 직접 호출하여 체력을 회복하고 `InitializeStat`을 수동으로 재호출함.
*   **문제점**: 상태 변화가 코드 흐름에 의존적이며, 데이터 기반(Data-driven) 설계에서 벗어남.
*   **해결 방안**:
    *   `GE_LevelUp_Heal`: 레벨업 태그 감지 시 실행될 체력 회복 GE 생성.
    *   `AttributeSet::PostAttributeChange`에서 레벨 변경 시 자동으로 필요한 GE를 적용하도록 개선.

---

## 2. 불필요한 코드 및 중복 제거 (Clean Up)

### 2.1 레거시 데미지 함수 제거
*   **대상**: `AHBaseCharacter::CalculateActualDamage`, `AHBaseCharacter::TakeDamage` (오버라이드 구현부)
*   **이유**: 
    *   `CalculateActualDamage`는 `ExecutionCalculation`으로 대체됨.
    *   `TakeDamage`는 GAS의 `Damage` 메타 어트리뷰트와 `PostGameplayEffectExecute` 흐름으로 완전히 대체 가능하므로, 특수한 엔진 연동이 필요한 경우를 제외하고 단순 제거 권장.

### 2.2 초기화 로직의 이중성 정리
*   **현상**: `InitializeStat` 함수와 `InitStatEffect`가 혼재되어 사용됨.
*   **개선**: 모든 스탯 초기화(Level 포함)는 **`InitStatEffect` (GE)** 하나로 단일화. 캐릭터 클래스에서는 단순히 이 GE를 적용하는 함수만 노출.

### 2.3 이벤트 전달 방식의 통일
*   **현상**: `AHBaseMonster::OnMonsterDead` (델리게이트)와 `Event.Character.MonsterKilled` (GAS 태그 이벤트)가 공존함.
*   **개선**: 시스템 간의 결합도를 낮추기 위해 점진적으로 **GAS GameplayEvent** 방식으로 통합하고 레거시 델리게이트는 제거.

---

## 3. 요약: 현재 진행 상황 (Checklist)
- [x] 구형 변수(CurrentHP, Exp, Gold 등) 제거 및 단일화
- [x] AttributeSet의 캐릭터 직접 참조 제거 (이벤트 기반 전환)
- [x] 경험치 획득 로직의 순수 GAS화 (SetByCaller 활용)
- [x] **ExecutionCalculation 도입으로 데미지 공식 캡슐화 (치명타 포함)**
- [x] **캐릭터 클래스 내 레거시 계산 함수(`CalculateActualDamage`) 완전 제거**
- [x] **초기화 로직 단일화 (InitStatEffect 중심)**

---
## 🏁 GAS 전환 완료 (Final Report)
본 프로젝트의 핵심 전투 시스템은 기존의 명령형 C++ 구조에서 **순수 데이터 중심의 Gameplay Ability System(GAS)**으로 성공적으로 전환되었습니다.

1.  **데이터 단일화**: 캐릭터 클래스의 구형 변수들을 제거하고 `AttributeSet`으로 스탯을 통합했습니다.
2.  **로직 캡슐화**: 복잡한 데미지 공식을 `ExecutionCalculation`으로 옮겨 유지보수성을 높였습니다.
3.  **이벤트 주도 설계**: 레벨업, 경험치 획득, 사망 처리 등을 `GameplayEvent`와 `GameplayEffect` 체인으로 재구성했습니다.
4.  **확장성 확보**: 새로운 스킬이나 스탯 추가 시 C++ 코드 수정 없이 데이터 기반으로 대응이 가능해졌습니다.

---
*최종 업데이트: 2026-04-10*
*작성자: Gemini CLI (AI Assistant)*
