# Docker 운영 가이드

## 1. 개요

idle-project 의 모든 서버 인프라는 Docker Compose 로 정의됩니다 (`infra/docker-compose.yml`).

| 서비스 | 이미지 | 포트 | 영속 |
| --- | --- | --- | --- |
| postgres | postgres:16-alpine | 5432 | `postgres-data` 볼륨 |
| redis | redis:7-alpine | 6379 | `redis-data` 볼륨 (AOF) |
| pgadmin | dpage/pgadmin4 | 5050 | dev 프로파일 |
| server | (server/Dockerfile) | 3000 | app 프로파일 (PR #7+) |
| prometheus | prom/prometheus | 9090 | ops 프로파일 (1.0+) |
| grafana | grafana/grafana | 3001 | ops 프로파일 (1.0+) |

## 2. 부팅 순서

1. `.env` 준비
2. `docker compose up -d postgres redis` (헬스체크 통과까지 대기)
3. `docker compose --profile dev up -d pgadmin`
4. PR #7 이후: `docker compose --profile app up -d server`

## 3. 헬스체크

- postgres: `pg_isready`
- redis: `redis-cli ping`
- 헬스체크 실패 시 30초 대기 후 재시도 → 5분 후 알람 (1.0 ops 프로파일).

## 4. 마이그레이션 (서버 PR #7 이후)

```powershell
docker compose exec server npm run db:migrate
```

롤백:
```powershell
docker compose exec server npm run db:rollback
```

## 5. 백업 / 복구

### 백업 (수동)
```powershell
docker compose exec postgres pg_dump -U $env:POSTGRES_USER $env:POSTGRES_DB > "backup-$(Get-Date -Format yyyyMMdd).sql"
```

### 자동 백업 (1.0)
- cron job — 매일 03:00 KST → `infra/backups/` → 7일 보관 → S3 (선택)

### 복구
```powershell
Get-Content backup-20260601.sql | docker compose exec -T postgres psql -U $env:POSTGRES_USER $env:POSTGRES_DB
```

## 6. 트러블슈팅

| 증상 | 원인 후보 | 조치 |
| --- | --- | --- |
| postgres restart loop | 디스크 권한 / 비밀번호 변경 | 볼륨 삭제 후 재생성 (개발 한정) |
| 포트 충돌 | 호스트에 다른 PG 실행 | `.env` 의 `POSTGRES_PORT` 변경 |
| server "ECONNREFUSED" | postgres 헬스체크 전 시작 | depends_on healthy 확인 |
| pgadmin 접속 안 됨 | 5050 포트 방화벽 | Windows Defender 허용 |

## 7. 보안 체크리스트 (운영 전환)

- [ ] 모든 비밀번호 `change_me_local_only` 미사용
- [ ] 외부 포트 노출 최소화 (postgres/redis 는 사내망만)
- [ ] HTTPS 종단 (nginx / Caddy)
- [ ] 백업 검증 정기 수행
- [ ] 시크릿 회전 (90일)
