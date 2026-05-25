# M2 인벤토리 V1 QA 시나리오

## 범위

PR #9 인벤토리 + 장비 V1의 드롭, 자동 흡수, 자동 장착, 장비 보너스, HUD 요약 표시를 검증한다.

## Given / When / Then

### 1. PIE 진입 시 인벤토리 빈 상태

- Given 새 PIE 세션을 시작한다.
- When 플레이어 Pawn의 `UInventoryComponent`를 조회한다.
- Then `Items.Num == 0`이고 모든 장착 슬롯은 비어 있다.

### 2. 슬라임 사망 5번 반복 시 드롭 확률 관찰

- Given 슬라임 5마리를 순차 처치할 수 있는 자동 전투 상태다.
- When 슬라임 사망 이벤트를 5번 반복한다.
- Then 장비 드롭 spawn은 평균 0~1개 범위로 관찰된다.

### 3. 드롭된 장비 자동 흡수

- Given 장비 드롭 액터가 플레이어와 오버랩 가능한 위치에 있다.
- When 플레이어가 드롭 액터와 오버랩한다.
- Then 장비가 인벤토리에 추가되고 드롭 액터는 제거된다.

### 4. Common 무기에서 Uncommon 무기 자동 교체

- Given Common 무기가 장착되어 있다.
- When 같은 슬롯의 Uncommon 무기가 더 높은 PowerScore로 드롭된다.
- Then 신규 무기가 자동 장착되고 HUD 좌하단 무기 줄이 갱신된다.

### 5. Weapon + Helmet 동시 장착 보너스 합산

- Given Weapon과 Helmet을 각각 장착한다.
- When `ComputeEquipmentBonus`를 호출한다.
- Then 두 장비의 ATK/DEF/HP 보너스 합계가 반환된다.

### 6. 강화 +1 PowerScore 증가

- Given 같은 기본 스탯의 장비 두 개가 있고 하나만 `EnhanceLevel = 1`이다.
- When `FItemPowerScore::Compute`를 각각 호출한다.
- Then 강화 +1 장비의 PowerScore가 10% 높게 계산된다.

### 7. 같은 등급이지만 낮은 PowerScore는 자동 장착 안 됨

- Given Rare 무기가 이미 장착되어 있다.
- When 같은 Rare 등급이지만 더 낮은 PowerScore의 무기가 드롭된다.
- Then 기존 장비가 유지되고 `OnEquippedChanged`는 발생하지 않는다.

### 8. 최대 100칸 도달 시 폐기 정책

- Given 인벤토리가 100칸까지 찬 상태다.
- When 추가 장비가 드롭된다.
- Then 현재 PR 범위에서는 추가 장비를 받지 않는다.
- And 가장 약한 아이템부터 자동 폐기하는 정책은 후속 PR에서 별도 구현/검증한다.

## 자동화 분류 매트릭스

| 시나리오 | 1차 도구 | 보조 확인 | 비고 |
| --- | --- | --- | --- |
| 빈 인벤토리 | UE Automation | PIE 스모크 | 컴포넌트 초기 상태 |
| 5% 드롭 | UE Automation | 수동 확률 샘플 | 난수는 허용 범위 검증 |
| 자동 흡수 | UE Automation | PIE 스모크 | 오버랩 이벤트 |
| 자동 장착 + HUD | UE Automation | 수동 HUD 확인 | Slate 텍스트 갱신 포함 |
| 보너스 합산 | UE Automation | 서버 공식 비교 | 클라이언트 authoritative |
| 강화 PowerScore | UE Automation + vitest | 코드 리뷰 | 서버 `equipment.ts` 미러 |
| 낮은 점수 유지 | UE Automation | 로그 확인 | 자동 교체 방지 |
| 100칸 폐기 정책 | 후속 PR | 수동 | 현재 범위 외 |
