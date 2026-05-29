# 길드 기초 밸런스 노트 — PR-G1 (CRUD/멤버십/계급)

> 대상: PR-G1 · 스펙 [`2026-05-29-guild-foundation-design.md`](../superpowers/specs/2026-05-29-guild-foundation-design.md) · 작성 2026-05-29
> 범위: 멤버십·계급 구조만. **기여/레벨/버프=PR-G2, 길드 보스/랭킹=PR-G3 (본 노트 범위 밖).**

## 1. 정원

| 항목 | 값 | 근거 |
| --- | --- | --- |
| 길드 최대 정원 | **30** | 비동기 협력 규모·공유 보스 HP 페이싱(G3) 적정. 길드 레벨 기반 정원 확장은 후속. |
| 1캐릭터 소속 | **1길드** (`guild_members.character_id` UNIQUE) | 정합성·단순성 |

## 2. 계급 (군대식, 인원 해금)

소규모 길드는 관리 부담을 낮추고(길드장/멤버만), 인원이 늘면 상위 계급이 해금되어 위계가 생긴다.

| 계급 | 해금 조건(멤버 수) | 슬롯 상한 | 권한 |
| --- | --- | --- | --- |
| 길드장(master) | 항상 1 | 1 | 전권(설정 name/join_mode, 계급 승강, 승인/거절, 해산·위임) |
| 부길드장(vice) | **≥ 11** | **1** | 승인/거절, notice 수정 |
| 간부(officer) | **≥ 21** | **3** | (G1) 명예 계급 — 승강 대상. 세부 권한은 후속 |
| 멤버(member) | 항상 | 무제한(정원 내) | 기본 |

- 클라(`GuildFormula::IsRankUnlocked`/`GetRankSlotCap`)와 서버(`isRankUnlocked`/`getRankSlotCap`)가
  **동일 상수**: `GUILD_CAPACITY=30`, `VICE_UNLOCK_AT=11`, `OFFICER_UNLOCK_AT=21`, `VICE_SLOT_CAP=1`, `OFFICER_SLOT_CAP=3`.
- 승강 검증: 해금 미충족 또는 슬롯 초과 시 거부. 정원 계산은 "대상 본인 제외 현재 해당 계급 인원 < cap".

## 3. 가입 모드 (길드별 선택)

| 모드 | 동작 |
| --- | --- |
| 자유(open) | 정원·중복 검증 후 **즉시 가입**(rank=member) |
| 승인(approval) | `guild_join_requests`에 신청 → 길드장/부길드장 approve/reject. approve 시 정원 재검증 |

길드장이 설정(`PATCH /v1/guilds/:id`, join_mode)으로 언제든 전환.

## 4. 길드장 공백 처리 (탈퇴/해산)

길드장이 탈퇴하면 무주공산을 막기 위해 자동 위임 또는 해산:
1. 부길드장 존재 → 부길드장에게 위임(master).
2. 없으면 `total_contribution` 최다 멤버에게 위임.
3. 후보 0명(혼자) → **길드 해산**(guilds 삭제 → FK cascade로 members/requests 정리).

## 5. SaveVersion

- **16 → 17**: 클라 로컬 캐시 `CachedGuildId`(FString), `CachedGuildRank`(uint8) 추가.
  서버 권위 스냅샷(`GET /v1/guilds/me`)을 재접속 시 캐시 — 오프라인에서도 소속/계급 표시.
- 기여/버프 캐시는 G2(SaveVer 18 예정), 보스 진행 캐시는 G3(SaveVer 19 예정).

## 6. 비고

- 헌납·기여도·길드 레벨 버프·상점은 본 PR에 **없음** → 기존 CP/전투 공식 불변.
- 어뷰즈/일일 상한 등 수치 밸런스는 G2 기여도 도입 시 별도 노트.
