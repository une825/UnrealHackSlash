# 📜 Project Standard Convention

본 문서는 **MyHackSlash** 프로젝트의 통합 코딩 컨벤션 및 개발 표준을 정의합니다. 모든 팀원과 AI 에이전트는 본 가이드를 준수해야 합니다.

---

## 1. Naming Conventions

### 1.1 Class Prefixes (접두사)
모든 커스텀 클래스와 구조체에는 프로젝트 식별자 **H**를 포함한 접두사를 사용합니다.
*   **UObject 기반**: `UH` (예: `UHGemBase`)
*   **AActor 기반**: `AH` (예: `AHBaseCharacter`)
*   **UStruct 기반**: `FH` (예: `FHGemData`)
*   **Interface 기반**: `IH` (예: `IHCombatInterface`)
*   **Enum 기반**: `EH` (예: `EHUnitType`)

### 1.2 Function Parameters
함수 파라미터의 용도를 명확히 하기 위해 접두사를 사용합니다.
*   **Input 파라미터**: `In` (예: `InSlotIndex`, `InGemData`)
*   **Output 파라미터**: `Out` (예: `OutHitResult`, `OutOperation`)
*   *참고: 언리얼 엔진의 기존 오버라이드 함수라도 프로젝트 구현부에서는 가급적 이 규칙을 준수합니다.*

### 1.3 Variables & Functions
*   **함수명**: 파스칼 케이스(PascalCase)를 사용하며 동사로 시작합니다. (예: `InitializeStat()`)
*   **변수명**: 파스칼 케이스(PascalCase)를 사용합니다. (예: `CurrentHP`)
*   **불리언(Boolean)**: `b` 접두사를 사용합니다. (예: `bIsEmpty`, `bIsDead`)

---

## 2. Directory & Module Structure

`Source/MyHackSlash/` 하위의 폴더 구조는 기능별로 엄격히 분리합니다.

| Folder | Description |
| :--- | :--- |
| **AI** | 비헤이비어 트리 노드 (Decorator, Service, Task) 및 AI 컨트롤러 |
| **DataAsset** | 각종 데이터 에셋 및 테이블 구조체 (`UPrimaryDataAsset`, `FTableRowBase`) |
| **GA** | Gameplay Ability System 관련 클래스 (Abilities, Effects, Tasks) |
| **Mode** | 게임 모드 및 게임 스테이트 관련 클래스 |
| **Skill** | 젬 시스템, 투사체 로직 등 전투 기술의 핵심 로직 |
| **System** | 매니저 클래스 (UI, Pool, Quest), 전역 타입 정의 |
| **UI** | 위젯 클래스 및 UI 전용 데이터 객체 |
| **Unit** | 플레이어 및 몬스터 캐릭터 클래스, 애니메이션 인스턴스 |

---

## 3. UI Development Rules

### 3.1 Data-Driven UI (ListView/TileView)
*   **EntryUI & EntryData**: 리스트의 항목으로 사용되는 위젯(`EntryUI`)과 데이터 객체(`EntryData`)는 **동일한 헤더 파일** 내에 정의하는 것을 원칙으로 합니다. 이는 파일 개수를 최소화하고 응집도를 높이기 위함입니다.
*   **Ownership**: UI는 데이터를 직접 수정하지 않습니다. 데이터 변경이 필요할 경우 반드시 관련 컴포넌트(예: `UHEquipmentComponent`)의 함수를 호출해야 합니다.

### 3.2 Communication (Delegates)
*   **Binding**: UI 갱신은 델리게이트 바인딩을 통한 **이벤트 주도형(Event-driven)** 방식을 지향합니다.
*   **Initialization**: `NativeConstruct`에서 필요한 델리게이트를 바인딩하고, 최초 1회 `Refresh()`를 호출하여 초기 상태를 반영합니다.

---

## 4. Coding Standards

### 4.1 Header Safety
*   모든 헤더 파일 상단에는 `#pragma once`를 포함합니다.
*   불필요한 헤더 포함을 줄이기 위해 전방 선언(Forward Declaration)을 적극 활용합니다.

### 4.2 Modern C++ & UE5
*   **Smart Pointers**: `TObjectPtr<T>`를 `UPROPERTY` 변수에 사용하여 포인터 추적을 명확히 합니다.
*   **Null Checks**: 모든 포인터 사용 전에는 반드시 유효성 검사(`nullptr == Ptr` 또는 `IsValid(Ptr)`)를 수행합니다.
*   **Const Correctness**: 데이터를 변경하지 않는 함수는 `const`를 붙여 의도를 명확히 합니다.

---

## 5. Development Workflow

1.  **Research**: 수정 전 관련 설계 문서(`~System_Design.md`)를 반드시 숙지합니다.
2.  **Strategy**: 변경 사항에 대한 논리적 타당성을 검토합니다.
3.  **Execution**: 코드 수정 후에는 관련 설계 문서도 최신 상태로 업데이트해야 합니다.
4.  **Validation**: 모든 변경 사항은 실제 작동 여부를 검증해야 하며, 버그 수정 시에는 재현 경로를 먼저 확인합니다.
