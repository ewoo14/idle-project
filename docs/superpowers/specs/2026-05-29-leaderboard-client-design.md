# 리더보드 클라이언트 연동 + 랭킹 UI — 설계 문서

> 작성일: 2026-05-29 · 대상: PR #76 · 브랜치: `plan/76-leaderboard-client`
> 분류: 신규 소셜/경쟁 벡터 (휴면 서버 모듈 활성화) · PM 자율 진행

---

## 0. 한 줄 요약

서버 `leaderboard` 모듈은 이미 완비(세이브 업로드 시 전투력/환생 점수 기록 + PG/Redis
+ `/power`·`/rebirth` 조회)됐으나 **클라이언트가 전혀 표시하지 않는다**. 클라이언트
연동 + 랭킹 UI + "내 순위"를 추가해 게임 **최초의 소셜/경쟁 기능**을 연다.

---

## 1. 목적 / 배경

- 게임에 경쟁/사회적 동기 부여가 전무 — 방치형 장기 리텐션의 큰 공백.
- **백엔드는 이미 가동 중**: `save.service.ts`가 세이브 업로드(#54)마다
  `leaderboard.updatePower(computePowerScore)` + `updateRebirth(rebirthCount)`를 시즌별
  기록. `GET /leaderboard/power|rebirth?season=&limit=`로 top-N 조회.
- **결손**: 클라이언트에 조회/표시 경로 0 (grep 결과 client 측 leaderboard 참조 없음).
- 추가 필요: 현재 top-N만 조회 가능 → **"내 순위"** 조회(특정 캐릭터 rank) 신설.

---

## 2. 핵심 결정 (PM 자율)

| 항목 | 결정 | 근거 |
| --- | --- | --- |
| 점수 소스 | 기존 그대로(세이브 업로드 시 서버 기록) | 백엔드 이미 가동, 신규 제출 경로 불필요 |
| 조회 | top-N(기존) + **내 순위 신설** | "내가 몇 등?" 핵심 UX |
| 랭킹 종류 | 전투력(Power) + 환생(Rebirth) 2종 | 기존 엔드포인트 활용 |
| 시즌 | 현재 시즌 id(SeasonService #22) 사용 | 기존 시즌 체계 재사용 |
| 보상 | **범위 외**(V1은 조회·표시만) | 시즌 보상은 후속(persistence·claim 복잡) |
| 세이브 | **변경 없음**(SaveVersion 14 유지) | 읽기 전용 표시 |

---

## 3. 통합 지점 (구현 개요)

| 파트 | 작업 |
| --- | --- |
| backend | **"내 순위" 신설**: repo `getPowerRank(season, characterId)`/`getRebirthRank`(PG rank window 쿼리, 미등록 시 0/없음), service `getMyRank`, route `GET /leaderboard/power/me`·`/rebirth/me`(season+characterId). 기존 조회·제출은 불변. vitest. |
| character (메인) | ApiClient `FetchLeaderboardPower(season, cb)`/`FetchLeaderboardRebirth(season, cb)`/`FetchMyLeaderboardRank(season, characterId, kind, cb)`(기존 async 콜백 패턴). `ULeaderboardService`(또는 데이터 홀더) — JSON → `FLeaderboardEntry{CharacterId, Score:int64, Rank:int32}` 배열 + 내 순위 파싱/보관. GameInstance 접근자 + 시즌 id 소스. graceful 오프라인(서버 불가 시 빈/캐시). Automation(파싱/모델). **세이브 변경 없음** |
| designer | 리더보드 패널 — Power/Rebirth 탭, top-N 리스트(순위·점수), **내 순위 하이라이트/별도 표시**, 시즌 라벨, 로딩/오프라인 상태. ko/en + CsvIntegrity |
| balance | 시즌/랭킹 모델 문서(전투력=CP 파생·환생=count, balance 공식 무변경). 시즌 보상 후속 명시 |
| qa | e2e(세이브 업로드→서버 기록→클라 조회→표시), 내 순위 정확성(등록/미등록), graceful 오프라인, season 경계 + 내 순위 parity |

---

## 4. 스코프

**In Scope (V1):** Power/Rebirth 리더보드 조회(top-N) + 내 순위(신설), 랭킹 UI(탭/리스트/내순위/시즌), ko/en, backend 내 순위 엔드포인트, qa/parity.

**Out of Scope (후속):** 시즌 경쟁 보상(rank-tier claim), 길드/친구, 실시간 갱신/푸시, 페이지네이션(top-100 초과), 점수 위변조 강화 검증(서버 권위 점수는 기존 computePowerScore 의존).

---

## 5. 리스크

| 리스크 | 완화 |
| --- | --- |
| 라이브 서버 의존(클라 테스트) | Automation은 JSON 파싱/랭킹 모델 단위 검증(서버 무의존), 네트워크는 graceful 폴백(#54 패턴) |
| 내 순위 쿼리 정확성 | PG `rank()` window + 등록/미등록/동점 경계 vitest |
| 시즌 id 불일치(클라↔서버) | SeasonService 현재 시즌 id 사용, 쿼리 파라미터 일관 |
| 기존 leaderboard 회귀 | 제출·top-N 조회 불변(추가만), 기존 테스트 유지 |
| 점수 신뢰(치트) | V1은 기존 서버 기록(computePowerScore) 의존, 강화 검증은 후속 |

---

## 6. 워크플로우 v3 / 후속

[1] PM 기획+PR → [2] Codex 5-team → [3] Claude TM → [4] Codex(결함 시) → [5] 검증 → [N] CI 그린 + PM 종합 + 머지. PM 자율.

**후속:** 시즌 경쟁 보상, 길드/친구, 실시간 갱신, 페이지네이션, 점수 검증 강화.
