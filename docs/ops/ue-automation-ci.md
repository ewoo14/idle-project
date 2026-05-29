# UE5 Automation CI / 로컬 게이트

> server-ci(GitHub-hosted 러너)는 **UE5 빌드/Automation을 검증하지 않는다**(러너에 UE 미설치). 이 사각지대로 과거 잠재 red가 누적됐다(예: #73 골드 오버플로, #75 던전 패널 stale, #81 SaveVersion 단언 stale). 본 문서는 그 사각지대를 닫는 게이트를 정의한다.

## 1. 로컬 게이트 (즉시 사용 — 머지 전 PM 필수)

`tools/ci/ue-automation.ps1` — 표준 jumbo(unity) 빌드 + 광범위 Automation을 한 번에 돌리고 실패 시 비0 종료.

```powershell
# 전체 IdleProject (기본, 머지 게이트 권장)
./tools/ci/ue-automation.ps1

# 빠른 점검(좁힌 필터)
./tools/ci/ue-automation.ps1 -Filter "IdleProject.Mastery+IdleProject.GameCore.SaveSystem"
```

- **표준 jumbo 빌드**(`-DisableUnity` 없이) → jumbo ODR 충돌 검출. `-DisableUnity` 우회 금지(adaptive unity가 변경 파일을 제외해 ODR을 놓침).
- **Automation 파싱** → `Result={Fail}` 또는 비정상 `EXIT CODE` 시 exit 1, 실패 테스트 + `Expected ... but it was` 진단 출력.
- 에디터가 열려 있으면 빌드 실패(Live Coding) → 종료 후 실행.
- 종료코드: 0 성공 / 1 빌드·테스트 실패 / 2 환경(UE 경로/uproject 없음).

> **머지 전 [5] 검증은 본 스크립트(기본 전체 필터)로** — SaveVersion bump·오버플로 등 코어 단언 스위트(IdleGameInstance/SaveSystem/Dungeon 등)까지 전수 검증되어야 한다([[reference-ue-headless-verify]] §1-b).

## 2. self-hosted 러너 (CI 자동화 — 러너 등록 필요)

`client-ci.yml`의 `ue5-automation` job이 동일 스크립트를 self-hosted Windows 러너에서 실행한다. **러너 미등록 상태에서 PR을 막지 않도록 라벨 `ue/automation` 게이트** (라벨 + 러너 둘 다 있을 때만 실행).

### 러너 등록 (1회, 사용자/개발책임자)
1. GitHub 리포 → Settings → Actions → Runners → New self-hosted runner (Windows).
2. 안내대로 러너 설치 후, **labels에 `ue5` 추가**(`self-hosted, windows, ue5`).
3. 러너 머신에 UE 5.7(`C:\Program Files\Epic Games\UE_5.7`) 설치 + LFS(`git lfs install`).
4. 서비스로 등록(`./svc.sh`/config) 해 상시 대기.

### 활성화 흐름
- 러너 등록 후, client 변경 PR에 **`ue/automation` 라벨**을 붙이면 `ue5-automation` job이 표준 jumbo 빌드 + 전체 IdleProject Automation을 실행.
- 안정화되면 `client-ci.yml`의 `if:` 라벨 게이트를 제거하고 `pull_request` paths `client/**` 자동 트리거로 승격(러너 상시 가동 전제).

## 3. 한계 / 후속
- GitHub-hosted 러너로는 UE 빌드 불가(엔진 미설치, 디스크/시간). self-hosted가 유일한 클라우드 자동화 경로.
- 후속: 러너 등록(개발책임자) → 라벨 게이트 → 자동 트리거 승격. Automation 결과 아티팩트(리포트 json) 업로드.
