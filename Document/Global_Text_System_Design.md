# Global Text System Design Document

본 문서는 코드에 직접 박혀 있는 표시 텍스트를 언어별 DataTable 기반으로 분리하기 위한 글로벌 텍스트 시스템을 정의합니다.

---

## 1. Core Types

| Type | Description |
| :--- | :--- |
| `EHTextLanguage` | 현재 프로젝트에서 지원하는 언어 enum |
| `FHGlobalTextRow` | 언어별 텍스트 DataTable의 Row 구조체 |
| `FHLanguageTextTable` | 언어와 DataTable의 매핑 구조체 |
| `UHGlobalTextDataAsset` | 기본 언어, 폴백 언어, 언어별 DataTable 목록을 보관하는 DataAsset |
| `UHGlobalTextManager` | 현재 언어 캐시를 관리하고 텍스트 키로 `FText`를 반환하는 GameInstanceSubsystem |

---

## 2. DataTable Format

DataTable은 `FHGlobalTextRow`를 Row Structure로 사용합니다.

| Field | Type | Description |
| :--- | :--- | :--- |
| RowName | `FName` | 텍스트 키. `UI.StartButton`처럼 범주를 포함해 작성합니다. |
| Text | `FText` | 해당 언어로 표시할 텍스트입니다. |

권장 CSV 예시는 다음과 같습니다.

```csv
Name,Text
UI.StartButton,시작
UI.OptionButton,설정
UI.QuitButton,종료
Reward.Gold.Title,골드 획득
```

영어 테이블은 같은 RowName을 유지하고 `Text`만 교체합니다.

```csv
Name,Text
UI.StartButton,Start
UI.OptionButton,Options
UI.QuitButton,Quit
Reward.Gold.Title,Gold Acquired
```

---

## 3. Runtime Flow

1. `UHGlobalTextManager`가 GameInstanceSubsystem으로 초기화됩니다.
2. `TextDataAssetPath`에 설정된 `UHGlobalTextDataAsset`을 로드합니다.
3. `DefaultLanguage`의 DataTable을 읽어 `ActiveTextCache`를 구성합니다.
4. `FallbackLanguage`가 현재 언어와 다르면 별도 폴백 캐시를 구성합니다.
5. UI 또는 시스템은 RowName 기반 키를 `GetText(InTextKey)` 또는 `FindText(InTextKey, OutText)`에 전달해 텍스트를 가져옵니다.
6. 언어가 바뀌면 `SetActiveLanguage()`가 캐시를 재구성하고 `OnLanguageChanged`를 브로드캐스트합니다.

---

## 4. Editor Setup

1. Content Browser에서 `Data Table`을 생성하고 Row Structure를 `FHGlobalTextRow`로 선택합니다.
2. 언어별로 `DT_GlobalText_Korean`, `DT_GlobalText_English`처럼 테이블을 분리합니다.
3. `UHGlobalTextDataAsset` 기반 DataAsset을 생성합니다. 예: `DA_GlobalText`.
4. `DefaultLanguage`, `FallbackLanguage`, `LanguageTextTables`에 언어별 DataTable을 등록합니다.
5. 프로젝트 설정 또는 `DefaultGame.ini`에서 `UHGlobalTextManager`의 `TextDataAssetPath`에 `DA_GlobalText`를 지정합니다.

예시 ini:

```ini
[/Script/MyHackSlash.HGlobalTextManager]
TextDataAssetPath=/Game/Blueprint/DataAsset/DA_GlobalText.DA_GlobalText
```

---

## 5. Usage Example

C++:

```cpp
if (UHGlobalTextManager* TextManager = GetGameInstance()->GetSubsystem<UHGlobalTextManager>())
{
	const FText StartText = TextManager->GetText(TEXT("UI.StartButton"));
}
```

Blueprint:

1. `Get Game Instance`
2. `Get Subsystem`에서 `HGlobalTextManager` 선택
3. `GetText`에 텍스트 키 입력
4. 반환된 `FText`를 `TextBlock` 등에 설정

---

## 6. Migration Rule

하드코딩된 UI 표시 텍스트를 교체할 때는 한 번에 모든 코드를 바꾸지 않고, 화면 또는 시스템 단위로 키를 추가한 뒤 위젯의 텍스트 바인딩만 교체합니다. 기존 `FText` DataTable 필드는 즉시 삭제하지 말고, 해당 UI가 글로벌 텍스트 키를 정상적으로 참조하는지 PIE에서 확인한 뒤 정리합니다.
