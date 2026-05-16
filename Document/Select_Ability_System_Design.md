# 🎲 Select Ability System Design Document

본 문서는 레벨업 등의 조건 발동 시 플레이어에게 무작위 보상/능력 선택지를 제공하는 'Select Ability System'의 데이터 구조 및 작동 로직을 정의합니다. (뱀서류/로그라이트 방식의 핵심 시스템)

---

## 1. Core Enumerations (핵심 열거형)
시스템에서 사용되는 기본 타입들을 정의합니다.

| Enum Name | Values | Description |
| :--- | :--- | :--- |
| `EHAbilityGrade` | `Silver`, `Gold`, `Dia` | 보상 선택지의 등급 |
| `EHRewardType` | `GetSkillGem`, `GetGold`, `GetReroll` | 보상의 종류 (추후 확장 가능) |

---

## 2. Data Structures (데이터 구조)
기획자가 언리얼 에디터에서 `DataAsset` 또는 `DataTable`을 통해 확률과 보상을 직접 튜닝할 수 있도록 구조화합니다.

### 2.1 Grade Probability Data (`UHSelectAbilityGradeDataAsset`)
등급별 등장 확률을 정의합니다. 
* *참고: 모든 확률의 합은 100%가 되도록 설계합니다.*

| Grade (`EHAbilityGrade`) | Probability (%) |
| :--- | :--- |
| Silver | 70.0 |
| Gold | 20.0 |
| Dia | 10.0 |

### 2.2 Reward Option Data (`FHRewardOptionData` - DataTable)
각 등급별로 풀(Pool)에 들어갈 선택지들을 정의합니다.

| Field | Type | Description |
| :--- | :--- | :--- |
| OptionID | RowName | 데이터 식별자 |
| Grade | `EHAbilityGrade` | 등급 |
| RewardType | `EHRewardType` | 보상 종류 |
| TargetID | `FName` | 대상 식별자 (Gem Name 등) |
| Title | `FText` | UI 제목 (추가됨) |
| Tier | `int32` | 성급 |
| Amount | `int32` | 수량 |
| Icon | `TSoftObjectPtr<UTexture2D>` | UI 아이콘 (추가됨) |
| Description | `FText` | UI 설명 (추가됨) |

---

## 3. System Logic Flow (작동 로직)

1. **Trigger Event (발동):**
   * 플레이어 레벨업 등 지정된 조건 달성 시 `UHSelectAbilityManager::GetRandomRewardOptions()` 함수 호출.
2. **Roll Grade (등급 결정):**
   * `UHSelectAbilityManager::RollGrade()` 함수 내부 로직.
   * `UHSelectAbilityGradeDataAsset`을 기반으로 0~100 사이의 난수를 생성하여 이번 팝업의 **단일 등급(Grade)**을 결정합니다. (누적 확률 방식 사용)
3. **Fetch & Filter Pool (풀 필터링):**
   * `FHRewardOptionData` DataTable에서 결정된 `Grade`와 일치하는 모든 선택지를 필터링하여 풀을 생성합니다.
4. **Select Options (선택지 추출):**
   * 풀 내부를 무작위로 섞은 후, 최대 3개의 고유(Unique)한 요소를 추출하여 반환합니다.
5. **Show UI (UI 출력):**
   * 추출된 `TArray<FHRewardOptionData>`를 `SelectAbilityPopupUI`에 전달하여 화면에 띄웁니다.
6. **Execute Reward (보상 지급):**
   * 유저가 UI에서 옵션을 선택하면 `UHSelectAbilityManager::ExecuteReward()`를 호출하여 실제 보상 효과를 적용합니다.

### 3.1 Multiplayer Selection Flow

멀티플레이에서는 레벨업으로 인한 능력 선택을 모든 플레이어가 동시에 처리합니다.

1. 서버가 공유 경험치 증가로 레벨업을 판정합니다.
2. 서버는 게임을 일시정지하고 모든 플레이어에게 능력 선택 UI를 표시하도록 알립니다.
3. 각 플레이어는 10초 안에 선택지를 고릅니다.
4. 선택한 플레이어의 보상은 서버 검증 후 즉시 적용하되, 게임 재개는 전체 선택 단계가 종료될 때까지 대기합니다.
5. 10초가 지나도 선택하지 않은 플레이어는 서버가 해당 플레이어의 선택지 중 하나를 자동 선택합니다.
6. 모든 플레이어의 선택 처리가 끝나면 서버가 일시정지를 해제하고 게임을 재개합니다.

자동 선택 정책은 최초 구현에서는 첫 번째 선택지를 기본값으로 사용합니다. 이후 기획 필요에 따라 최고 등급 우선, 무작위 선택, 이전 선호 타입 우선 등으로 확장할 수 있습니다.

---

## 4. UI Specification (`UHSelectAbilityPopupUI`)

* **Base Class:** `UHSelectAbilityPopupUI` (C++)
* **Input:** `UHSelectAbilityManager::GetRandomRewardOptions()`로부터 추출된 `TArray<FHRewardOptionData>`.
* **Components:**
  * `AbilityListView` (`UListView`): 3개의 선택지를 리스트 항목으로 표시.
  * `UHSelectAbilityEntryUI`: 개별 선택지 항목 UI. `UHSelectAbilityData` (UObject)를 통해 데이터를 전달받음.
* **Logic:**
  * **On Show (Solo):** `NativeConstruct` 시점에 `UGameplayStatics::SetGamePaused(true)`를 호출하여 게임을 일시정지. (마우스 커서 등 입력 처리는 외부 시스템에서 관리)
  * **On Show (Multiplayer):** 서버가 선택 단계를 시작할 때 모든 클라이언트에 UI를 표시합니다. 일시정지와 10초 타이머는 서버 상태를 기준으로 관리하며, UI는 남은 시간을 표시하고 선택 요청만 서버에 전송합니다.
  * **On Select:** 유저가 항목의 `SelectButton`을 클릭하면 `UHSelectAbilityManager::ExecuteReward()`를 호출하여 보상을 지급하고, `RemoveFromParent()`를 통해 UI를 닫음.
  * **On Close (Solo):** `NativeDestruct` 시점에 `SetGamePaused(false)`를 호출하여 게임을 재개.
  * **On Close (Multiplayer):** 개별 UI가 닫혀도 게임은 즉시 재개하지 않습니다. 서버가 모든 플레이어의 선택 완료 또는 10초 타임아웃을 확인한 뒤 일괄 재개합니다.

---

## 5. Implementation Notes for AI (AI 구현 참고사항)

1. **Strategy / Command Pattern:** `ERewardType`이 계속 추가될 수 있으므로, 보상 지급 로직은 `switch-case`로 하드코딩하기보다, 각 보상 타입을 처리하는 인터페이스나 함수형 포인터(Delegate) 구조로 분리하여 OCP(개방-폐쇄 원칙)를 지킬 것.
2. **Random Weight Logic:** 등급을 뽑는 로직은 단순한 누적 확률(Cumulative Probability) 방식을 사용하여 `UDataAsset`의 값이 변동되어도 자동으로 대응되도록 구현할 것.
3. **Extensibility:** 추후 '슬롯별로 다른 등급이 나오는 기획'으로 변경될 가능성도 고려하여, 등급 롤링 로직을 유연하게 분리해 둘 것.
