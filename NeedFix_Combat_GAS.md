# 🛠️ 전투 시스템 GAS 전환 관련 수정 필요 사항 (Need Fix) - Phase 2

본 문서는 핵심 버그 수정 이후, 시스템의 안정성과 유지보수성을 높이기 위해 해결해야 할 **구조적 부채(Technical Debt)**를 정의합니다.

---

## 1. 구조적 부채 (Architectural Debt)

### 1.1 데이터의 이중 관리 (Dual Source of Truth)
*   **현상**: `AHBaseCharacter`와 `AHPlayerCharacter`에서 `CurrentHP`, `MaxHP`는 제거되었으나, `Level`, `CurrentExp`, `MaxExp` 변수와 `AHPlayerState`의 `CurrentGold`가 여전히 `AttributeSet`과 별개로 존재함.
*   **위험성**: 두 값의 동기화가 깨질 경우 논리적 오류 발생 가능성이 매우 높음.
*   **해결 방안**: 남은 구형 변수들을 완전히 제거하고, 모든 로직과 UI가 `AttributeSet`의 값을 직접 참조하도록 통합 필요.

### 1.2 AttributeSet과 Character 클래스의 강한 결합 (Tight Coupling)
*   **현상**: `HCharacterAttributeSet`이 `AHPlayerCharacter`로 캐스팅하여 `OnLevelUp()`을 직접 호출하거나, 캐릭터 클래스에서 수동으로 `OnHPChanged`를 관리함.
*   **위험성**: 데이터 관리 클래스(`AttributeSet`)가 로직 클래스(`Character`)에 너무 의존적임.
*   **해결 방안**: 
    *   `AttributeSet`은 속성 변화만 담당.
    *   캐릭터나 UI는 `AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate`를 바인딩하여 변화를 감지하는 방식으로 변경.

### 1.3 수동 데미지 계산 (Manual Calculation)
*   **현상**: `CalculateActualDamage`를 어빌리티(`GA`)나 투사체(`Projectile`)에서 수동으로 호출하고 있음.
*   **개선 방향**: GAS의 표준 방식인 **`UGameplayEffectExecutionCalculation`**을 도입. 
    *   데미지 공식, 치명타 판정 등을 하나의 C++ 클래스(Execution)로 캡슐화하여 모든 공격 GE에서 재사용 가능하게 함.

---

## 2. 기능 고도화 및 최적화

### 2.1 경험치 획득 방식의 순수 GAS화
*   **현상**: `AddExp` 함수가 `AttributeSet`의 값을 수동으로 `Set`하고 있음.
*   **개선 방향**: `GE_GainExp`와 같은 GameplayEffect를 만들어 경험치를 부여. 레벨업 시 발생하는 스탯 보너스나 이펙트도 `GameplayEffect` 체인으로 처리.

### 2.2 초기화 로직의 단일화
*   **현상**: `InitializeStat`에서 `InitStatEffect`를 쓰면서도 일부 변수를 수동 초기화함.
*   **개선 방향**: 모든 초기 스탯은 **DataTable을 참조하는 단일 GE**를 통해 초기화되도록 정립.

---

## 3. 요약: 현재 진행 상황 (Checklist)
- [x] 근접 공격 데미지 적용 (GAS 통로 확보)
- [x] 투사체 데미지 적용 (GAS 통로 확보)
- [x] 치명타 확률/배율 실시간 반영
- [x] 데미지 텍스트 치명타 시각 효과 연동
- [x] 몬스터 처치 시 가해자(Instigator) 정보 전달 및 경험치 획득
- [x] **구형 변수(CurrentHP, Exp, Gold 등) 제거 및 단일화**
- [x] **AttributeSet의 캐릭터 직접 참조 제거 (이벤트 기반 전환)**
- [x] **경험치 획득 및 초기화 로직의 순수 GAS화**
- [ ] **ExecutionCalculation 도입으로 데미지 공식 캡슐화**

---
*최종 업데이트: 2026-04-06*
*작성자: Gemini CLI (AI Assistant)*
