# Listen Server First Multiplayer Audit

본 문서는 현재 `MyHackSlash` 프로젝트를 우선 리슨 서버(Listen Server) 멀티플레이 구조로 전환할 때 수정해야 할 항목을 정리합니다. 기준은 Unreal Engine 5.5, PIE 멀티플레이 검증, 서버 권한(authority) 중심 게임플레이, 클라이언트 로컬 UI/입력 분리입니다.

데디케이티드 서버(Dedicated Server)는 추후 패키징/운영 단계의 확장 목표로 둡니다. 현재는 Epic Games Launcher 배포판 UE 5.5에서 서버 타겟 빌드가 제한되므로, 1차 구현과 검증은 PIE Listen Server 및 PIE Dedicated Server 옵션을 기준으로 진행합니다.

---

## 1. 현재 상태 요약

현재 프로젝트는 대부분 싱글 플레이 기준으로 작성되어 있습니다. `AHPlayerCharacter`는 `PossessedBy()`와 `OnRep_PlayerState()`에서 ASC를 초기화하고 있어 멀티플레이 전환의 출발점은 있지만, 실제 복제 설정과 서버 권한 분리는 아직 부족합니다.

가장 큰 문제는 다음과 같습니다.

| 영역 | 현재 상태 | 리슨 서버 전환 시 문제 |
| :--- | :--- | :--- |
| 멀티플레이 실행 | 싱글 플레이 기준 흐름 | PIE Listen Server 기준 접속/권한 검증 필요 |
| 데디 서버 빌드 | `MyHackSlash.Target.cs`, `MyHackSlashEditor.Target.cs`만 존재 | 실제 데디 서버 패키징은 소스 빌드 엔진 준비 후 검증 |
| ASC/Attribute | `PlayerState`에 ASC가 있으나 복제 설정 없음 | 클라이언트 UI/상태 동기화 실패 가능 |
| 캐릭터 상태 | `IsDead`, `Attackable` 등 일반 변수 | 사망/공격 상태가 클라이언트에 동기화되지 않음 |
| 젬 인벤토리/장착 | `UObject` 젬 배열을 로컬 컴포넌트가 직접 변경 | 클라이언트 조작, 비복제 UObject, 서버 상태 불일치 |
| 스킬 자동 발동 | `UHEquipmentComponent::TickComponent()`에서 항상 실행 | 클라이언트와 서버가 중복 발동할 수 있음 |
| 투사체/몬스터/코인 | 풀 매니저가 일반 `SpawnActor` 후 숨김 처리 | 복제 액터 풀링 정책 필요 |
| 웨이브/스폰 | `UWorldSubsystem` 내부 상태로만 관리 | `WorldSubsystem` 상태는 클라이언트에 자동 복제되지 않음 |
| UI/일시정지 | 서버/클라이언트 구분 없이 UI와 `SetGamePaused()` 호출 | 리슨 서버 호스트와 클라이언트 UI 역할이 섞일 수 있고, pause는 모든 플레이어에 영향 |
| 경험치/레벨업 선택 | 플레이어 단일 흐름 기준 | 멀티플레이 공유 경험치, 2배 필요 경험치, 동시 선택 UI 정책 필요 |
| 보상/경제 | `PlayerState::AddGold()` 직접 호출 | 서버 검증 없이 클라이언트에서 보상 조작 가능 |
| 사운드/VFX | GameplayCue와 로컬 재생이 혼재 | 서버에서 불필요한 사운드 재생, 일부 클라이언트 미표시 가능 |

---

## 2. 필수 수정 항목

### 2.1 리슨 서버 PIE 실행 준비

대상 파일:

| 파일 | 조치 |
| :--- | :--- |
| `Source/MyHackSlash.Target.cs` | 기존 게임 타겟 유지 |
| `Source/MyHackSlashEditor.Target.cs` | PIE 멀티플레이 검증용 에디터 타겟 유지 |
| `Mode/MyHackSlashPlayerController.cpp` | 로컬 컨트롤러에서만 HUD 생성 |
| `UI/HIntroUI.cpp` | 리슨 서버 호스트의 게임 시작 시 `ServerTravel` 사용 |
| `System/HUIManager.*` | 서버/비로컬 환경에서 위젯 생성 방지 |
| `System/HSoundManager.*` | 서버 역할에서 불필요한 로컬 사운드 재생 방지 |

필요 작업:

1. PIE `Number of Players = 2` 이상으로 실행 가능해야 합니다.
2. `Net Mode = Play As Listen Server`에서 호스트와 클라이언트가 각각 정상 HUD를 가져야 합니다.
3. 호스트는 서버 권한과 로컬 플레이어 역할을 동시에 가지므로, `HasAuthority()`와 `IsLocalController()`를 혼동하지 않도록 분기합니다.
4. UI와 사운드는 로컬 플레이어 기준으로만 실행하고, 게임플레이 판정은 서버 권한 기준으로 실행합니다.
5. 데디 서버 타겟은 추후 확장 준비로 유지할 수 있지만 1차 검증의 필수 조건은 아닙니다.

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `Source/MyHackSlashServer.Target.cs` 추가 | 완료, 추후 데디 서버 확장용 |
| `UHUIManager` 데디 서버 위젯 생성 차단 | 완료 |
| `UHSoundManager` 데디 서버 사운드 재생 차단 | 완료 |
| `AMyHackSlashPlayerController` 로컬 컨트롤러에서만 HUD 생성 | 완료 |
| `UHIntroUI` 리슨 서버 시작 시 `ServerTravel(Map?listen)` 사용 | 완료 |
| `AHIntroGameMode` 데디 서버 인트로 UI 생성 차단 | 완료 |
| 서버 빌드/실행 검증 | 현재 Launcher 배포판 UE 5.5에서 `Server targets are not currently supported from this engine distribution.`로 차단됨 |

주의:

- 리슨 서버 구현은 `MyHackSlashServer.Target.cs` 빌드가 없어도 PIE에서 검증할 수 있습니다.
- 데디케이티드 서버 타겟을 실제로 빌드하려면 서버 타겟을 지원하는 Unreal Engine 소스 빌드 설치본이 필요합니다.
- 현재 Launcher 배포판에서는 `MyHackSlashServer.Target.cs`를 추가해도 서버 타겟 빌드 단계에서 UnrealBuildTool이 중단됩니다.
- 기존 에디터 타겟(`MyHackSlashEditor Win64 Development`) 빌드는 통과해야 합니다.

예상 검증:

| 검증 | 방법 |
| :--- | :--- |
| 에디터 빌드 | `MyHackSlashEditor Win64 Development` 빌드 |
| PIE 리슨 서버 | Play 옵션에서 `Number of Players = 2`, `Net Mode = Play As Listen Server` |
| PIE 클라이언트 분리 | 필요 시 `Run Under One Process` 해제 후 별도 창으로 검증 |
| 추후 데디 서버 | 소스 빌드 엔진 준비 후 `MyHackSlashServer Win64 Development` 빌드 |

---

### 2.2 ASC와 AttributeSet 복제 설정

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Unit/Player/HPlayerState.*` | ASC 생성만 하고 `SetIsReplicated(true)`와 replication mode 설정 없음 | ASC 복제 활성화, `Mixed` 또는 프로젝트 정책에 맞는 replication mode 설정 |
| `Unit/Monster/HBaseMonster.*` | 몬스터 ASC도 복제 설정 없음 | 몬스터 ASC 복제 정책 결정 |
| `Attribute/HCharacterAttributeSet.*` | Attribute에 `ReplicatedUsing`/`DOREPLIFETIME_CONDITION_NOTIFY` 없음 | HP, MaxHP, Hunger, Gold, Level 등 복제 및 `OnRep` 추가 |
| `Unit/HBaseCharacter.*` | Attribute 변경 델리게이트에 UI 갱신 의존 | 복제된 Attribute 변경이 클라이언트에서 정상 브로드캐스트되는지 검증 |

우선순위가 높은 Attribute:

| Attribute | 이유 |
| :--- | :--- |
| `Health`, `MaxHealth` | HP UI, 사망 판정 표시 |
| `Hunger`, `MaxHunger` | 플레이어 생존 UI |
| `MovementSpeed` | 클라이언트 이동 체감과 보정 |
| `Experience`, `MaxExperience`, `Level` | 레벨업 UI/보상 |
| `Gold`, `MaxGold` | 상점/웨이브 보상 |
| `AttackDamage`, `AttackRange`, `AttackRadius`, `AttackSpeedRate`, `CriticalRate`, `CriticalMultiplier` | 클라이언트 예측/표시, 서버 데미지 계산 기준 |

메모:

- `PlayerState`에 ASC를 둔 구조는 멀티플레이에 적합합니다.
- 단, `AHPlayerState::AddGold()`와 `ConsumeGold()`는 서버 권한에서만 실행되도록 제한해야 합니다.

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `AHPlayerState` ASC 복제 활성화 | 완료 |
| `AHPlayerState` ASC replication mode를 `Mixed`로 설정 | 완료 |
| `AHPlayerState` 네트워크 갱신 빈도 조정 | 완료 |
| `AHBaseMonster` ASC 복제 활성화 | 완료 |
| `AHBaseMonster` ASC replication mode를 `Minimal`로 설정 | 완료 |
| `UHCharacterAttributeSet` 주요 Attribute `ReplicatedUsing` 추가 | 완료 |
| `UHCharacterAttributeSet` `OnRep`/`GetLifetimeReplicatedProps` 추가 | 완료 |
| `AHPlayerState::AddGold()`/`ConsumeGold()` 서버 권한 제한 | 완료 |
| `AHBaseCharacter` 기본 actor/movement replication 활성화 | 완료 |
| PIE Listen Server에서 HP/Gold/Exp/Hunger UI 갱신 검증 | 진행 필요 |

---

### 2.3 캐릭터 상태 복제와 서버 권한 분리

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Unit/HBaseCharacter.*` | `IsDead`, `Attackable`, `LastDamageCauser`가 비복제 | `IsDead`는 `ReplicatedUsing=OnRep_IsDead` 검토, 공격 가능 상태는 GameplayTag 또는 서버 권한 상태로 관리 |
| `Unit/HBaseCharacter.cpp` | `SetDead()`에서 래그돌/충돌/물리 처리를 즉시 실행 | 서버 사망 판정 후 클라이언트에는 `OnRep` 또는 GameplayCue/Multicast로 연출 |
| `Unit/Player/HPlayerCharacter.cpp` | `SetDead()`에서 `SetGlobalTimeDilation()` 사용 | 멀티플레이에서는 한 플레이어 사망으로 전체 월드 시간 변경 금지 |
| `Unit/HBaseCharacter.cpp` | `HandleHUDDamageEffect()`가 대상 캐릭터 기준 UI 호출 | 로컬 플레이어에게만 UI 효과 실행하도록 `IsLocallyControlled()` 또는 owning client RPC 사용 |
| `UI/Settings/HSettingsUI.*` | 설정창이 열릴 때 항상 `SetGamePaused()` 호출 | 솔로에서만 일시정지, 멀티플레이에서는 로컬 UI만 열고 서버 게임은 계속 진행 |

권장 방향:

1. 실제 데미지와 사망 판정은 서버에서만 수행
2. 사망 상태는 GAS DeadTag 또는 복제 변수 중 하나로 일원화
3. 래그돌은 서버 물리 복제를 쓸지, 클라이언트 연출용으로만 쓸지 정책 결정
4. 카메라 쉐이크, HUD 피격 효과는 owning client에서만 실행
5. 설정창 일시정지는 솔로 플레이 전용으로 제한

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `AHBaseCharacter::IsDead`를 `ReplicatedUsing=OnRep_IsDead`로 복제 | 완료 |
| `AHBaseCharacter::Attackable`, `LastDamageCauser` 복제 | 완료 |
| `OnDeadTagChanged()`에서 서버만 `SetDead()` 실행 | 완료 |
| `OnRep_IsDead()`에서 클라이언트 사망 연출 적용 | 완료 |
| `OnRep_IsDead()`에서 풀 재사용 시 클라이언트 생존 상태 복구 | 완료 |
| `AHPlayerCharacter` 사망 시 `SetGlobalTimeDilation()` 제거 | 완료 |
| 플레이어 사망 카메라/BGM 연출을 로컬 플레이어에서만 실행 | 완료 |
| `HandleHUDDamageEffect()`를 로컬 플레이어에서만 실행 | 완료 |
| `AHBaseMonster::SetDead()` 보상/드랍/풀 반납 서버 권한 제한 | 완료 |
| 풀 재사용 몬스터 AIController 유지 및 사망 즉시 BT 정지/재시작 정책 정리 | 완료 |
| 풀 재사용 몬스터 생존 복구 시 이동 속도/Blackboard 초기화 | 완료 |
| 멀티플레이 몬스터 타겟 선택 안정화 | 완료 |
| 클라이언트 몬스터 이동/회전 표시용 NetUpdateFrequency 조정 | 완료 |
| 몬스터 이동/공격 중 CharacterMovement 회전과 BT 타겟 회전 충돌 분리 | 완료 |
| 몬스터 MoveTo 경로 추종 회전 품질 보정 | 완료 |
| 래그돌 물리 복제 품질 및 클라이언트별 보정 | 진행 필요 |
| `HSettingsUI` 멀티플레이 일시정지 분리 | 완료 |

---

### 2.4 입력과 어빌리티 발동 흐름 정리

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Unit/Player/HPlayerCharacter.cpp` | `GASInputPressed()`가 로컬에서 바로 `TryActivateAbility()` 호출 | GAS prediction key 정책을 명확히 하고 클라이언트 예측/서버 승인 흐름 검증 |
| `Skill/HEquipmentComponent.cpp` | 자동 발동 스킬이 모든 인스턴스의 Tick에서 실행 | `GetOwner()->HasAuthority()` 조건으로 서버에서만 자동 발동 |
| `GAS/GA/HGA_Attack.cpp` | 공격 이벤트/몽타주 흐름이 로컬/서버 구분 없음 | 데미지 판정은 서버, 몽타주는 예측 또는 복제 정책 결정 |
| `GAS/GA/HGA_AttackHitCheck.cpp` | 타겟 데이터 기반 데미지 적용에 서버 권한 체크 없음 | 서버에서만 `ApplyGameplayEffectSpecToTarget()` 실행 |
| `GAS/GA/HGA_ProjectileBase.cpp` | 투사체 생성이 권한 체크 없이 실행 | 서버에서만 복제 투사체 생성 또는 cosmetic-only 투사체 분리 |

주의:

- 최근 적용한 “대표 슬롯만 몽타주 재생” 규칙은 `InputID == 1`을 기준으로 동작합니다. 멀티플레이에서도 장착 상태와 `InputID`가 서버 기준으로 동기화되어야 유지됩니다.
- 자동 슬롯은 서버에서만 발동해야 중복 투사체와 중복 데미지를 막을 수 있습니다.

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `UHEquipmentComponent::UpdateAutoCast()` 서버 권한 제한 | 완료 |
| `UHGA_AttackHitCheck` 서버에서만 타겟 데이터 처리 및 GE 적용 | 완료 |
| `UHGA_ProjectileBase::SpawnProjectiles()` 서버 권한 제한 | 완료 |
| `AHProjectile` actor/movement replication 활성화 | 완료 |
| `AHProjectile` 데미지/속성/반경/수명/진영 데이터 복제 | 완료 |
| `AHProjectile` 충돌/폭발/데미지 처리를 서버에서만 실행 | 완료 |
| `UHMonsterSpawnManager` 스폰/사망 보고를 서버 월드에서만 실행 | 완료 |
| GameplayCue 기반 투사체 폭발 VFX/SFX 정리 | 진행 필요 |
| 복제 액터 풀링 수명/숨김/dormancy 정책 정리 | 완료 |
| NiagaraActor 풀링 시 보조 표시 컴포넌트 숨김 및 반납 시 detach | 완료 |

---

### 2.5 젬 인벤토리/장착 데이터 구조 재설계

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Skill/HGemInventoryComponent.*` | `TArray<TObjectPtr<UHGemBase>> InventoryGems`는 기본적으로 네트워크 복제에 부적합 | `FHGemInstanceData` 같은 `USTRUCT` 기반 복제 데이터로 전환 검토 |
| `Skill/HEquipmentComponent.*` | 장착 슬롯도 `UHMainGem*`, `UHSupportGem*` 직접 보관 | 슬롯 상태를 `GemInstanceId` 또는 `FHGemSlotState`로 복제 |
| `Skill/HGemInventoryComponent.cpp` | `AddGem`, `RemoveGemInstance`, 자동 합성이 로컬 호출 가능 | 서버 RPC로 요청, 서버 검증 후 복제 |
| `Skill/HEquipmentComponent.cpp` | `EquipGem`, `UnequipGem`, `EquipSupportGem`이 직접 상태 변경 | `ServerEquipGem`, `ServerUnequipGem`, `ServerEquipSupportGem` 추가 필요 |

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `FHGemInstanceData` 표시용 복제 데이터 추가 | 완료 |
| `UHGemInventoryComponent` 인벤토리 표시 상태 복제 및 `OnRep` UI 갱신 | 완료 |
| `UHEquipmentComponent` 메인 젬 슬롯 표시 상태 복제 및 `OnRep` UI 갱신 | 완료 |
| `AHGameState`를 통한 클라이언트 젬 데이터 에셋 참조 복제 | 완료 |
| 젬 인스턴스 식별용 `FGuid InstanceId` 추가 | 완료 |
| 메인 젬 장착/해제 서버 RPC | 완료 |
| 보조 젬 장착/해제 서버 RPC | 완료 |
| 보조 젬 슬롯 표시 상태 복제 | 완료 |
| 합성 서버 검증 및 악의적 호출 방어 | 완료 |
| 클라이언트 드래그/드롭 장착 RPC 슬롯/소유 검증 | 완료 |
| PIE Listen Server에서 클라이언트 드래그/드롭 장착 검증 | 진행 필요 |

권장 데이터 예시:

```cpp
USTRUCT(BlueprintType)
struct FHGemInstanceData
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid InstanceId;

	UPROPERTY()
	FName GemID;

	UPROPERTY()
	int32 Tier = 1;

	UPROPERTY()
	HEGemCategory GemCategory = HEGemCategory::Main;
};
```

권장 방향:

1. 서버가 인벤토리와 장착 상태의 단일 소유자
2. 클라이언트 UI는 복제된 배열을 읽고 변경 요청만 전송
3. `UHGemBase` UObject는 런타임 계산용 임시 객체로 제한하거나, 서버 전용 내부 캐시로 사용
4. 장착 변경 시 서버가 GAS Ability 부여/제거를 수행

---

### 2.6 투사체, 몬스터, 코인, 오브젝트 풀링 네트워크 정책

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Skill/HProjectile.*` | `bReplicates` 설정 없음, 데미지/폭발이 모든 인스턴스에서 실행 가능 | 서버 스폰, 복제 이동, 서버 폭발/데미지 적용 |
| `System/HObjectPoolManager.*` | 액터를 숨기고 충돌 끄는 방식이 네트워크 상태와 충돌 가능 | 복제 액터 풀링 정책 별도 설계 |
| `System/HMonsterSpawnManager.*` | 스폰 타이머가 권한 체크 없이 실행 | 서버에서만 스폰 |
| `Unit/Monster/HBaseMonster.*` | 사망, 코인 드랍, 풀 반납이 권한 체크 없이 실행 | 서버 사망 처리, 드랍 스폰, 풀 반납 |
| `Item/HCoin.*` | 골드 지급이 overlap에서 직접 `PS->AddGold()` 호출 | 서버 overlap에서만 지급, 획득 피드백은 owning client 또는 multicast |

권장 정책:

- 몬스터, 투사체, 코인처럼 게임플레이 결과가 있는 액터는 서버에서만 생성/판정합니다.
- Niagara, 데미지 텍스트, 커서 FX처럼 결과가 없는 연출은 클라이언트 로컬 또는 GameplayCue로 처리합니다.
- 복제 액터를 풀링할 경우 `SetActorHiddenInGame`, collision, movement, owner, instigator, dormancy, relevancy를 함께 리셋해야 합니다.
- 초기 전환 단계에서는 풀링을 잠시 끄고 `Destroy()` 기반으로 멀티플레이 정확성을 먼저 검증하는 편이 안전합니다.

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `UHMonsterSpawnManager::StartMonsterWave()` 서버 권한 제한 | 완료 |
| `UHMonsterSpawnManager::ExecuteSpawnTick()` 서버 권한 제한 | 완료 |
| `UHMonsterSpawnManager::OnMonsterDied()` 서버 권한 제한 | 완료 |
| 풀에서 재사용되는 몬스터 hidden/dormancy 상태 NetUpdate 보강 | 완료 |
| 몬스터 복제 표시 PIE 검증 | 진행 필요 |
| `AHCoin` actor/movement replication 및 서버 overlap 골드 지급 | 완료 |
| `AHCoin` 물리 바디와 플레이어 획득 trigger 분리 | 완료 |
| `AHCoin` 클라이언트 pickup 요청과 서버 거리 검증 | 완료 |
| `AHCoin` 획득 후 클라이언트 잔상 방지용 multicast 비활성화 | 완료 |
| `AHCoin` 획득 클라이언트 로컬 즉시 숨김 및 서버 검증 후 풀 반납 | 완료 |
| `AHPotionItem` actor/movement replication 및 서버 overlap 효과 적용 | 완료 |
| `AHMagnetItem` actor/movement replication 및 서버 overlap 효과 적용 | 완료 |
| 아이템 획득 피드백 owning client 또는 multicast 분리 | 진행 필요 |

---

### 2.7 웨이브/스폰 상태를 GameState 계층으로 이동

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `System/HWaveManager.*` | `UWorldSubsystem` 내부의 `CurrentState`, `CurrentWaveIndex`, `CurrentTimer`, `CurrentKillCount`는 복제되지 않음 | `AHGameState` 또는 replicated actor/component로 상태 이동 |
| `System/HMonsterSpawnManager.*` | `UGameplayStatics::GetPlayerPawn(GetWorld(), 0)` 기준 스폰 | 멀티플레이용 타겟 플레이어 선택 정책 필요 |
| `System/HWaveManager.cpp` | `SetGamePaused(true)`로 결과 UI/상점 처리 | 멀티플레이에서는 전체 pause 대신 서버 상태 전환 + 개별 UI 표시 |
| `System/HWaveManager.cpp` | `CalculateInterest()`가 player 0만 대상으로 처리 | 모든 플레이어 또는 개인별 보상 정책 필요 |

권장 방향:

1. `AHMyHackSlashGameState` 신규 생성
2. 웨이브 상태와 진행률을 `GameState`에 복제
3. `GameMode`는 서버 전용으로 웨이브 시작/종료를 결정
4. 클라이언트 UI는 `GameState`의 `OnRep` 또는 delegate로 갱신
5. 협동 플레이 보상인지 개인 보상인지 정책 확정

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `FHReplicatedWaveState`를 `AHGameState`에 추가 | 완료 |
| `UHWaveManager`가 웨이브 시작/진행/종료/대기 상태를 `AHGameState`에 반영 | 완료 |
| `UHMainHudUI`가 `AHGameState` 복제 웨이브 상태 변경으로 웨이브 UI 갱신 | 완료 |
| `WaveResultUI`를 각 owning client에 RPC로 표시 | 완료 |
| `CalculateInterest()`가 모든 플레이어에게 개인별 이자 지급 | 완료 |
| 웨이브 결과/상점 UI pause 정책의 멀티플레이 분리 | 진행 필요 |

---

### 2.8 UI, 설정, 사운드의 로컬 클라이언트 분리

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Mode/MyHackSlashPlayerController.cpp` | `BeginPlay()`에서 무조건 `MainHUD` 생성 | `IsLocalController()` 조건 추가 |
| `System/HUIManager.*` | `CreateWidget(GetWorld())` 기반 전역 UI 매니저 | 로컬 플레이어/PlayerController를 명시해서 위젯 생성 |
| `UI/Settings/HSettingsUI.*` | 설정 UI가 `SetGamePaused()` 호출 | 멀티플레이에서는 설정 중 개인 입력만 막고 서버 게임은 계속 진행 |
| `System/HSoundManager.*` | `WorldSubsystem`에서 서버도 사운드 로직 실행 가능 | 사운드는 클라이언트에서만 재생 |
| `GAS/HGCN_CharacterHitted.cpp` | GameplayCue에서 HUD/사운드/VFX 모두 호출 | 서버 실행 방지, 로컬 플레이어 대상 UI만 실행 |

권장 방향:

- UI는 `PlayerController` 또는 `LocalPlayer` 기준으로 생성합니다.
- 데디 서버에는 viewport가 없으므로 모든 `CreateWidget`, `AddToViewport`, 로컬 사운드 호출은 클라이언트 전용 분기가 필요합니다.
- 설정 저장(`UHSettingsManager`)은 클라이언트 로컬 SaveGame으로 유지해도 됩니다.
- 설정창은 솔로 플레이에서만 게임을 일시정지합니다. 멀티플레이에서는 해당 클라이언트의 입력 모드와 UI 포커스만 변경하고 서버 pause를 호출하지 않습니다.

### 2.8.1 Multiplayer Level-Up Pause Policy

멀티플레이에서도 모든 일시정지를 금지하는 것은 아닙니다. 레벨업 능력 선택은 전투 흐름과 보상 형평성에 직접 영향을 주므로 예외적으로 서버가 전체 게임을 일시정지합니다.

정책:

1. 멀티플레이 경험치는 파티 공유 경험치로 누적합니다.
2. 멀티플레이 레벨업 필요 경험치는 솔로 플레이 기준의 2배입니다.
3. 공유 경험치가 레벨업 기준에 도달하면 서버가 능력 선택 단계를 시작합니다.
4. 서버는 전체 게임을 일시정지하고 모든 플레이어에게 선택 UI를 표시합니다.
5. 선택 제한 시간은 10초입니다.
6. 10초 안에 선택하지 않은 플레이어는 서버가 자동으로 하나의 선택지를 선택합니다.
7. 모든 선택 처리가 끝나면 서버가 일시정지를 해제합니다.

구현 위치 권장:

| 책임 | 권장 위치 |
| :--- | :--- |
| 공유 경험치, 현재 공유 레벨, 다음 레벨 필요 경험치 | `GameState` 또는 서버 권한 replicated component |
| 레벨업 판정과 선택 단계 시작/종료 | `GameMode` 또는 서버 전용 레벨업 매니저 |
| 10초 선택 타이머 | 서버 권한 타이머 |
| 플레이어별 선택 완료 상태 | `PlayerState` |
| 선택 UI 표시/남은 시간 표시 | owning client UI |
| 미선택 자동 선택 | 서버 |

---

### 2.9 맵 생성과 네비게이션 동기화

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `Unit/Player/HPlayerCharacter.cpp` | 모든 플레이어 Tick에서 `MapManager->UpdateMap(GetActorLocation())` 호출 | 서버 기준 업데이트로 제한 |
| `System/HInfiniteMapManager.*` | 플레이어 1명의 위치만 기준으로 타일 생성/제거 | 여러 플레이어 위치를 고려하거나 세션 정책 결정 |
| `System/HMapTile.*`, `System/HMapProp.*` | 타일/프롭 복제 여부 확인 필요 | 서버 생성 타일 복제 또는 클라이언트 deterministic 생성 중 선택 |

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `AHPlayerCharacter::Tick()` 맵 업데이트 서버 권한 제한 | 완료 |
| `AHMapTile` actor/movement replication 활성화 | 완료 |
| `AHMapProp` actor/movement replication 활성화 | 완료 |
| 다중 플레이어 기준 타일 유지 범위 계산 | 진행 필요 |
| 복제 액터 풀링 비활성화/반납 상태 동기화 정책 | 진행 필요 |

권장 정책:

- 협동 생존 게임이라면 서버가 모든 플레이어의 bounding area를 계산해 필요한 타일을 유지합니다.
- 타일과 프롭이 충돌/전투에 영향을 준다면 서버 생성 + 복제가 안전합니다.
- 순수 배경이면 클라이언트 로컬 생성도 가능하지만 랜덤 seed를 서버에서 내려줘야 시각 일관성이 유지됩니다.

---

### 2.10 보상 선택, 상점, 경제 서버 검증

대상 파일:

| 파일 | 문제 | 조치 |
| :--- | :--- | :--- |
| `System/HSelectAbilityManager.*` | `GameInstanceSubsystem`에 보상 상태와 reroll 횟수 저장 | 플레이어별 상태는 `PlayerState` 또는 PlayerController owned component로 이동 |
| `System/HSelectAbilityManager.cpp` | `UGameplayStatics::GetPlayerPawn(GetWorld(), 0)` 기준 보상 지급 | 선택한 플레이어의 owning client/server RPC 흐름 필요 |
| `UI/ShopUI/*` | 상점 구매가 서버 검증 없이 호출될 가능성 | 서버 RPC로 구매 요청, 서버에서 골드 검증 |
| `Unit/Player/HPlayerState.cpp` | `AddGold`, `ConsumeGold`가 권한 체크 없음 | 서버 전용 실행, 클라이언트 요청은 RPC로 제한 |
| `Unit/Player/HPlayerCharacter.cpp` | 레벨업 경험치가 플레이어 개별 처리 중심 | 멀티플레이에서는 공유 경험치와 2배 요구량 기준으로 레벨업 |

구현 메모:

| 항목 | 상태 |
| :--- | :--- |
| `AMyHackSlashPlayerController::ServerPurchaseShopItem()` 추가 | 완료 |
| 상점 UI가 `RowName`만 서버에 보내고 서버가 현재 웨이브 `ShopRewardTable`에서 재조회 | 완료 |
| 상점 구매 골드 소비를 서버 `AHPlayerState::ConsumeGold()`에서 검증 | 완료 |
| 상점 젬/스탯/즉시 효과 적용 서버 권한화 | 완료 |
| `FHRewardOptionEntry`로 보상 선택지 `RowName` 보존 | 완료 |
| `AHGameState`를 통한 클라이언트 보상 선택 데이터 참조 복제 | 완료 |
| `AMyHackSlashPlayerController::ServerSelectAbilityReward()` 추가 | 완료 |
| 보상 선택 UI가 `RowName`만 서버에 보내고 서버가 보상 테이블에서 재조회 | 완료 |
| 보상 지급 대상 owning 플레이어 기준 적용 | 완료 |
| 서버가 선택지 3개를 생성하고 `PlayerController`에 복제 | 완료 |
| 보상 선택 UI가 복제된 서버 선택지만 표시 | 완료 |
| 새로고침 서버 요청 및 서버 선택지 재생성 | 완료 |
| 현재 서버 선택지에 포함된 `RowName`만 허용하는 검증 | 완료 |
| 멀티플레이 공유 경험치/레벨업 선택 정책 | 진행 필요 |

권장 흐름:

1. 서버가 보상 후보 3개를 생성하거나 seed를 생성
2. 솔로 플레이는 기존처럼 즉시 일시정지 후 owning client에 보상 UI 표시
3. 멀티플레이는 서버가 전체 게임을 일시정지하고 모든 플레이어에게 보상 UI 표시
4. 클라이언트는 선택 ID만 서버에 전송
5. 서버가 유효성 검증 후 보상 적용
6. 10초 타임아웃 시 서버가 미선택 플레이어의 보상을 자동 선택
7. 모든 선택 완료 후 서버가 게임을 재개하고 복제된 Attribute/Inventory 변경으로 UI 갱신

---

## 3. 권장 구현 순서

### Phase 1. 리슨 서버 PIE 기본 접속

1. `MyHackSlashEditor Win64 Development` 빌드 통과
2. PIE `Number of Players = 2`, `Net Mode = Play As Listen Server` 실행 확인
3. 호스트와 클라이언트에 각각 본인 HUD만 생성되는지 확인
4. 서버 권한 게임플레이와 로컬 UI/사운드 분기 확인
5. `MyHackSlashServer.Target.cs`는 추후 데디 서버 확장용으로 유지하되 현재 Phase의 필수 검증에서 제외

### Phase 2. ASC/Attribute 복제

1. `AHPlayerState` ASC 복제 설정
2. `UHCharacterAttributeSet` 주요 Attribute 복제 추가
3. HP/Gold/Exp/Hunger UI가 클라이언트에서 갱신되는지 확인
4. 데미지와 사망 판정을 서버 기준으로 고정

### Phase 3. 전투와 투사체 서버 권한화

1. 자동 스킬 발동을 서버에서만 실행
2. 히트 체크와 GE 적용을 서버에서만 실행
3. 투사체 서버 스폰 및 복제
4. GameplayCue로 피격 VFX/사운드/데미지 텍스트 표시

### Phase 4. 인벤토리/장착 복제

1. 젬 인벤토리 데이터를 `USTRUCT` 기반 복제 데이터로 전환
2. 장착/해제/합성은 서버 RPC로 요청
3. 장착 결과에 따라 서버가 Ability를 부여/제거
4. UI는 복제 상태를 읽기 전용으로 표시

구현 메모:

- `UHGemBase`는 `FGuid InstanceId`를 보유하고, 클라이언트 UI는 복제된 표시 데이터에서 같은 ID를 가진 임시 젬 객체를 재구성합니다.
- UI가 `EquipGem`, `EquipSupportGem`, `UnequipGem`, `UnequipSupportGem`을 호출하면 비권한 클라이언트에서는 서버 RPC로 우회합니다.
- 서버는 전달된 `InstanceId`를 서버 인벤토리/장착 슬롯에서 다시 찾아 실제 장착 상태와 GAS Ability를 변경합니다.
- 현재 RPC는 정상 UI 흐름 기준의 1차 전환이며, 합성/상점/보상 등 외부 진입점의 서버 검증은 추가 작업이 필요합니다.

### Phase 5. 웨이브/맵/보상 멀티플레이화

1. 웨이브 상태를 `GameState`로 이동
2. 몬스터 스폰은 서버만 수행
3. 설정창은 멀티플레이에서 일시정지하지 않도록 분리
4. 공유 경험치와 2배 필요 경험치 기준의 멀티플레이 레벨업 흐름 구현
5. 레벨업 능력 선택은 서버 전체 일시정지, 10초 제한, 미선택 자동 선택으로 처리
6. 웨이브 종료/상점/보상 UI는 플레이어별 owning client에 표시
7. 맵 타일 생성 기준을 다중 플레이어 기준으로 수정

---

## 4. 파일별 체크리스트

| 우선순위 | 파일 | 수정 내용 |
| :--- | :--- | :--- |
| P0 | PIE Play Settings | `Number of Players = 2`, `Play As Listen Server` 기본 검증 |
| P0 | `Mode/MyHackSlashPlayerController.cpp` | 로컬 컨트롤러에서만 HUD 생성 |
| P0 | `System/HUIManager.*` | 로컬 플레이어/PlayerController 기반 위젯 생성 |
| P0 | `System/HSoundManager.*` | 클라이언트 전용 사운드 재생 |
| P0 | `Unit/Player/HPlayerState.*` | ASC 복제, Gold 서버 권한화 |
| P0 | `Attribute/HCharacterAttributeSet.*` | Attribute 복제 및 `OnRep` 추가 |
| P0 | `Unit/HBaseCharacter.*` | 사망/공격 상태 서버 권한화 및 클라이언트 연출 분리 |
| P0 | `Skill/HEquipmentComponent.*` | 자동 발동 서버 제한, 장착 서버 RPC화 |
| P0 | `GAS/GA/HGA_AttackHitCheck.cpp` | 데미지 적용 서버 권한 체크 |
| P0 | `GAS/GA/HGA_ProjectileBase.cpp` | 투사체 서버 생성 |
| P0 | `Skill/HProjectile.*` | 복제 설정, 서버 폭발/데미지 |
| P1 | `System/HWaveManager.*` | 상태를 GameState로 이동 |
| P1 | `System/HMonsterSpawnManager.*` | 서버 전용 스폰, 다중 플레이어 스폰 기준 |
| P1 | `Item/HCoin.*` | 서버 전용 골드 지급, 복제/피드백 분리 |
| P1 | `Skill/HGemInventoryComponent.*` | 인벤토리 복제 데이터 구조로 전환 |
| P1 | `System/HSelectAbilityManager.*` | 플레이어별 보상 상태, 서버 검증 |
| P1 | `GameState` 또는 신규 레벨업 매니저 | 공유 경험치, 2배 필요 경험치, 10초 선택 타이머 |
| P2 | `Source/MyHackSlashServer.Target.cs` | 추후 데디 서버 빌드 타겟 유지/검증 |
| P2 | `System/HInfiniteMapManager.*` | 서버 기준 다중 플레이어 맵 갱신 |
| P2 | `System/HObjectPoolManager.*` | 복제 액터 풀링 정책 정리 |

---

## 5. 검증 시나리오

### 5.1 PIE 리슨 서버 실행

Editor Play 옵션:

| 옵션 | 값 |
| :--- | :--- |
| Number of Players | 2 이상 |
| Net Mode | Play As Listen Server |
| Run Under One Process | 초기에는 On, 문제 분석 시 Off |

확인:

- 호스트와 클라이언트가 같은 세션에 접속되는지
- 호스트가 시작 버튼을 눌렀을 때 `ServerTravel(Map?listen)`로 모든 클라이언트가 같은 인게임 맵에 진입하는지
- 호스트와 클라이언트 모두 본인 HUD만 가지는지
- 호스트가 서버 권한과 로컬 UI를 동시에 처리해도 중복 UI/사운드가 없는지
- 웨이브가 서버에서 시작되는지

### 5.2 PIE 클라이언트 2명 접속

필요하면 Play 옵션에서 별도 프로세스 실행 또는 `Play As Client` 모드로 서버/클라이언트 분리 흐름을 확인합니다.

확인:

- 각 클라이언트에 본인 HUD만 생성되는지
- HP/Gold/Exp/Hunger가 각자 올바르게 표시되는지
- 한 클라이언트의 설정창이 다른 플레이어의 게임을 멈추지 않는지
- 공유 경험치가 솔로 기준 2배 요구량에 도달했을 때 모든 클라이언트에 선택창이 뜨는지
- 능력 선택 중에는 서버가 전체 게임을 멈추고, 10초 후 미선택 플레이어가 자동 선택되는지

### 5.3 전투 검증

확인:

- 클라이언트가 공격해도 서버에서만 데미지가 적용되는지
- 자동 슬롯 스킬이 중복 발동하지 않는지
- 투사체가 모든 클라이언트에 동일하게 보이는지
- 몬스터 사망과 코인 드랍이 중복되지 않는지

### 5.4 보상/상점 검증

확인:

- 골드 지급/소비가 서버 검증을 거치는지
- 보상 선택 UI가 해당 플레이어에게만 뜨는지
- 멀티플레이 레벨업 보상 선택 UI는 모든 플레이어에게 동시에 뜨는지
- 클라이언트가 임의로 보상 함수를 호출해도 서버 상태가 변하지 않는지

---

## 6. 결론

현재 구조에서 리슨 서버 멀티플레이를 구현하려면 가장 먼저 PIE Listen Server 기본 접속, ASC/Attribute 복제, 서버 권한 전투 판정, 젬 인벤토리/장착 상태 복제를 처리해야 합니다. 그 다음 웨이브, 스폰, 보상, UI를 서버 상태와 로컬 클라이언트 표시로 분리하는 순서가 안전합니다.

데디케이티드 서버 빌드 타겟은 추후 운영/패키징 단계의 확장 목표입니다. 리슨 서버 단계에서는 `MyHackSlashServer.Target.cs` 빌드 성공보다 PIE 멀티플레이에서 권한과 복제 흐름이 올바른지가 더 중요합니다.

특히 `UWorldSubsystem`과 `UGameInstanceSubsystem`은 상태를 편하게 보관하기 좋지만, 네트워크 상태 복제의 주체가 아닙니다. 웨이브처럼 모든 클라이언트가 알아야 하는 상태는 `GameState`, 플레이어별 상태는 `PlayerState`, 클라이언트 개인 설정은 `GameInstanceSubsystem` 또는 `SaveGame`으로 나누는 것이 적합합니다.
