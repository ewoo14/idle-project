# M1 자동 전투 V1 QA 시나리오

PR #6 자동 전투 V1의 PIE 수동 검증 시나리오입니다. 모든 케이스는 UE5 Editor Output Log를 함께 확인합니다.

## 1. PIE 진입 시 기본 전투 오브젝트 생성

- Given UE5 Editor에서 `Game` 레벨을 연다.
- When PIE를 시작한다.
- Then 플레이어 캐릭터 1명과 슬라임 몬스터 5마리가 생성되어 화면에 표시된다.

## 2. 자동 전투 타깃 추격

- Given PIE가 실행 중이고 몬스터가 살아 있다.
- When 별도 입력 없이 대기한다.
- Then 캐릭터가 가장 가까운 몬스터를 타깃으로 잡고 추격한다.

## 3. 사거리 도달 후 공격

- Given 캐릭터가 몬스터를 추격 중이다.
- When 캐릭터가 공격 사거리 안에 진입한다.
- Then 캐릭터가 공격을 시작하고 몬스터 HP가 감소한다.

## 4. 몬스터 사망과 골드 드롭 생성

- Given 몬스터 HP가 낮아진 상태다.
- When 몬스터 HP가 0이 된다.
- Then 몬스터가 사망 처리되고 GoldDrop이 생성된다.

## 5. GoldDrop 자동 흡수

- Given GoldDrop이 생성되어 있다.
- When 캐릭터가 GoldDrop 흡수 범위에 들어간다.
- Then Gold가 증가하고 GoldDrop Actor가 Destroy된다.

## 6. 몬스터 5초 후 재생성

- Given 몬스터가 사망했다.
- When 사망 후 5초가 지난다.
- Then 동일 위치에 몬스터가 다시 spawn된다.

## 7. HUD 진행도 실시간 갱신

- Given PIE가 실행 중이고 HUD가 표시되어 있다.
- When HP, EXP, Gold 값이 변경된다.
- Then HUD의 HP / EXP / 골드 표시가 실시간으로 갱신된다.

## 8. NetworkClient guest register 호출

- Given 서버가 실행 중이다.
- When 게임을 시작한다.
- Then `NetworkClient` guest register 호출 로그와 성공 콜백이 Output Log에 표시된다.

## 9. 서버 미기동 graceful 진행

- Given 서버가 실행되지 않은 상태다.
- When 게임을 시작한다.
- Then register 실패 로그가 남더라도 게임 진행, 캐릭터 조작, 자동 전투가 계속 동작한다.

## 10. 모든 몬스터 동시 사망 후 일괄 respawn

- Given 몬스터 5마리가 모두 사망한 상태다.
- When 마지막 사망 시점 기준 5초가 지난다.
- Then 모든 몬스터가 각자의 원래 위치에 다시 spawn된다.
