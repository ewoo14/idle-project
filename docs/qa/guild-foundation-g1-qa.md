# 길드 기초 PR-G1 QA 검증 노트

> 대상: PR-G1 (CRUD/멤버십/계급) · 작성 2026-05-29 · 검증 주체: PM

## 1. 자동 검증 게이트 결과

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/modules/guild`) | 생성/가입(자유·승인)/정원/1캐릭1길드/계급 해금·정원/권한/탈퇴·위임·해산 | **27 passed** |
| 서버 lint(biome)/build(tsc) | 전체 | **GREEN** (147 files / 0 err) |
| UE 표준 jumbo 빌드 | 전체 모듈(ODR) | **Succeeded** |
| UE Automation(1차) | GameCore/Mastery/Consumable/Dungeon/Rune (SaveVer17 회귀) | **118 / 0** |
| UE Automation(2차) | GameCore/Localization/UI (길드 패널·CsvIntegrity) | (본 PR 게이트 — 본문 갱신) |

## 2. 시나리오 커버리지 (서버 vitest 단위)

- 길드 생성: 무소속만, 이름 2~16·유니크, 생성자=길드장.
- 가입 자유: 즉시 가입(member). 승인: 신청→approve(정원 재검증)/reject.
- 거부 케이스: 정원 30 초과, 1캐릭1길드 중복, 권한 없는 actor 관리 시도.
- 계급: 해금 경계(10→vice잠금/11→해금, 20→officer잠금/21→해금), 슬롯(vice 1·officer 3) 초과 거부.
- 길드장 공백: 부길드장 위임 → 최고 기여 멤버 위임 → 후보 0 해산.

## 3. 클라 Automation (서버 무의존)

- `IsRankUnlocked`/`GetRankSlotCap` 서버 parity(임계 11/21, 슬롯 1/3).
- `ApplySnapshot` 후 접근자(HasGuild/GetMyRank/GetMembers) 정확.
- 세이브 **v17 라운드트립**(CachedGuildId/CachedGuildRank save→load 일치), 전 세이브 테스트 v17 갱신 회귀.
- 로컬라이즈 길드 50키 ko/en CsvIntegrity.

## 4. 한계 / 후속

- **cross-DB 정합 SQL**(guild_members.character_id ↔ characters.id)은 라이브 PG/testcontainers 미기동으로 본 PR 단위 검증에서 제외 — `GuildRepoPg` 실제 SQL은 통합 테스트 인프라 가동 시 또는 스테이징 E2E에서 검증 권장.
- 길드명 자유 입력 위젯 부재로 프리셋 순환 방식(후속 텍스트 입력 인프라 도입 시 개선).
- PR 본문에 길드 패널 스크린샷 첨부(무소속/내 길드/관리 3화면).
