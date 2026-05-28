# PR #63 Designer Notes - Rune Class Slot HUD

## HUD Placement

Rune Class Slot HUD V1 extends the existing right-side rune panel instead of adding a new surface. Slot index 6 uses the localized `Class Rune` title, class-specific mastery copy, and the same rarity/value columns as the six regular slots.

The common class-rune craft CTA is a compact row between the rune shop header and slots. It uses `--accent-blue` when craftable, `--accent-red` for insufficient essence, and `--text-muted` for disabled text.

## Responsive Check

The panel keeps the existing 1080p/1440p/4K Canvas-height 1.0-2.0 scaling and 380-500px width clamp. The added craft row increases height only inside the rune panel, so it does not create a separate overlap target.

## Assets

No new bitmap assets are introduced. Existing LFS rules in `.gitattributes` cover future UI art exports for `.uasset`, `.png`, `.jpg`, `.fbx`, audio, and video assets.
