# PR #82 마스터리 로컬 보너스 V2 — QA 노트 (Given/When/Then)

> PM/Claude 직접 작성(Codex 한도 대체). 테스트는 character/designer 파트에 포함.

## 시나리오
### S1. 2종 공식 (단조/클램프/정수)
- `GetLocalBonus2(track, level)`: 0레벨=0, 양수 단조 증가, Equipment/Beast 0.5 클램프. `GetAbyssBonusEntries`: floor(Lv/50), 상한 +3. 클라↔서버 parity. (커버: `Mastery.LocalBonus2Application` + `mastery.parity.test.ts`)

### S2. 2종 적용 (각 단일, 1종/전역 비중복)
- 전투 Lv↑ → 처치 골드↑ / 장비 Lv↑ → 큐브 가격↓ / 심연 Lv 50/100/150↑ → 던전 입장 +1/+2/+3 / 룬 Lv↑ → 분해 에센스↑ / 야성 Lv↑ → 펫 먹이비용↓ / 탐험 Lv↑ → 오프라인 보상↑. (커버: `Mastery.LocalBonus2Application`, `Dungeon.ServiceAbyssBonusEntries`, 영향 스위트 PetFeed/Offline)

### S3. 회귀
- 0레벨 시 기존 산출 불변(처치골드/큐브가격/던전입장/에센스/먹이비용/오프라인). 1종(#74)·전역(#72) 불변. 기존 DungeonService 시그니처 호환(BonusEntries 기본 0). (커버: 기존 Dungeon/Pet/Offline Automation)

### S4. SaveVersion 단언 정합
- SaveVersion 16. **stale 테스트 복구**: Dungeon.SaveMigration/Mastery.GameInstanceHooks가 16 기대로 정정(#81 bump 잔여). (커버: 해당 두 테스트 Success)

## 검증 명령
- 서버: `cd server; npm run lint && npm run test -- mastery` (40)
- 클라: 표준 jumbo 빌드 + `Automation RunTests IdleProject.Mastery+IdleProject.GameCore.Dungeon+IdleProject.GameCore.SaveSystem+IdleProject.UI.HUD+IdleProject.Localization.CsvIntegrity` (+Pet/Offline 영향 스위트)

## 비고
- **세이브 변경 없음**(SaveVer 16). character가 표준 jumbo 빌드 ODR 확인.
- **교훈(메모리화 대상): SaveVersion bump 시 버전 단언 스위트(Dungeon.SaveMigration/Mastery.GameInstanceHooks 등) 전수 재검증 — server-ci UE 미검증이라 PM [5]가 광범위 Automation 필수.**
