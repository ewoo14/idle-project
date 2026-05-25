로컬라이즈 한/영 V1 의 i18n 인프라 + HUD 라벨 테이블화 + 언어 전환을 구현하라. 기획서: docs/planning/slices/24-localization-ko-en-v1.md. 기존: client/Content/Localization/Game/ko/StoryText.csv, client/Source/IdleProject/UI/IdleHUD.cpp + IdleHUDWidget.cpp (하드코딩 한글 라벨 다수), GameCore/IdleGameInstance.

구현 범위 (이번 designer 호출):
1. 문자열 테이블 인프라: client/Content/Localization/Game/{ko,en}/ 에 네임스페이스별 CSV(또는 UE StringTable) — UI(스킬/퀘스트/오프라인/환생/펫/직업 HUD 라벨), Story(기존 ko 확장), Skill, Quest. ko 와 en 동일 키셋.
   - UI 네임스페이스: 기존 HUD 하드코딩 한글 라벨("스킬","퀘스트","수령","환생","펫","직업 선택","경과 시간","획득 골드" 등)을 키로 추출.
2. HUD 라벨 치환: IdleHUD/IdleHUDWidget 의 핵심 하드코딩 FText::FromString("한글") → StringTable/LOCTEXT 키 참조로. (전수 아니어도 핵심 HUD 라벨 위주, 잔여는 키 추가만.)
3. 언어 전환: UIdleGameInstance(또는 Settings) 에 Language(ko/en) + 적용(UE Culture/Localization 또는 자체 조회 키→현재 언어 문자열) + 저장(SaveLastSeen 패턴). 토글 메서드.
4. en 키는 일단 영문 placeholder/직역으로 채우되 미번역 0 (자연스러운 번역은 story 후속 호출이 보강). UI 라벨 en 은 designer 가 채움.
5. Automation(Tests/): 로컬라이즈 조회 모델(키→ko/en 문자열), 키셋 ko↔en 정합(누락/중복 0), 언어 전환.

제약: 한글 주석. Story/Skill/Quest 본문 번역 품질 보강은 story 후속 — designer 는 인프라 + UI 라벨 + 전환. 가능하면 Build.bat + Automation 검증. push 금지. 커밋 prefix: codex(designer):.
