# PR #82 기획서 — 마스터리 트랙별 로컬 보너스 V2 (트랙당 2종)

> **PM 자율 진행 (Claude 에이전트 구현팀, Codex 6/1 복구까지)**. #74 마스터리 로컬 보너스(트랙당 1종) 후속 — 각 트랙에 **2종째 로컬 보너스**를 추가(1종과 겹치지 않는 경제/QoL 효과). **세이브 변경 없음**(트랙 레벨에서 파생). 무한 성장([[project-infinite-growth]]). client + server 5-team.

## 1. 목표 / DoD
#74로 각 트랙이 1종 로컬 보너스를 가짐. V2는 2종째를 더해 마스터리 심화를 완성.

### DoD
1. **중앙화**: `FMasteryFormula::GetLocalBonus2(track, level)` ↔ 서버 `localBonus2` 미러 + `UMasteryService::GetLocalBonus2(track)`. 곡선은 1종과 동일 철학(`k·ln(1+level)`), RNG/성공률 회피.
2. **2종 효과(1종과 비중복, 트랙별 단일 적용)**:
   - 전투: 1종 처치 EXP +% → **2종 처치 골드 +%**
   - 장비: 1종 강화 골드비용 절감 → **2종 잠재 큐브(재설정/등급) 골드 가격 절감 %**(클램프)
   - 심연: 1종 던전 보상 +% → **2종 던전 일일 입장 +N**(레벨 임계마다 +1, 상한 예 +3)
   - 룬: 1종 코어 가산 → **2종 룬 에센스 획득 +%**(분해/획득 경로)
   - 야성: 1종 펫 보너스 +% → **2종 펫 먹이 골드 비용 절감 %**(클램프)
   - 탐험: 1종 퀘스트 보상 +% → **2종 오프라인 보상 +%**
3. **단일 적용**(#72/#74 이중 금지 교훈), 전역 월드파워(#72)·1종(#74)과 다른 효과·지점.
4. **세이브 변경 없음**(SaveVersion 16 유지).
5. **UI**: 숙련 패널 트랙별 2종 보너스도 표시. ko/en CsvIntegrity.
6. **테스트**: 클라 Automation(2종 단조·클램프·던전입장 임계·각 적용·회귀) + 서버 vitest + parity. CI GREEN + **표준 jumbo 빌드 PM 검증**.

## 2. 작업 분배 (Claude 서브에이전트, claude(<role>):)
| 파트 | 작업 |
| --- | --- |
| character (메인) | GetLocalBonus2(track,level)+서버 미러+UMasteryService::GetLocalBonus2, 6 소비 지점 적용(처치골드/큐브가격/던전입장+N/룬에센스/펫먹이비용/오프라인), 클라 Automation |
| backend | mastery.ts localBonus2 미러 + vitest parity |
| designer | 숙련 패널 2종 보너스 표시 + ko/en |
| balance | 6 계수·던전입장 임계·클램프 + 무한 비폭주 |
| qa | 2종 단조/클램프/던전입장/각 적용/회귀 + parity |

## 3. 범위 외
트랙당 3종+, 강화 성공률 보정(RNG parity 회피 유지).

## 4. 워크플로우 v3
Claude 서브에이전트 구현 → PM 리뷰/통합. [5] **표준 jumbo(unity) 빌드 PM 직접 검증**. 머지 전 CI 그린.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 던전 입장 +N 정수 경계 | 임계/상한 명확 + Automation 경계 |
| 1종/전역과 중복감 | 2종은 1종과 다른 효과·지점, 단일 적용 |
| 무한 폭주 | 로그 곡선 + 클램프(가격절감/입장 상한) + balance |
| 서버↔클라 drift | localBonus2 fround 미러 + parity |
| jumbo ODR | 신규 익명 헬퍼 동명 grep + PM 표준 jumbo 빌드 |
