# Need Fix

본 문서는 현재 `MyHackSlash` 코드베이스에서 정리 가치가 있는 불필요 코드, 복잡한 코드, 멀티플레이 전환 리스크를 추적합니다. 기준은 2026-05-18 현재 소스, `Project_Standard_Convention.md`, PIE Listen Server 검증 흐름입니다.

---

## P0. 멀티플레이 동작에 직접 영향

### 1. 플레이어 0 고정 참조 제거

대상:

- `Source/MyHackSlash/System/HSelectAbilityManager.cpp:99`
- `Source/MyHackSlash/System/HSelectAbilityManager.cpp:105`
- `Source/MyHackSlash/System/HSelectAbilityManager.cpp:112`
- `Source/MyHackSlash/System/HMonsterSpawnManager.cpp:92`

문제:

- `UGameplayStatics::GetPlayerPawn(GetWorld(), 0)` / `GetPlayerController(GetWorld(), 0)`가 남아 있습니다.
- 보상 지급과 몬스터 스폰 기준이 플레이어 0에 고정되어, 클라이언트 플레이어가 레벨업/보상/스폰 흐름에서 누락될 수 있습니다.

정리 방향:

- 보상 실행은 이미 있는 `ExecuteRewardForPlayer()` 경로만 남기고, 인자 없는 `ExecuteReward()`는 제거하거나 솔로 전용 래퍼로 명확히 제한합니다.
- 몬스터 스폰 위치는 모든 살아있는 플레이어 중 하나를 선택하거나, 플레이어별 스폰 예산/거리 정책을 `HMonsterSpawnManager`에 명시합니다.

### 2. UI 생성 시 Owning Player가 명확하지 않음

대상:

- `Source/MyHackSlash/System/HUIManager.cpp:24`
- `Source/MyHackSlash/System/HUIManager.cpp:53`
- `Source/MyHackSlash/System/HUIManager.cpp:83`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:373`

문제:

- `UHUIManager`가 `UGameInstanceSubsystem`인데 `CreateWidget<UUserWidget>(GetWorld(), LoadedClass)`로 위젯을 생성합니다.
- 멀티플레이에서 위젯의 owning player가 불명확하면 `GetOwningPlayer()` 기반 UI, 특히 `SelectAbilityPopupUI`가 플레이어 1에서 비어 있거나 뜨지 않을 수 있습니다.

정리 방향:

- `ShowWidgetByName(FName, APlayerController*, int32)` 또는 `ShowWidgetByName(FName, ULocalPlayer*, int32)` 형태로 호출자를 명시합니다.
- `PlayerController`의 client RPC에서 `UIManager->ShowWidgetByName(..., this, ZOrder)`로 생성하도록 정리합니다.
- `ActiveWidgets` 키도 전역 `FName` 하나가 아니라 로컬 플레이어별 맵으로 분리합니다.

### 3. 레벨업 선택창 pause/UI 정책 분리

대상:

- `Source/MyHackSlash/GAS/GA/HGA_LevelUp.cpp:73`
- `Source/MyHackSlash/GAS/GA/HGA_LevelUp.cpp:77`
- `Source/MyHackSlash/UI/SelectAbilityPopupUI/HSelectAbilityPopupUI.cpp:16`
- `Source/MyHackSlash/UI/SelectAbilityPopupUI/HSelectAbilityPopupUI.cpp:57`

문제:

- 현재 레벨업 어빌리티가 개별 캐릭터 기준으로 `BeginSelectAbilitySelection()`을 호출합니다.
- 선택창 위젯이 생성/파괴 시 직접 `SetGamePaused()`를 호출합니다.
- 멀티플레이 정책인 “모든 플레이어에게 선택 UI 표시, 10초 후 자동 선택, 모두 선택 후 재개”와 책임 위치가 맞지 않습니다.

정리 방향:

- 레벨업 선택 단계는 `GameMode` 또는 서버 전용 레벨업 매니저가 시작/종료를 소유합니다.
- 위젯은 pause를 직접 소유하지 않고, 서버 상태를 표시하고 선택 요청만 보냅니다.
- 플레이어별 선택 완료 상태는 `PlayerState`, 선택지/남은 시간은 `PlayerController` 또는 replicated 상태로 둡니다.

### 4. 웨이브/상점 pause와 UI 표시 책임 혼재

대상:

- `Source/MyHackSlash/System/HWaveManager.cpp:37`
- `Source/MyHackSlash/System/HWaveManager.cpp:43`
- `Source/MyHackSlash/System/HWaveManager.cpp:129`
- `Source/MyHackSlash/UI/ShopUI/HShopUI.cpp:174`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:407`

문제:

- `HWaveManager`, `ShopUI`, `PlayerController`가 모두 pause 상태를 직접 변경합니다.
- 상점 UI는 `HWaveManager`가 전역 `UIManager`로 직접 띄우고, 웨이브 결과 UI는 `PlayerController` client RPC로 띄웁니다. 같은 성격의 UI 표시 흐름이 두 방식으로 나뉘어 있습니다.

정리 방향:

- 서버는 웨이브 상태만 결정하고, 플레이어별 UI 표시는 owning client RPC로 일원화합니다.
- pause는 서버 권한 흐름에서만 변경하고, UI 클래스는 닫기 요청만 보내도록 제한합니다.

---

## P1. 복잡도와 유지보수 비용

### 5. `AMyHackSlashPlayerController` 책임 과다

대상:

- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:69`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:222`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:256`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:335`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:373`

문제:

- 입력, HUD 생성, 상점 구매 검증, 상점 보상 적용, 레벨업 선택지 생성/복제, 웨이브 결과 UI까지 한 클래스에 모여 있습니다.
- `// test` 주석이 남아 있고, `ApplyPurchasedShopItem()` 안에서 태그 문자열 판별, 즉석 GE 생성, 젬 ID 조립까지 수행합니다.

정리 방향:

- `PlayerController`는 클라이언트 요청/RPC와 owning client UI 표시만 맡깁니다.
- 상점 적용은 상점 서비스/매니저 또는 `HSelectAbilityManager`와 유사한 보상 적용 함수로 분리합니다.
- `BeginPlay()`의 `// test` 주석은 실제 HUD 생성 책임으로 이름을 바꾸거나 별도 `ShowMainHud()` 함수로 분리합니다.

### 6. 상점 효과가 문자열 매칭과 런타임 GE 생성에 의존

대상:

- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:270`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:273`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:280`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:298`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:322`

문제:

- `ItemTag.ToString().Contains(...)`로 Attribute를 판별합니다.
- `NewObject<UGameplayEffect>(GetTransientPackage())`로 임시 GE를 만들어 영구 스탯 보너스를 적용합니다.
- 젬 보상은 문자열 마지막 토큰과 `_T1` 조합에 의존합니다.

정리 방향:

- `FHShopRow`에 `FGameplayAttribute`, `TSubclassOf<UGameplayEffect>`, `FName GemFullId` 등 명시 필드를 두거나 태그-Attribute 매핑 테이블을 둡니다.
- 상점/보상 효과는 데이터 에셋 또는 GameplayEffect 클래스로 이동합니다.

### 7. 젬 인벤토리/장착 로직의 서버 객체와 표시 객체 혼재

대상:

- `Source/MyHackSlash/Skill/HGemInventoryComponent.cpp:30`
- `Source/MyHackSlash/Skill/HGemInventoryComponent.cpp:103`
- `Source/MyHackSlash/Skill/HEquipmentComponent.cpp:42`
- `Source/MyHackSlash/Skill/HEquipmentComponent.cpp:283`

문제:

- 서버는 실제 `UHGemBase` 객체 배열을 조작하고, 클라이언트는 `OnRep`에서 표시용 임시 `UHGemBase` 객체를 재생성합니다.
- 같은 배열 이름(`InventoryGems`, `EquippedMainGems`, `SlotSupportGems`)이 서버 권위 객체와 클라이언트 표시 객체를 모두 의미합니다.
- 장착, 인벤토리 제거, 보조젬 호환성, Ability 부여, 복제 데이터 동기화가 한 함수에 묶여 있습니다.

정리 방향:

- 권위 상태는 `FHGemInstanceData` 중심으로 두고, `UHGemBase`는 서버 런타임/클라이언트 표시 캐시임을 이름으로 분리합니다.
- `EquipGem()` 내부 단계를 `MoveFromInventory`, `ReplaceMainGem`, `GrantAbility`, `SyncReplicatedData`처럼 작게 나눕니다.
- 클라이언트 표시 객체 재구성은 별도 factory/helper로 모읍니다.

### 8. 투사체가 이동, 풀링, VFX, 데미지를 모두 소유

대상:

- `Source/MyHackSlash/Skill/HProjectile.cpp:61`
- `Source/MyHackSlash/Skill/HProjectile.cpp:79`
- `Source/MyHackSlash/Skill/HProjectile.cpp:175`
- `Source/MyHackSlash/Skill/HProjectile.cpp:216`
- `Source/MyHackSlash/Skill/HProjectile.cpp:296`

문제:

- `AHProjectile` 하나가 풀 상태 복구, Niagara 부착/반납, 폭발 스케일링, sweep 데미지, GAS 적용, fallback `TakeDamage`까지 처리합니다.
- 폭발 VFX는 서버 `Explode()` 안에서 풀링 Niagara를 직접 스폰하므로, 멀티플레이 연출 정책이 데미지 판정과 강하게 결합되어 있습니다.

정리 방향:

- 서버 데미지 판정과 cosmetic 폭발 연출을 분리합니다.
- 폭발 VFX/SFX는 GameplayCue 또는 multicast 전용 함수로 옮기고, 투사체는 충돌/수명/반납 상태만 명확히 관리합니다.
- `DamageEffectClass`가 없는 fallback은 임시 호환 코드인지 정책인지 결정하고 문서화합니다.

---

## P2. 청결도와 컨벤션

### 9. 깨진 한글 주석과 불필요한 디버그 로그 정리

대상:

- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:156`
- `Source/MyHackSlash/Skill/AnimNotifyState/HAnimNotifyState_Attack.h:15`
- `Source/MyHackSlash/System/HMonsterSpawnManager.cpp:97`
- `Source/MyHackSlash/Unit/Player/HPlayerCharacter.cpp:190`
- `Source/MyHackSlash/Unit/Player/HPlayerCharacter.cpp:211`
- `Source/MyHackSlash/Unit/Player/HPlayerCharacter.cpp:349`

문제:

- 일부 주석이 `���` 형태로 깨져 있습니다.
- 레벨 초기화, 경험치 획득, 굶주림 시작/종료 등 일반 플레이 중 반복될 수 있는 로그가 `Warning`/`LogTemp`로 남아 있습니다.

정리 방향:

- 깨진 주석은 삭제하거나 의미 있는 한국어 주석으로 복구합니다.
- 반복 로그는 제거하거나 전용 로그 카테고리와 `Verbose` 수준으로 낮춥니다.

### 10. include 스타일 불일치

대상:

- `Source/MyHackSlash/Mode/MyHackSlashGameMode.cpp:13`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.h:10`
- `Source/MyHackSlash/Mode/MyHackSlashPlayerController.cpp:27`
- `Source/MyHackSlash/Unit/HBaseCharacter.h:10`
- `Source/MyHackSlash/System/HSelectAbilityManager.cpp:9`

문제:

- 프로젝트 내부 헤더에 `#include <...>` 스타일이 섞여 있습니다.
- 주변 코드는 대부분 `"..."` include를 사용하므로 컨벤션과 가독성이 흔들립니다.

정리 방향:

- 프로젝트 내부 헤더는 `"..."`로 통일합니다.
- 헤더에서는 가능한 전방 선언으로 대체하고, 구현 파일에서 include합니다.

### 11. 하드코딩 에셋 경로

대상:

- `Source/MyHackSlash/Mode/MyHackSlashGameMode.cpp:29`
- `Source/MyHackSlash/Mode/MyHackSlashGameMode.cpp:36`
- `Source/MyHackSlash/System/HUIManager.cpp:11`

문제:

- `ConstructorHelpers`로 Blueprint/DataAsset 경로를 코드에 박아 둔 부분이 남아 있습니다.
- 에셋 경로 변경 시 C++ 수정/빌드가 필요하고, 테스트 맵이나 모드별 UI 구성이 어려워집니다.

정리 방향:

- GameMode 기본 Pawn/Controller와 UI DataAsset은 Blueprint 기본값 또는 프로젝트 설정/DataAsset 참조로 이동합니다.
- C++ 생성자에서는 fallback만 처리하거나 참조를 강제하지 않습니다.

---

## 권장 작업 순서

1. 플레이어 1 레벨업 선택창 미표시 수정: `HUIManager` owning player 명시, `ClientShowSelectAbilityPopup()` 호출/생성 흐름 확인.
2. `GetPlayerPawn(0)` / `GetPlayerController(0)` 제거: 보상과 스폰의 멀티플레이 기준 확정.
3. pause/UI 표시 책임 정리: `WaveManager`, `ShopUI`, `SelectAbilityPopupUI`, `PlayerController`의 역할 분리.
4. `MyHackSlashPlayerController` 비대화 축소: 상점 효과 적용과 보상 선택 상태를 별도 서비스/매니저로 이동.
5. 깨진 주석, include 스타일, 반복 디버그 로그 정리.

---

## 참고

- 현재 워킹트리에는 이전 멀티플레이/코인/젬 작업 변경이 남아 있으므로, 실제 리팩터링은 기능 단위로 작게 나누는 것이 안전합니다.
- 위 항목은 “즉시 버그”와 “기술 부채”를 섞어 정리한 목록입니다. P0은 멀티플레이 동작에 직접 영향이 있으므로 먼저 처리하는 것을 권장합니다.
