# 🎯 Quest & Progression System Design

본 문서는 **MyHackSlash** 프로젝트의 퀘스트(Quest) 및 진행(Progression) 시스템 설계를 정의합니다. 플레이어의 레벨업, 경험치 획득, 그리고 몬스터 처치 시 발생하는 이벤트 처리 구조를 설명합니다.

---

## 1. 개요 (Overview)

본 시스템은 플레이어의 전투 성과를 추적하고, 이를 기반으로 경험치 보상 및 퀘스트 진행 상태를 업데이트합니다. **이벤트 기반(Event-Driven)** 설계를 통해 객체 간 결합도를 낮추고 확장이 용이하도록 구성되었습니다.

---

## 2. 주요 구성 요소 (Core Components)

### 2.1 UHQuestManager (Subsystem)
*   **역할**: 몬스터 사망 이벤트를 수집하여 보상 지급(경험치) 및 퀘스트 목표 갱신을 총괄.
*   **관리**: `GameInstanceSubsystem`으로 생명주기가 관리되며, 전역적으로 접근 가능.

### 2.2 AHBaseMonster
*   **역할**: 처치 시 보상 정보(`ExpReward`)를 보관하고 사망 이벤트를 발행.
*   **이벤트**: `OnMonsterDead` 델리게이트 (공격자 정보와 자기 자신을 전달).

### 2.3 AHPlayerCharacter
*   **역할**: 경험치(`CurrentEXP`)를 누적하고 임계치 도달 시 레벨업 수행.
*   **로직**: `AddExp()` 함수를 통해 경험치를 획득하며, 레벨업 시 스탯 갱신 및 보상 팝업(능력 선택)을 트리거함.

---

## 3. 진행 로직 (Progression Logic)

### 3.1 몬스터 사망 및 보상 워크플로우
1.  **Monster Death**: 몬스터의 `CurrentHP`가 0 이하가 되면 `SetDead()` 호출.
2.  **Event Broadcast**: `OnMonsterDead.Broadcast(LastDamageCauser, this)` 실행.
3.  **Manager Handling**: `UHQuestManager::ProcessMonsterDeath()`가 호출됨.
    *   `LastDamageCauser`가 플레이어인지 확인.
    *   몬스터로부터 `ExpReward` 값을 가져와 플레이어에게 전달.
4.  **Player Progression**: `Player->AddExp()` 실행.
    *   경험치가 `MaxEXP`를 초과하면 레벨업.
    *   UI 갱신 (경험치 바 등).

### 3.2 퀘스트 시스템 (Quest Tracking)
*   `ProcessMonsterDeath`에서 `DeadMonster`의 ID나 태그를 확인하여 현재 진행 중인 퀘스트의 처치 카운트를 증가시킴. (추후 확장 예정)

---

## 4. 데이터 구조 (Data Structure)

### 4.1 보상 설정
*   각 몬스터는 `UHUnitProfileData` 또는 자체 변수를 통해 경험치 보상량을 정의합니다.
*   레벨업에 필요한 경험치(`MaxEXP`)는 플레이어의 레벨에 따라 동적으로 계산되거나 테이블(DataTable)을 통해 관리됩니다.

---

## 5. UI 연동 (UI Integration)

*   **HUD**: 플레이어의 현재 레벨과 경험치 비율을 `MainHud`에 실시간으로 표시.
*   **LevelUp Popup**: 레벨업 시 게임을 일시 정지하고 `UHSelectAbilityPopupUI`를 생성하여 플레이어에게 새로운 능력을 선택할 기회를 제공.

---

## 6. 향후 확장 계획 (Future Extensions)

*   **다양한 퀘스트 타입**: 수집 퀘스트(아이템 획득), 생존 퀘스트(시간 버티기) 등 추가.
*   **퀘스트 UI**: 현재 진행 중인 퀘스트 목록과 목표 수치를 보여주는 전용 위젯 구현.
*   **전적 시스템**: 이번 세션에서 처치한 총 몬스터 수, 획득 골드 등 통계 기록 시스템 연동.
