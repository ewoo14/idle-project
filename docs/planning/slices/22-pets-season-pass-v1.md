# PR #22 기획서 — 펫 2종 + 시즌 패스 베타 V1 (M5 PR #11)

> M5 마지막. **펫 2종(강아지 골드+, 새 드롭+)** + **시즌 패스 무료 트랙 베타**(티어/시즌토큰/보상 수령). GDD §6.2(펫), §6.4(시즌 패스). 퀘스트(PR #18)·골드·오프라인 위에 구축.

## 1. 목표 / DoD
펫을 장착하면 패시브 보너스(골드/드롭)가 적용되고, 일일/메인 퀘스트 완료로 시즌 토큰을 얻어 시즌 패스 무료 트랙 티어 보상을 수령한다.

### DoD 검증
1. 펫 2종(강아지=골드+%, 새=드롭률+%) 보유/장착, 장착 시 해당 보너스가 골드 획득/드롭에 반영.
2. 시즌 패스 무료 트랙: 티어(예 10) + 티어별 보상, 시즌 토큰 누적(퀘스트 완료 시 +N) → 티어 도달 → 보상 수령.
3. 펫/시즌 UI: 펫 목록/장착, 시즌 패스 진행 바/티어/수령.
4. 서버 Vitest(펫 보너스/시즌 진행/수령/토큰) + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 서버 — 펫 + 시즌 모듈 (메인)
- `core/data/pets.ts`: 펫 2종(petId, name, bonusType(gold|drop), bonusPercent). 읽기 전용.
- `core/data/season.ts`: 시즌 패스 티어 정의(무료 트랙 — tier, requiredTokens, rewardType, rewardAmount).
- `modules/pet/`: 펫 보유/장착 상태(equip 1마리 V1), 보너스 조회. (offline 모듈 패턴)
- `modules/season/`: 시즌 토큰 잔량, 티어 진행, 보상 수령(claim), 시즌 토큰 적립(퀘스트 완료 훅 — quest 모듈 연동 또는 클라 보고). migration(pet_state, season_state).
- Vitest: 펫 보너스 계산, 시즌 티어 도달/수령/중복 방지, 토큰 누적.

### 2.2 클라이언트 — 펫 보너스 + UI (C++)
- 펫 보너스 적용: 장착 펫의 골드+/드롭+ 가 골드 획득(IdleMonster GoldDrop/GameInstance)·드롭률에 반영. C++ 미러(서버 데이터와 동일 %).
- 시즌 토큰: 퀘스트 완료(QuestService claim) 시 시즌 토큰 +N 적립(클라), 티어 진행/수령.
- 펫/시즌 UI(IdleHUD/위젯): 펫 장착, 시즌 패스 티어/진행/수령. 단축키.
- Automation: 펫 보너스 계산, 시즌 진행/수령 순수 로직.

#### Character C++ mirror
- `UPetService`: V1 기본 보유 펫은 `dog`(gold +20%) / `bird`(drop +15%)이며 1마리만 장착한다.
- `USeasonService`: 시즌 1 무료 트랙 10티어를 `server/src/core/data/season.ts`와 동일 수치로 미러한다.
- `UIdleGameInstance::ClaimQuest()` 성공 시 클라이언트 V1 시즌 토큰 `+10`을 적립한다.
- `AIdleMonster::HandleDeath()`는 골드 드롭 수량과 장비 드롭 확률에 장착 펫 보너스를 곱한다.
- 네트워크는 `/v1/pets`, `/v1/season` 조회와 장착/수령 요청을 best-effort로 호출하며 실패해도 로컬 진행을 막지 않는다.

### 2.3 데이터/밸런스
- 펫 보너스 % (강아지 골드 +20%, 새 드롭 +15% 1차), 시즌 티어 요구 토큰/보상 1차값. 문서.

### 2.4 테스트
- 서버 Vitest + 클라 Automation.

## 3. 범위 외
- 펫 3~5종(햄스터/슬라임/여우), 펫 레벨/먹이주기 성장, 펫 합성(후속).
- 시즌 패스 프리미엄 트랙/결제, 시즌 종료/교체 자동화, 가챠(1.0 이후).
- 펫 아트/애니(후속, 방향만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 백엔드 (메인) | pets.ts/season.ts + pet/season 모듈·진행·토큰·수령 + migration + Vitest | ✅ 메인 (`backend`) |
| 캐릭터 | 펫 보너스 골드/드롭 반영 + 시즌 토큰 적립(퀘스트 연동) + Automation | ✅ 보조 (`character`) |
| 디자이너 | 펫/시즌 패스 UI | ✅ 보조 (`designer`) |
| 밸런스 | 펫 %/시즌 토큰·보상 + 문서 | ✅ 보조 (`balance`) |
| QA | 펫 장착·보너스 / 시즌 진행·수령 시나리오 | ✅ 보조 (`qa`) |

## 5. 호출 순서
1. `backend`(메인) → 데이터/모듈/진행/테스트  2. `character` → 보너스/토큰  3. `designer` → UI  4. `balance`/`qa` → ([4] 흡수 가능)

## 6. 워크플로우 v3
[1] 기획+PR → [2] Codex 개발(+게시) → [3] Claude TM → [4] Codex TM+fix → [5] Claude 검증 → [N] **CI 그린 확정** + PM 종합 + 머지. 사용자 PIE 차후 일괄([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]).

## 7. 리스크
| 리스크 | 완화 |
| --- | --- |
| 펫 보너스 서버/클라 parity | 동일 % 상수 미러 + parity 테스트 |
| 시즌 토큰 적립 경로(퀘스트 연동) | V1 클라 적립 + 서버 graceful, 권위 후속 |
| 시즌 종료 처리 | V1 단일 시즌 고정, 교체 후속 |

## 8. 후속
- 펫 확장/성장/먹이, 시즌 프리미엄 트랙/결제, 시즌 교체 자동화, 펫 아트.
