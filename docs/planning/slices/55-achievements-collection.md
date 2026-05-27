# PR #55 기획서 — 업적·수집 시스템 (영구 성장 메타 벡터)

> **방치형 영구 성장 메타 벡터.** 처치/강화/환생/초월/탑/수집 누적 달성이 **영구 글로벌 스탯 배수**를 부여한다. 초월(#47)·탑 마일스톤(#51)처럼 `RefreshDerivedStats` 합성 배수(Transcend × Tower × **Achievement**)에 곱해진다. [Infinite Growth] 정합 — **무한 티어 트랙**(누적 임계가 곡선으로 무한 증가)으로 낮은 하드 캡 지양. 경제 중립(골드 유입 아닌 파워 배수). server + client 멀티시스템(+designer/qa/balance).

## 1. 목표 / DoD
플레이어가 누적 행위(처치·강화·환생·초월·탑·수집)로 업적 티어를 달성하면 영구 스탯 배수가 늘어 CP(#49)가 무한 성장한다. 업적 진행은 저장(#52~#54)으로 영속·클라우드 동기화된다.

### DoD 검증
1. **업적 카테고리·항목 (최대한 많고 다양하게 — 사용자 지시 [[project-content-richness]])**: 소수 트랙이 아니라 **다수 카테고리 × 다수 항목**. 각 누적 트랙은 무한 티어(곡선 임계 base×growth^tier). 카테고리(최소 8, 항목 수십 개):
   - **전투(Combat)**: 누적 처치(MonstersKilled), 보스 처치, 크리티컬 누적, 누적 피해, 상태이상 부여, 속성별 처치
   - **진행(Progression)**: 도달 레벨 마일스톤, 스테이지/챕터 클리어, 환생 횟수, 초월 횟수, 탑 최고층
   - **장비(Gear)**: 강화 누적, 최고 강화 단계, 등급별 아이템 획득(Common~Mythic), 세트 완성, 슬롯 전부 장착
   - **경제(Economy)**: 누적 골드 획득, 골드 소비, 상점 뽑기 횟수
   - **스킬(Skill)**: 스킬 랭크 누적, 최고 랭크, 직업별 스킬 마스터
   - **펫(Pet)**: 펫 먹이기, 펫 레벨 합
   - **퀘스트(Quest)**: 메인/일일 완료 누적, 시즌 티어 수령
   - **수집(Collection)**: 고유 아이템 발견(등급/슬롯/세트별), 고유 몬스터 처치, 고유 스킬 보유
   - **기타(Misc)**: 오프라인 보상 수령, 플레이 마일스톤
   각 항목은 정의 테이블(ID/카테고리/지표/티어 임계/포인트/표시명)로 **다수(수십 개) 등록**. 단발 달성 + 무한 티어 혼합. 클라/서버 정의 미러(parity).
2. **영구 스탯 배수**: `FAchievementFormula::GetStatMultiplier(완료 티어/포인트) = 1 + TotalPoints×0.01`(무한, 점근 아님 — 곡선 임계로 후반 둔화). `UIdleGameInstance::GetAchievementStatMultiplier()` → `AIdleCharacter::RefreshDerivedStats` 합성 `StatMultiplier = Transcend × Tower × Achievement`(Hp/물공/마공/물방/마방, rate 제외 — #51 일관). 0 포인트 ×1.0 회귀안전.
3. **진행 추적(UAchievementService)**: 기존 `RecordMonsterKilled`/`RecordGearEnhanced`/`RecordQuestProgress` + 레벨업/환생/초월/탑 이벤트 훅으로 누적 지표 갱신 → 티어 달성 판정 → 영구 포인트 반영 + 델리게이트(OnAchievementUnlocked/OnAchievementProgress).
4. **영속/클라우드**: 업적 상태(누적 지표 + 달성 티어/포인트) UIdleSaveGame 추가 → CaptureToSave/ApplyFromSave(#52/#53 패턴, RestoreState 새니타이즈) → clientSave 경유 클라우드 동기화(#54 자동). v2 세이브 호환(없으면 0 기본=회귀안전).
5. **클라/서버 공식 미러**: 서버 achievement 모듈(achievement.ts: 티어 임계·포인트·GetStatMultiplier) + parity 테스트(Math.fround float32 정합) — transcend/tower 패턴 일관. (업적 상태 권위는 클라+세이브; 서버는 공식/정의 미러 + 후속 검증 여지.)
6. **테스트**: 클라 Automation(트랙 티어 달성·포인트·GetStatMultiplier·합성 배수·라운드트립·0 회귀안전) + 서버 vitest(공식 parity·티어 임계) + balance(배수 곡선 무한·비폭발 검증). UE 빌드/Automation + 서버 build/test/lint GREEN.

## 2. 범위 (In Scope)
### 2.1 공식 (character + backend 미러)
- `FAchievementFormula`(GameCore): 트랙 티어 임계(base×growth^tier 곡선), 티어→포인트, TotalPoints→StatMultiplier(1+points×0.01). 서버 `achievement.ts` 미러 + parity(Math.fround).
### 2.2 진행/서비스 (character)
- `UAchievementService`: 트랙별 누적 지표 + 달성 티어/포인트 보관, RecordXxx/이벤트 훅 → 티어 판정 → OnAchievementUnlocked. CaptureState/RestoreState(새니타이즈).
- `UIdleGameInstance`: GetAchievementStatMultiplier(), Record* 훅에서 service 갱신, Ensure 패턴, UIdleSaveGame 연동. RefreshDerivedStats 재계산 트리거.
- `AIdleCharacter::RefreshDerivedStats`: 합성에 Achievement 배수 추가.
### 2.3 저장 (character) — UIdleSaveGame 업적 필드 + capture/apply/restore (clientSave 클라우드 자동).
### 2.4 UI (디자이너) — 업적 패널(트랙별 진행/현재 티어/다음 임계/총 배수) HUD + 토스트(달성) + 로컬라이즈 ko/en.
### 2.5 테스트 (character/backend/balance) — 위 DoD 6.

## 3. 범위 외 (후속)
- 서버 권위 업적 검증/리워드(현재 클라+세이브 권위), 업적 보상으로 재화/아이템 지급(현재 영구 스탯 배수만 — 경제 중립), 수집 도감 상세 UI(아이템별 일러스트), 일일/시즌 업적.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | FAchievementFormula + UAchievementService + GameInstance 훅/배수 + RefreshDerivedStats 합성 + UIdleSaveGame 연동 + Automation | ✅ 메인 (`character`) |
| 백엔드 | achievement.ts 공식/정의 미러 + parity vitest | ✅ 보조 (`backend`) |
| 디자이너 | 업적 패널 HUD + 달성 토스트 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 배수 곡선 무한·비폭발 시뮬 검증(balance-sim) | ✅ 보조 (`balance`) |
| QA | 트랙 달성→배수→CP 증가, 저장/클라우드 복원, 0 회귀 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증(UE+서버) → [N] **CI 그린 확정**(server-ci 포함) + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). [[project-infinite-growth]] [[feedback-substantial-slices]].

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 배수 폭발(무한 성장 과도) | 곡선 임계(후반 둔화) + points×0.01 소폭 + balance-sim 검증, rate 스탯 제외 |
| 클라/서버 공식 불일치 | achievement.ts 미러 + Math.fround parity 테스트(transcend/tower 패턴) |
| 합성 배수 회귀(Transcend×Tower) | 0 포인트 ×1.0, 합성 곱 순서 보존, Automation 합성 케이스 |
| 진행 이벤트 누락(훅) | RecordMonsterKilled/GearEnhanced/레벨/환생/초월/탑 전 경로 훅 + 단조 증가 Automation |
| 저장/클라우드 누락 | UIdleSaveGame 필드 + capture/apply/restore 새니타이즈 + clientSave 라운드트립(#54) |
| 기존 전투/스탯 회귀 | 업적 0 = 기존 동작, RefreshDerivedStats 합성만 추가 |

## 7. 후속 (콘텐츠 볼륨 확장 — 사용자 지시 [[project-content-richness]])
- **본 PR 후 콘텐츠 확장 슬라이스 연속**: 퀘스트 대폭 확장(메인 다수 + 주간/도전 + 다양화), 캐릭터 직업 추가(현 5직업→확장), 아이템 종류 확대(ID/세트/고유옵션 다양화).
- 서버 권위 업적 검증, 수집 도감 상세 UI, 일일/시즌 업적, 업적 보상 다양화(경제 영향 시 신중).
