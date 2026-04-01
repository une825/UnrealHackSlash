# 🗺️ Infinite Map System Design

본 문서는 **MyHackSlash** 프로젝트의 무한 맵 자동 생성 및 환경 프롭 배치 시스템의 설계를 정의합니다.

---

## 1. 개요 (Overview)

본 시스템은 플레이어의 위치를 실시간으로 추적하여 발밑에 지형 타일을 동적으로 배치하고, 멀어진 타일을 회수함으로써 성능 효율을 유지하면서도 끝없이 이어지는 지형 경험을 제공합니다.

---

## 2. 핵심 컴포넌트 (Core Components)

### 2.1 AHMapTile (타일 액터)
*   **역할**: 무한 맵을 구성하는 최소 단위 지형 조각.
*   **주요 기능**:
    *   `PrepareFromPool()`: 풀에서 활성화될 때 그리드 좌표에 맞춰 월드 위치를 재설정.
    *   `GenerateProps()`: 타일 위에 돌, 풀 등 환경 요소를 랜덤 배치.
    *   `ClearProps()`: 타일 반납 시 생성된 프롭들도 함께 오브젝트 풀로 반납.

### 2.2 UHInfiniteMapManager (World Subsystem)
*   **역할**: 타일의 생성, 배치, 회수 로직을 총괄하는 중앙 제어 장치.
*   **작동 방식**:
    1.  플레이어의 월드 좌표를 타일 크기로 나누어 **그리드 좌표(GridPos)** 계산.
    2.  플레이어 주변 `ViewDistance` 내의 그리드 중 타일이 없는 곳에 새 타일 배치.
    3.  범위를 벗어난 타일은 `ActiveTiles` 맵에서 제거하고 오브젝트 풀로 반납.

### 2.3 UHMapConfigDataAsset (데이터 에셋)
*   **역할**: 맵의 테마와 설정을 통합 관리.
*   **설정 항목**:
    *   타일 클래스 및 크기 (`MapTileSize`)
    *   가시 거리 (`MapViewDistance`)
    *   프롭 목록 및 가중치 (`MapPropPool`, `SpawnWeight`)
    *   타일당 프롭 밀도 (`Min/MaxPropCount`)

---

## 3. 환경 프롭 배치 로직 (Environment Decoration)

### 3.1 결정론적 랜덤 (Deterministic Random)
*   플레이어가 동일한 좌표로 돌아왔을 때 항상 같은 위치에 프롭이 있도록, **그리드 좌표를 시드(Seed)**로 하는 랜덤 스트림을 사용합니다.
    *   `Seed = (GridX * HashA) ^ (GridY * HashB)`

### 3.2 가중치 기반 선택 (Weighted Selection)
*   `FMapPropData`의 `SpawnWeight`를 기반으로 특정 프롭(예: 풀)이 더 자주 나오거나 특정 프롭(예: 돌)이 드물게 나오도록 제어합니다.

### 3.3 장애물 및 콜리전
*   **돌(Rock)**: `BlockAll` 설정을 통해 플레이어 및 AI의 이동을 차단하는 장애물로 작동.
*   **풀(Grass)**: `NoCollision` 설정을 통해 시각적 장식으로만 작동하며 이동에 방해를 주지 않음.

---

## 4. 네비게이션 및 AI (Navigation)

### 4.1 실시간 네비메쉬 (Runtime NavMesh)
*   동적으로 생성되는 타일 위에서 AI가 이동할 수 있도록 `Runtime Generation`을 **Dynamic**으로 설정.

### 4.2 네비게이션 인보커 (Navigation Invoker)
*   월드 전체가 아닌 플레이어 주변만 실시간으로 네비메쉬를 굽도록 `UNavigationInvokerComponent`를 플레이어 캐릭터에 부착하여 최적화.

---

## 5. 성능 최적화 (Performance Optimization)

*   **오브젝트 풀링**: 모든 타일과 프롭은 `UHObjectPoolManager`를 통해 재사용되어 런타임 생성/삭제 비용을 최소화합니다.
*   **가시성 제어**: 플레이어와 멀어진 타일은 즉시 비활성화되어 렌더링 및 물리 계산 부하를 줄입니다.
