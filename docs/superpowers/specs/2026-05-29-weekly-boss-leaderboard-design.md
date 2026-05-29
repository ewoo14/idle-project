# 주간 보스 데미지 리더보드 — 설계 문서 (선행 기획, PR #79 예정)

> 작성일: 2026-05-29 · 대상: PR #79(Codex 복구 6/1 후 디스패치) · PM 자율
> 상태: **구현 보류(Codex 한도)**, 기획만 선행. 디스패치 시 현행 코드 재검증.

## 0. 요약
#77 주간 보스의 **주간 누적 데미지**를 #76 리더보드에 **주간 단위 랭킹**으로 추가한다. 경쟁 동기 + 기존 두 시스템 연결.

## 1. 배경
- #76 리더보드: Power/Rebirth 2종(시즌 정수 키). 서버 가동, 클라 표시(#76).
- #77 주간 보스: 주간 누적 데미지(ISO week 리셋). 현재 경쟁 요소 없음.
- 결합: 주간 데미지를 **ISO week 키 리더보드**로 → "이번 주 보스 데미지 순위".

## 2. 핵심 결정
| 항목 | 결정 |
| --- | --- |
| 랭킹 키 | **ISO week 문자열**(예 `2026-W22`) — 주간 자동 분리/리셋 |
| 제출 시점 | 세이브 업로드(#54) 시 `save.service`가 payload의 weeklyBossDamage+weekId 기록(주간 보스 챌린지 후 autosave→업로드 경유) |
| 조회 | top-N + 내 순위(#76 패턴 재사용) |
| UI | #76 리더보드 패널에 "주간 보스" 탭 추가(현재 주) |

## 3. 통합 지점
| 파트 | 작업 |
| --- | --- |
| backend | `leaderboard_weekly_damage`(week_id text, character_id, damage bigint) 테이블(마이그레이션) + repo `upsertWeeklyDamage`/`listWeeklyDamage(weekId, limit)`/`getWeeklyDamageRank(weekId, characterId)` + service `updateWeeklyDamage`/`getWeekly`/`getMyWeeklyRank` + 라우트 `GET /leaderboard/weekly?week=` ·`/weekly/me?week=&characterId=` + save.service 업로드 훅(payload weeklyBossDamage/weeklyBossWeekId 기록) + vitest |
| character | ApiClient `FetchWeeklyDamageLeaderboard(week)`/`FetchMyWeeklyRank(week, characterId)`, ULeaderboardService 주간 종류 추가(ELeaderboardKind에 WeeklyDamage), GameInstance RefreshLeaderboard(WeeklyDamage)=GetCurrentUtcWeekString 사용. Automation(파싱) |
| designer | 리더보드 패널 "주간 보스" 탭(현재 주 라벨·top-N·내 순위) + ko/en |
| balance | 주간 데미지 랭킹 모델 문서(데미지=CP×도전, 공식 무변경) |
| qa | 제출→조회→표시, 내 주간 순위, 주 경계(다음 주 빈 보드), parity |

- 세이브 변경 없음(주간 보스 상태는 #77에 이미 저장; payload는 #54 clientSave에 포함). 서버 payload는 weeklyBossDamage/weeklyBossWeekId 이미 #77에서 추가됨 → save.service가 그 값을 leaderboard에 기록.

## 4. 스코프
**In:** 주간 데미지 리더보드(제출/조회/내순위) + 패널 탭 + 마이그레이션 + parity. **Out:** 역대 주간 보관/명예의 전당, 주간 보상(랭크 보상은 #78 패턴 후속).

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 주 키 정합(클라↔서버) | ISO week 문자열 동일 소스(GetCurrentUtcWeekString ↔ payload weekId) |
| 기존 리더보드 회귀 | 신규 종류 추가만, Power/Rebirth 불변 |
| 마이그레이션 | 신규 테이블만(기존 무변경) |
| jumbo(unity) ODR | 신규 헬퍼 동명 grep + 표준 jumbo 빌드 PM 검증 |

## 6. 워크플로우 v3 / 후속
표준 v3. 후속: 주간 랭크 보상(#78 연계), 역대 주간 명예의 전당.
