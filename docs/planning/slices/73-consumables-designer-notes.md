# PR #73 Consumables Designer Notes

## Scope

- HUD adds a compact consumable inventory panel with six fixed rows:
  AttackTonic, GuardTonic, AllStatElixir, FortuneScroll, GoldFeast,
  WisdomBooster.
- Each row shows localized name, localized effect tooltip text, owned count,
  `Use` action, and active remaining time when applicable.
- Active buff bar shows the first active buff with name and countdown; the full
  row list still exposes all active states.

## Layout

- Placement: right-side utility stack under the gold shop area.
- Scale: Canvas-height clamp 1.0-2.0, matching existing HUD panels for 1080p,
  1440p, and 4K.
- Colors: existing `--bg-panel`, `--bg-primary`, `--text-primary`,
  `--text-muted`, `--accent-gold`, and `--accent-blue` only.
- Active buffs use `--accent-gold`; usable stocked rows use `--accent-blue`;
  empty rows use muted text.
- No new bitmap or UMG binary assets were added. Existing `.gitattributes`
  already LFS-tracks future `*.uasset` and image exports.

## Localized Copy

| Type | ko name | en name | ko effect | en effect |
| --- | --- | --- | --- | --- |
| AttackTonic | 공격 토닉 | Attack Tonic | 물공/마공 +30% 30분 | PATK/MATK +30% for 30m |
| GuardTonic | 수호 토닉 | Guard Tonic | HP/물방/마방 +30% 30분 | HP/PDEF/MDEF +30% for 30m |
| AllStatElixir | 전능 엘릭서 | All-Stat Elixir | 코어 스탯 +20% 30분 | Core stats +20% for 30m |
| FortuneScroll | 행운 주문서 | Fortune Scroll | 드롭 +30% 30분 | Drop +30% for 30m |
| GoldFeast | 황금 만찬 | Gold Feast | 골드 +50% 30분 | Gold +50% for 30m |
| WisdomBooster | 지혜 부스터 | Wisdom Booster | EXP +50% 30분 | EXP +50% for 30m |

## ViewModel Contract

- `BuildConsumablePanelViewModel(const UBuffService&, int64 NowUnixSec)` is the
  UI contract for Canvas/UMG.
- Owned counts use `UBuffService::GetCount`.
- Active state uses `UBuffService::IsBuffActive`.
- Countdown uses `UBuffService::GetBuffRemainingSec` and formats as `MM:SS`.
- Effect labels use the ko/en `UI.csv` keys, not hard-coded runtime text.
- Use hitboxes call `UIdleGameInstance::TryUseConsumable`.
- `UIdleGameInstance::TryBuyConsumable` remains the shop acquisition API for
  future UMG shop buttons.

## Validation

- `IdleProject.UI.HUD.ConsumablePanelViewModel` covers row order, localized
  names/effects, owned count, use action, hitbox naming, active state, and
  countdown.
- `IdleProject.Localization.CsvIntegrity` includes all consumable keys so ko/en
  parity fails fast on missing rows.
