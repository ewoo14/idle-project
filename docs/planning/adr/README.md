# Architecture Decision Records (ADR)

주요 기술 결정을 시점 / 사유 / 대안 / 결정 / 영향 의 5단으로 기록합니다.

## 명명

`NNNN-<제목>.md` (4자리 번호, 한글 제목 가능)

예:
- `0001-server-runtime-node-vs-go.md`
- `0002-db-postgres-vs-mongo.md`
- `0003-ue5-version-54-lts.md`

## 템플릿

```markdown
# ADR-NNNN: <제목>

- 시점: YYYY-MM-DD
- 상태: 제안 | 수락됨 | 폐기됨 | 대체됨(NNNN)
- 결정자: PM
- 관련 PR: #N

## 컨텍스트
> 어떤 상황에서 무엇을 결정해야 했는가.

## 대안
1. 옵션 A — 장단점
2. 옵션 B — 장단점
3. 옵션 C — 장단점

## 결정
> 어떤 옵션을 선택했는가, 왜.

## 영향
> 어떤 시스템/팀/일정에 영향을 주는가. 후속 작업.

## 폐기 / 변경 시
> 어떤 조건이면 재검토하는가.
```

첫 ADR 들은 백엔드 슬라이스 PR (#7) 에서 작성 예정.
