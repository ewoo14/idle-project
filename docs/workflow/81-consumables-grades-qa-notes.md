# PR #81 소비 아이템 등급별 차등 — QA 노트 (Given/When/Then)

> PM/Claude 직접 작성(Codex 한도 대체). 테스트는 character/designer 파트에 포함, 본 노트는 커버리지 종합.

## 시나리오
### S1. 등급별 효과 % (Lesser 0.5 / Standard 1 / Greater 2)
- Given 타입 T, When `GetBuffPercent(T, grade)`, Then Lesser=Standard×0.5, Greater=Standard×2.0, 지속은 등급 무관 고정. 클라↔서버 parity. (커버: `ConsumableTests` 등급 차등 + `consumable.parity.test.ts`)

### S2. 보유/사용 (등급 키)
- Given (T, grade) 보유, When `TryUseConsumable(T, grade)`, Then 해당 등급 카운트 -1, 타입 T 버프 활성=그 등급 %·지속. (커버: `Consumable.GradeBuffService`)

### S3. 스택 (최신 등급 갱신)
- Given T 버프가 Lesser 활성 중, When Greater 사용, Then 버프가 Greater %·종료시각으로 갱신(최신 등급). (커버: `ConsumableTests` 스택)

### S4. 세이브 v15→v16
- Given v15 세이브(소비 엔트리에 grade 없음), When 로드, Then 모든 소비=Standard(1)로 마이그레이션. v16 라운드트립 grade 보존, 활성 버프 등급 복원. (커버: `SaveSystem.ConsumableGradeV15Migration` + 라운드트립)

### S5. 수급
- Given 골드 충분, When `TryBuyConsumable(T, grade)`, Then 등급별 가격(Lesser×0.6/Standard/Greater×2.5) 차감 후 (T,grade) +1. (커버: `ConsumableTests` 상점)

### S6. UI
- Given 소비 패널, When 열기, Then 타입×등급 보유 행 표시, 사용 hitbox (type,grade), 활성 버프 등급 표기. (커버: `UI.HUD.ConsumablePanelViewModel` 등급-aware + `CsvIntegrity`)

### S7. 회귀
- 기존 Standard 소비 동작·기존 소비 패널·세이브 불변. 기존 2-인자 GetBuffPercent/GetCount/TryUseConsumable 호환 오버로드. (커버: 기존 Consumable Automation 유지)

## 검증 명령
- 서버: `cd server; npm run lint && npm run test -- consumable save` (53)
- 클라: 표준 jumbo(unity) 빌드(`-DisableUnity` 없이) + `Automation RunTests IdleProject.Consumable+IdleProject.GameCore.SaveSystem+IdleProject.UI.HUD.ConsumablePanelViewModel`

## 비고
- character가 `-DisableAdaptiveUnity` 풀 유니티 빌드까지 통과(ODR 없음). SaveVersion 15→16.
- balance/qa 노트는 PM/Claude 직접(Codex 한도). 구현/테스트 코드는 character/designer(claude) 서브에이전트.
