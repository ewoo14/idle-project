# 리더보드 클라이언트 연동 + 랭킹 UI 구현 계획

> **For agentic workers:** v3 워크플로우(각 Task→Codex 파트). 문서/주석/커밋 본문 **한글**, 식별자 영문.

**Goal:** 이미 가동 중인 서버 leaderboard(Power/Rebirth)를 클라이언트에 연동·표시하고 "내 순위"를 추가한다.

**Architecture:** 서버는 세이브 업로드(#54) 시 점수 기록·top-N 조회까지 완비 → 클라 조회/표시만 추가 + backend "내 순위" 엔드포인트 신설. 세이브 변경 없음.

**설계 출처:** [`docs/superpowers/specs/2026-05-29-leaderboard-client-design.md`](../specs/2026-05-29-leaderboard-client-design.md)

---

## Task 1: backend — "내 순위" 엔드포인트 (backend)
**Files:** Modify `server/src/modules/leaderboard/leaderboard.repo.ts`, `leaderboard.service.ts`, `leaderboard.routes.ts`, `leaderboard.schema.ts`, `leaderboard.test.ts`.
- [ ] repo: `getPowerRank(seasonId, characterId): Promise<{rank:number, score:bigint}|null>` (PG `rank() over (order by power_score desc)` 서브쿼리, 미등록 null). `getRebirthRank` 동형.
- [ ] service: `getMyRank(kind:'power'|'rebirth', seasonId, characterId)` → repo 위임(미등록 rank 0).
- [ ] routes: `GET /power/me`·`/rebirth/me` (querystring season+characterId(uuid)). schema 추가.
- [ ] 실패 테스트: 등록 캐릭터 rank 정확, 미등록 0/없음, 동점 처리, 시즌 분리.
- [ ] `cd server; npm run lint && npm run test -- leaderboard && npm run build` GREEN(**lint 필수**).
- [ ] 커밋 `feat: 리더보드 내 순위 엔드포인트 (PR #76)`.

## Task 2: client — ApiClient + LeaderboardService + Automation (character)
**Files:** Modify `client/Source/IdleProject/NetworkClient/ApiClient.h/.cpp`; Create `GameCore/LeaderboardService.h/.cpp`, `GameCore/LeaderboardTypes.h`(`FLeaderboardEntry{FString CharacterId; int64 Score; int32 Rank;}`, `ELeaderboardKind{Power,Rebirth}`); Modify `GameCore/IdleGameInstance.h/.cpp`; Create `Tests/LeaderboardTests.cpp`.
- [ ] ApiClient(기존 async 콜백 패턴): `FetchLeaderboard(ELeaderboardKind, int32 Season, TFunction<void(bool,FString)>)`(GET /leaderboard/power|rebirth?season=&limit=100), `FetchMyRank(ELeaderboardKind, int32 Season, const FString& CharacterId, TFunction<void(bool,FString)>)`(/me).
- [ ] `ULeaderboardService`: JSON 응답 → `TArray<FLeaderboardEntry>` + `FLeaderboardEntry MyEntry`(kind별 보관). `ParsePowerJson`/`ParseRebirthJson`/`ParseMyRankJson`(BlueprintCallable 또는 내부) — **순수 파싱은 Automation으로 서버 무의존 검증**.
- [ ] GameInstance: `GetLeaderboardService()`, `RefreshLeaderboard(ELeaderboardKind)`(시즌 id=SeasonService 현재, ApiClient 호출→파싱→보관, graceful 오프라인 빈 유지). 캐시된 CharacterId 사용(EnsureCharacter).
- [ ] Automation: 샘플 JSON 파싱→엔트리 배열/순위/점수 정확, 내 순위 파싱, 빈/오류 응답 graceful.
- [ ] 커밋 `feat: 리더보드 클라 연동 + 서비스 (PR #76)`.

## Task 3: UI — 리더보드 패널 (designer)
**Files:** 리더보드 패널 위젯/HUD + ko/en CSV.
- [ ] Power/Rebirth 탭, top-N 리스트(순위·점수), 내 순위 별도 하이라이트, 시즌 라벨, 로딩/오프라인 상태. 열기 시 `RefreshLeaderboard`.
- [ ] ko/en 키(제목/탭/순위/점수/내순위/오프라인) + CsvIntegrity.
- [ ] 커밋 `feat: 리더보드 랭킹 UI + ko/en (PR #76)`.

## Task 4: 밸런스/문서 (balance)
**Files:** Create `docs/planning/leaderboard-v1-note.md`.
- [ ] 랭킹 모델(Power=computePowerScore/CP 파생, Rebirth=count), 시즌 분리, 점수 무변경(공식 영향 없음) 명시. 시즌 보상 후속 설계 메모.
- [ ] 커밋 `docs: 리더보드 V1 노트 (PR #76)`.

## Task 5: QA (qa)
**Files:** `Tests/LeaderboardTests.cpp` 보강 + `leaderboard.test.ts` 보강 + qa 노트.
- [ ] e2e 흐름(업로드→기록→조회→표시 모델), 내 순위 정확/미등록, graceful 오프라인, 시즌 경계. server 내 순위 parity/경계.
- [ ] 커밋 `test: 리더보드 E2E/내순위 (PR #76)`.

---

## Self-Review
- 스펙 §3 → Task 1(backend 내순위)·2(클라)·3(UI)·4(문서)·5(qa).
- 세이브 변경 없음(SaveVersion 14 유지). 기존 leaderboard 제출·top-N 불변(추가만).
- 네트워크 graceful(#54 패턴), Automation은 JSON 파싱 단위(서버 무의존).
- Placeholder 없음. `FLeaderboardEntry`/`ELeaderboardKind`/`getMyRank`/`FetchLeaderboard` 명칭 일관.

## 워크플로우 v3 매핑
1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
