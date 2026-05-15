# Repository Guidelines

## 프로젝트 구조 및 모듈 구성
이 저장소는 Unreal Engine 5.5 기반 핵앤슬래시 프로젝트입니다. 메인 런타임 모듈은 `Source/MyHackSlash`의 `MyHackSlash`입니다.

- `Source/MyHackSlash/GAS`, `GAS/GA`: Gameplay Ability System 실행 계산, 어빌리티, 태스크, 타겟 액터
- `Source/MyHackSlash/Unit`: 플레이어, 몬스터, 기본 캐릭터, 애니메이션 클래스
- `Source/MyHackSlash/System`: 매니저, 전역 타입, 오브젝트 풀, 웨이브, 퀘스트, 사운드, UI 조정
- `Source/MyHackSlash/UI`: HUD, 능력 선택, 상점, 공통 UMG 위젯 클래스
- `Source/MyHackSlash/DataAsset`: 하드코딩을 피하기 위한 데이터 에셋 및 테이블 행 타입
- `Content`: Blueprint, 머티리얼, 텍스처, 메시, UI, 데이터 에셋
- `Config`: 엔진, 입력, Gameplay Tag, 프로젝트 설정

`Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, `.vs`는 생성물로 보고 소스처럼 수정하지 않습니다.

## 빌드, 테스트, 개발 명령
모든 명령은 저장소 루트에서 실행합니다.

```powershell
start UnrealEditor.exe MyHackSlash.uproject
```
Unreal Editor에서 프로젝트를 엽니다.

```powershell
& "$env:UE_5.5\Engine\Build\BatchFiles\Build.bat" MyHackSlashEditor Win64 Development -Project="$PWD\MyHackSlash.uproject" -WaitMutex
```
`UE_5.5` 환경 변수가 로컬 엔진 설치 경로를 가리킬 때 에디터 타겟을 빌드합니다.

```powershell
UnrealEditor-Cmd.exe MyHackSlash.uproject -ExecCmds="Automation RunTests Project; Quit" -unattended -nop4
```
프로젝트 Automation Test가 있을 경우 테스트를 실행합니다.

## 코딩 스타일 및 명명 규칙
Unreal C++ 스타일과 루트의 `Project_Standard_Convention.md`를 최우선으로 따릅니다. 주변 코드의 들여쓰기와 include 방식을 맞추고, 모든 헤더에는 `#pragma once`를 둡니다. 불필요한 include 대신 전방 선언을 우선 검토합니다.

커스텀 타입은 프로젝트 접두사 `H`를 사용합니다: `UH`는 `UObject`, `AH`는 `AActor`, `FH`는 구조체, `IH`는 인터페이스, `EH`는 enum입니다. 함수와 변수는 PascalCase, bool은 `b` 접두사, 파라미터는 `In`/`Out` 접두사를 사용합니다.

## 테스트 가이드라인
현재 별도 테스트 폴더는 없습니다. Gameplay 변경은 Editor PIE에서 검증하고, 결정적인 동작은 Automation Test 추가를 검토합니다. 테스트 이름은 기능과 동작을 드러내도록 작성합니다. 예: `FHGemInventory_AutoCombineTest`.

버그 수정 시에는 재현 경로, 수정 내용, 검증 절차를 함께 남기고 관련 Blueprint 및 Data Asset 참조가 깨지지 않았는지 확인합니다.

## 커밋 및 Pull Request 가이드라인
최근 커밋은 `상점 추가`, `치명타 텍스트 효과 사라진 문제 수정`처럼 짧은 한국어 요약을 사용합니다. 커밋 메시지는 변경 결과 중심으로 간결하게 작성합니다.

PR에는 변경 요약, 영향받는 시스템, 검증 방법을 포함합니다. UI, VFX, 애니메이션, Blueprint 노출 변경은 스크린샷이나 짧은 영상을 첨부하고, 필요한 에셋 또는 설정 마이그레이션을 명시합니다.

## Agent-Specific Instructions
모든 대화와 코드 설명은 한국어로 진행합니다. 기술 용어는 필요하면 영문을 병기합니다.

수정 전에는 관련 `~System_Design.md` 문서와 `Project_Standard_Convention.md`를 확인합니다. 새 클래스나 기능은 H 접두사, `In`/`Out` 파라미터, UI 데이터 정의 규칙을 따릅니다. 코드 변경 후 관련 `.md` 설계 문서도 최신 상태로 업데이트합니다.

구현 전에는 가정과 모호한 점을 명확히 합니다. 해석이 여러 개이면 차이를 설명하고, 더 단순한 접근이 있으면 함께 제시합니다. 작업은 요청 범위에 맞게 작게 유지하며, 단일 용도 추상화나 요청되지 않은 확장성은 추가하지 않습니다.

기존 코드는 필요한 줄만 수정합니다. 관련 없는 리팩터링, 포맷 변경, 주석 정리는 피하고 주변 스타일에 맞춥니다. 변경으로 생긴 미사용 import, 변수, 함수만 정리하며, 기존의 무관한 죽은 코드는 삭제하지 말고 언급만 합니다.

검증 가능한 목표를 기준으로 작업합니다. 버그 수정은 재현 경로를 먼저 확인하고, 기능 추가나 리팩터링은 테스트 또는 PIE 검증 기준을 정합니다. 게임플레이 수치는 C++에 하드코딩하지 말고 Data Asset 또는 설정 값으로 분리합니다. UI는 게임 상태를 직접 소유하지 않고, 컴포넌트, 매니저, delegate 기반 갱신 흐름을 사용합니다.
