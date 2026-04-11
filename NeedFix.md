# 🛠️ 일반 시스템 개선 및 불필요 코드 정리 (Need Fix)

본 문서는 GAS 외의 일반적인 시스템(UI, Manager, Inventory 등)에서 발견된 개선 필요 사항과 구조적 결합도를 낮추기 위한 작업 목록을 정의합니다.

---

## 1. 매니저 클래스 개선 (Manager & Subsystem)

### 1.1 HSelectAbilityManager의 보상 지급 로직 순수화
*   **현상**: `ExecuteReward`에서 `PlayerState->AddGold()`를 직접 호출하거나 캐릭터 컴포넌트를 직접 찾아 수정함.
*   **문제점**: 매니저가 특정 클래스에 과도하게 의존하며, 상태 변경이 트래킹되지 않음.
*   **해결 방안**: 
    *   골드 지급: `GE_AddGold` (GameplayEffect)를 생성하고 플레이어의 ASC를 통해 적용.
    *   젬 지급: 젬 데이터를 담은 `GameplayEvent`를 전송하여 캐릭터나 인벤토리 컴포넌트가 스스로 처리하게 함.

### 1.2 HSoundManager의 하드코딩 사운드 키 관리
*   **현상**: `PlaySFXByKey(TEXT("LevelUpSound"), ...)`와 같이 문자열 키를 직접 사용함.
*   **개선**: 사운드 키를 `FGameplayTag`로 관리하거나, 전용 Enum/DataTable을 통해 오타 방지 및 관리 편의성 증대.

---

## 2. UI 및 데이터 흐름 개선

### 2.1 UI 데이터 단일화 준수
*   **현상**: 일부 UI 클래스에서 모델(Data)과 뷰(Widget)가 엄격히 분리되지 않고 직접 접근하는 경우가 있음.
*   **개선**: `Project_Standard_Convention.md`의 규칙에 따라 `EntryUI`와 `EntryData`를 일치시키고, 모든 UI 갱신은 델리게이트 기반으로 전환.

### 2.2 MonsterSpawnManager의 스폰 로직 최적화
*   **현상**: 몬스터 스폰 시 매번 `SpawnActor`를 호출할 가능성 (이미 오브젝트 풀을 사용 중이나, 데이터 에셋과의 연동 확인 필요).
*   **개선**: 풀링 시스템과 데이터 에셋(`HA_MonsterSpawnDataAsset`) 간의 연동을 더 견고히 하여 런타임 비용 최소화.

---

## 3. 요약: 현재 진행 상황 (Checklist)
- [x] **HSelectAbilityManager의 보상 지급 방식을 GAS 기반으로 전환**
- [x] **사운드 및 이펙트 호출 키를 GameplayTag로 전환**
- [x] **UI 위젯의 직접적인 데이터 수정 로직 제거 (델리게이트 통일)**
- [x] **글로벌 상수 및 타입 정의(`HGlobalTypes.h`)의 체계적 정리**

---
## 🏁 일반 시스템 개선 완료 (General Systems Report)
본 프로젝트의 보조 시스템들(사운드, 보상 매니저 등)이 더 견고하고 유연한 구조로 개선되었습니다.

1.  **사운드 시스템**: `FGameplayTag`를 기반으로 사운드 에셋을 관리하도록 변경하여 타입 안정성을 확보했습니다.
2.  **보상 시스템**: 매니저와 캐릭터 간의 직접적인 함수 호출을 제거하고, `GameplayEvent`와 `GameplayEffect`를 통한 비결합(Decoupling) 구조를 완성했습니다.
3.  **UI 및 최적화**: 기존의 컨벤션을 재확인하고, 오브젝트 풀링이 적재적소에 사용되고 있음을 검증했습니다.

---
*최종 업데이트: 2026-04-10*
*작성자: Gemini CLI (AI Assistant)*
