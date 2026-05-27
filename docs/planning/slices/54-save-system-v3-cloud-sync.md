# PR #54 기획서 — Save System v3 클라우드 세이브 동기화

> **저장 3부작 완성.** V1(#52 코어)·v2(#53 Inventory/Skill/Quest/Season)는 **로컬 USaveGame** 전용. 서버에는 #7 백엔드 save 모듈(`GET/PUT /save/`, 버전·캐릭터 종속·퇴행 거부 검증·리더보드 연동)이 있으나 **클라이언트가 로컬 세이브를 서버와 동기화하지 않는다**(ApiClient 에 save up/down 부재, characterId 미발급). save-v3 는 로컬↔서버 세이브를 잇는다: 로그인/접속 시 서버 세이브 다운로드+병합, autosave/종료 시 업로드, 오프라인 graceful 폴백, 충돌 해소. server + client 멀티시스템(+designer/qa).

## 1. 목표 / DoD
계정 로그인 후 한 기기에서 진행한 클라우드 세이브가 다른 기기/재설치에서 복원된다. 서버 불가 시 기존 로컬 세이브로 graceful 동작(회귀 없음).

### DoD 검증
1. **클라 ApiClient 동기화 API**: `EnsureCharacter`(POST /character/ 또는 GET 으로 characterId 확보·캐시) + `UploadSave`(PUT /save/ {characterId, version, payload}) + `DownloadSave`(GET /save/?characterId) 콜백. 기존 RegisterGuest/JWT(AuthToken) 흐름 재사용.
2. **세이브 payload 매핑(클라)**: 로컬 코어 상태 → 서버 payload(level/rebirthCount/maxEquipmentGrade 필수 + gold/totalExp/lastSeenUnixSec + 확장 필드 transcendCount/towerHighestFloor/skillPoints 등 `additionalProperties`). version 단조 증가(로컬 보관).
3. **GameInstance 동기화 훅**: Init/접속 성공 시 DownloadSave→로컬과 병합(MergeServerSave). autosave(#52 RequestAutosave 디바운스)·Shutdown 시 로컬 저장 후 UploadSave(throttle). 서버 미응답/미인증 시 로컬 전용(기존 동작 유지, 회귀 금지).
4. **충돌 해소 정책**: 서버 검증이 이미 퇴행 거부(level/rebirth/gold) → 클라 정책 = 접속 시 서버 세이브가 로컬보다 진행도 높으면(우선순위 rebirthCount→level→gold) 채택·적용, 아니면 로컬 업로드. 결정 로직 순수 함수(`FCloudSaveMergePolicy`)로 분리해 테스트.
5. **서버 스키마/검증 현행화(필수)**: 현재 클라(#45 Mythic=6, 무한성장/환생/초월/탑)가 서버에 거부되지 않도록 `putSaveSchema`/`validateSavePayload` 갱신 — maxEquipmentGrade 0~5→0~6(Mythic), level 상한 현행화(환생 리셋 고려), transcendCount/towerHighestFloor 등 확장 필드 검증 추가, totalExp-level 결합 환생/초월 정합(또는 선택 필드 완화). 기존 퇴행 거부·anti-cheat 유지.
6. **테스트**: 서버 vitest(스키마/검증 신규 캡·필드, 퇴행 거부 회귀) + 클라 Automation(MergeServerSave 정책: 서버 우세→채택/로컬 우세→업로드/동률, payload 매핑 라운드트립, 서버 미응답 시 로컬 폴백). UE 빌드/Automation + 서버 build/test/lint GREEN.

## 2. 범위 (In Scope)
### 2.1 서버 (backend) — save 스키마/검증 현행화
- `save.schema.ts` putSaveSchema: maxEquipmentGrade max 6, level 상한 상향(또는 합리적 캡), 확장 필드(transcendCount/towerHighestFloor/skillPoints) 명시. `save.service.ts` validateSavePayload 동기 갱신(퇴행 거부 유지, totalExp 결합 환생/초월 케이스 정합). vitest 갱신.
### 2.2 클라 (character/network) — 동기화
- `ApiClient`: EnsureCharacter/UploadSave/DownloadSave(HTTP + 콜백, JSON 직렬화). characterId 캐시(GConfig 또는 SaveGame).
- `UIdleGameInstance`: 서버 payload 매핑(Capture 코어→payload), DownloadSave→`FCloudSaveMergePolicy::Decide`→적용, autosave/Shutdown UploadSave(throttle), 오프라인 폴백. version 보관.
- `FCloudSaveMergePolicy`(순수): 로컬 vs 서버 진행도 비교(rebirthCount→level→gold→lastSeen)→AdoptServer/KeepLocal 결정.
### 2.3 UI (디자이너)
- 동기화 상태 인디케이터(동기화됨/오프라인/업로드 중/충돌 해소) HUD + 로컬라이즈 ko/en. 기존 "저장됨" 토스트와 정합.
### 2.4 테스트
- 서버 vitest + 클라 Automation(위 DoD 6).

## 3. 범위 외 (후속)
- 실시간 멀티 디바이스 동시 편집/락, 세이브 암호화/서명, 다중 캐릭터 슬롯 UI, 서버 권위 시뮬레이션(치트 전수 차단). 풀 계정 UI(이메일 가입/소셜 로그인)는 별도.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 백엔드 | save 스키마/검증 현행화(grade6/level/확장필드/퇴행유지) + vitest | ✅ 메인 (`backend`) |
| 캐릭터·전투 | ApiClient up/down/EnsureCharacter + GameInstance 동기화 훅 + FCloudSaveMergePolicy + Automation | ✅ 메인 (`character`) |
| 디자이너 | 동기화 상태 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| QA | 다운로드→플레이→업로드, 오프라인 폴백, 충돌 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex backend+character 메인(+designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정**(서버 변경 → server-ci 포함) + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 서버 검증 캡(level200/grade5)이 현재 클라 거부 | 스키마/검증 현행화(grade6, level 상향, 확장 필드), 퇴행 거부 anti-cheat 유지 + vitest |
| 클라우드 병합 데이터 손실/덮어쓰기 | `FCloudSaveMergePolicy` 순수 함수 + Automation(서버우세/로컬우세/동률), 서버 퇴행 거부와 정합 |
| 서버 미응답 시 진행 차단 | 동기화 실패 graceful → 로컬 전용 동작 유지(기존 회귀 금지), 콜백 타임아웃 |
| version 충돌 | version 단조 증가(로컬 보관) + 서버 history 누적, 접속 시 최신 다운로드 |
| HTTP/직렬화 정합(클라 JSON ↔ 서버 schema) | payload 필드명/타입 정합 테스트, 서버 vitest 스키마 검증 |
| characterId 발급 흐름 | EnsureCharacter(없으면 생성/캐시), 미인증 시 로컬 전용 |

## 7. 후속
- 실시간 멀티 디바이스 락, 세이브 암호화/서명, 다중 캐릭터 슬롯, 풀 계정 UI(이메일/소셜).
