# 길드 기초 PR-G2 QA 검증 노트

> 대상: PR-G2 (기여도/길드 레벨·버프/상점) · 작성 2026-05-30 · 검증: PM

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/modules/guild`) | 기여 3종/상한/길드 레벨 임계/주간 리셋/상점 구매·부족거부 | **44 passed**(G1 22 + G2 22) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation | GameCore(버프 parity·ShopRewardGrant·SaveSystem v18)/Combat/Mastery/Consumable/Dungeon | **126/0** |
| UE Automation(UI) | GameCore/Localization(CsvIntegrity)/UI/Combat | **147/0** |

## 2. 시나리오 커버리지

- **기여 3종**: 일일 출석(+50, 1일1회·중복거부), 헌납(floor(gold/1000)·일일상한500·골드 클라권위 미차감), 전투/던전 자동(던전+5/보스+10, 주간상한2000).
- **누적 동시 적립**: guild.exp(레벨) + contribution_points(상점) + weekly_contribution(랭킹, 멤버별 weekly_reset_id lazy 리셋).
- **길드 레벨/버프**: getGuildLevel 누적 임계(10000×1.6^) 경계, getGuildBuff(+0.4%/Lv) 서버↔클라 parity. 공격력·골드 **단일 적용 지점**(이중적용 가드, #72 교훈), 오프라인 캐시 적용.
- **상점**: 6종 실존 재화(보호서/재설정큐브/골드/지혜물약/등급큐브/룬정수) 구매 → 포인트 차감 + **실제 재화 지급**(ApplyGuildShopReward), 포인트 부족 거부.

## 3. 클라 Automation (서버 무의존)

- 버프 parity(레벨→%, 누적 임계 경계), CP/골드 버프 반영, 기여 델타 누적·플러시·주간 클램프, 오프라인 버프 캐시.
- **ShopRewardGrant**: gold/essence(RuneEssence)/expPotion(WisdomBooster)/protectionScroll/resetCube/rankCube 각 지급 + 미지 타입 불변.
- 세이브 **v18 라운드트립**, 전 세이브 테스트 v18 갱신.

## 4. 설계 교정 기록 (PM)

- 1차 구현이 상점 보상을 **게임에 없는 재화(enhanceStone/petFood)** 로 발명 → 죽은 재화 + SaveVer 19 과다 bump. PM이 **실존 #71/#61/#73 재화로 교정** + 죽은 재화 제거 + SaveVer 18 복원([[feedback_substantial_slices]] 취지: 작동하는 슬라이스).

## 5. 한계 / 후속

- cross-DB 정합 SQL은 라이브 DB 미기동으로 통합/스테이징 후속.
- 자동 기여 단위/주간 상한은 초기값 — 활동량 데이터로 재튜닝.
- 길드 보스 데미지 기여(4번째 발생원)·주간 길드 랭킹 = PR-G3. PR 본문 패널 스크린샷.
