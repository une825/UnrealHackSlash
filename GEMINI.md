# Gemini CLI Instructions

- 모든 대화와 코드 설명은 **한국어**로 진행해 주세요.
- 기술적인 용어는 문맥에 따라 영문을 병기하거나 적절한 한국어 용어를 사용해 주세요.

## 코딩 컨벤션 (Coding Conventions)

### 함수 파라미터 명명 규칙
- 함수의 모든 파라미터에는 용도에 따라 `In` 또는 `Out` 접두사를 붙여야 합니다.
  - 입력 파라미터: `In` (예: `InNewLevel`, `InDamageAmount`)
  - 출력 파라미터: `Out` (예: `OutResult`, `OutHitLocation`)
- 언리얼 엔진의 기존 오버라이드 함수라도 프로젝트 내 구현부에서는 가급적 이 규칙을 준수합니다.

### UI 데이터 정의 규칙
- `ListView`나 `TileView`의 항목으로 사용되는 `EntryUI` 전용 데이터 클래스(`UObject`)나 구조체(`FStruct`)는 해당 `EntryUI` 헤더 파일 내에 함께 정의하는 것을 원칙으로 합니다.
- 이는 파일 개수를 최소화하고 데이터와 UI 사이의 응집도를 높이기 위함입니다.

## 설계 및 개발 프로세스 (Design & Development Process)

- 코드 분석 및 수정 전에는 반드시 프로젝트 루트 경로(`C:\Une\Unreal\MyHackSlash`)에 있는 `~System_Design.md` 파일 중 관련이 있다고 판단되는 파일을 먼저 읽어야 합니다.
- 코드를 수정한 후에는 변경 사항에 맞춰 관련된 `.md` 설계 문서 파일들을 최신 상태로 업데이트해야 합니다.
