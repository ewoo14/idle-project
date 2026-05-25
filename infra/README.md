# infra/ — 인프라

Docker Compose 기반 개발/운영 스택.

## 빠른 시작

```powershell
cd infra
Copy-Item .env.example .env
# (선택) .env 의 비밀번호를 수정

docker compose up -d              # postgres + redis
docker compose ps                  # 상태 확인
docker compose logs -f postgres    # 로그
docker compose down                # 정리
```

## 프로파일

| 프로파일 | 추가 서비스 | 용도 |
| --- | --- | --- |
| 기본 | postgres, redis | 항상 |
| `dev` | + pgadmin | 개발 |
| `app` | + server | PR #7 머지 이후 |
| `ops` | + prometheus, grafana | 1.0 이후 |

예: `docker compose --profile dev --profile app up -d`

## 비밀 관리

- 로컬: `.env` 파일 (gitignore)
- 운영: docker secrets 또는 호스트 환경 변수 + secret manager
- JWT 키: 첫 부팅 시 `infra/secrets/jwt_private.pem`, `jwt_public.pem` 생성 후 마운트 (PR #7 절차)

## 백업

`docs/ops/recovery.md` (PR #7 에서 추가) 참고.

## 추가 문서

- 자세한 운영: `docs/ops/docker.md`
- 아키텍처: `docs/planning/02-architecture.md`
