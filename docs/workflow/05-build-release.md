# 05 — 빌드 / 릴리스 자동화 (M6 V1)

> 출시(Steam EA) 준비 빌드/릴리스 절차. V1 은 **서버 Docker 자동화 + UE 패키징 스크립트 + 버전 태그 릴리스**. 사운드/Steam SDK 업로드는 외부 의존(에셋·파트너 계정) 후속.

## 버전
- 단일 소스: 루트 `VERSION` 파일(예 `0.1.0`). 서버 이미지 태그·UE 패키징 폴더명에 사용.
- 릴리스: `VERSION` 갱신 → 커밋 → `git tag v<버전>` → push.

## 자동화 (`.github/workflows/release.yml`)
- `v*` 태그 push(또는 수동 dispatch) 시:
  1. **서버 Docker 이미지 빌드 검증** — `docker build ./server` (multi-stage, node:22-alpine).
  2. **GitHub 릴리스 생성** — `gh release create --generate-notes`.
- 외부 레지스트리 push(GHCR/Docker Hub)는 레지스트리 시크릿 필요 → 후속(시크릿 설정 후 push 스텝 추가).

## UE 클라이언트 패키징 (`tools/build/package-client.ps1`)
- GitHub 표준 러너에 UE 5.7 부재 → **로컬 또는 self-hosted 러너**에서 실행:
  ```powershell
  .\tools\build\package-client.ps1 -Configuration Shipping
  ```
- BuildCookRun(cook/build/stage/pak/archive) → `client/Saved/Packaged/<버전>-<구성>/`.
- CI 패키징은 self-hosted 러너(UE 설치) 도입 시 release.yml 에 잡 추가(후속).

## 후속 (외부 의존)
- 사운드/BGM 에셋 통합(에셋 필요).
- Steam SDK(Steamworks 파트너 계정) + 빌드 업로드(steamcmd).
- self-hosted 러너 UE 패키징 CI.
- Docker 레지스트리 push(시크릿).
