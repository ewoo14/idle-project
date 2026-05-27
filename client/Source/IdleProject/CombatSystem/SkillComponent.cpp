#include "CombatSystem/SkillComponent.h"

#include "CharacterSystem/IdleMonster.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
#include "Internationalization/IdleLocalization.h"

namespace
{
const FName ShieldUpId(TEXT("shield_up"));
const FName ManaShieldId(TEXT("mana_shield"));
const FName FocusId(TEXT("focus"));
const FName WeaponMasteryId(TEXT("weapon_mastery"));
const FName ToughnessId(TEXT("toughness"));
const FName BerserkersFuryId(TEXT("berserkers_fury"));
const FName SpellMasteryId(TEXT("spell_mastery"));
const FName ManaFlowId(TEXT("mana_flow"));
const FName ArcaneOverloadId(TEXT("arcane_overload"));
const FName CriticalEyeId(TEXT("critical_eye"));
const FName QuickDrawId(TEXT("quick_draw"));
const FName EagleEyeId(TEXT("eagle_eye"));
const FName EvasionStanceId(TEXT("evasion_stance"));
const FName NimbleHandsId(TEXT("nimble_hands"));
const FName LuckyInstinctId(TEXT("lucky_instinct"));
const FName AssassinateId(TEXT("assassinate"));
const FName BlessingId(TEXT("blessing"));
const FName PurifyId(TEXT("purify"));
const FName WisdomTrainingId(TEXT("wisdom_training"));
const FName DivineVitalityId(TEXT("divine_vitality"));
const FName SanctuaryId(TEXT("sanctuary"));
const FName GuardianAegisId(TEXT("guardian_aegis"));
const FName SacredOathId(TEXT("sacred_oath"));
const FName BulwarkTrainingId(TEXT("bulwark_training"));
const FName DivineBastionId(TEXT("divine_bastion"));
const FName FrenzyStanceId(TEXT("frenzy_stance"));
const FName BloodFrenzyId(TEXT("blood_frenzy"));
const FName PainToPowerId(TEXT("pain_to_power"));
const FName BerserkApexId(TEXT("berserk_apex"));
const FName ArcaneBindingId(TEXT("arcane_binding"));
const FName PactMasteryId(TEXT("pact_mastery"));
const FName SpiritReservoirId(TEXT("spirit_reservoir"));
const FName GrandFamiliarId(TEXT("grand_familiar"));

FSkillDefinition MakeSkill(
	const TCHAR* SkillId,
	EClassId ClassId,
	const TCHAR* DisplayName,
	ESkillType Type,
	ESkillEffectType EffectType,
	float Cooldown,
	float DamageCoeff,
	float BuffMagnitude,
	float BuffDuration,
	float GaugeGainOnHit = 0.0f,
	float GaugeGainOnTakeDamage = 0.0f,
	ESkillStatusEffect StatusEffect = ESkillStatusEffect::None,
	float StatusDuration = 0.0f,
	float StatusMagnitude = 0.0f,
	ESkillElement Element = ESkillElement::None)
{
	FSkillDefinition Skill;
	Skill.SkillId = FName(SkillId);
	Skill.ClassId = ClassId;
	Skill.DisplayName = IdleProject::Localization::Text(TEXT("Skill"), SkillId);
	Skill.Type = Type;
	Skill.EffectType = EffectType;
	Skill.Cooldown = Cooldown;
	Skill.DamageCoeff = DamageCoeff;
	Skill.BuffMagnitude = BuffMagnitude;
	Skill.BuffDuration = BuffDuration;
	Skill.GaugeGainOnHit = GaugeGainOnHit;
	Skill.GaugeGainOnTakeDamage = GaugeGainOnTakeDamage;
	Skill.StatusEffect = StatusEffect;
	Skill.StatusDuration = StatusDuration;
	Skill.StatusMagnitude = StatusMagnitude;
	Skill.Element = Element;
	return Skill;
}
}

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::LoadDefaultWarriorSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("heavy_strike"), EClassId::Warrior, TEXT("강타"), ESkillType::Active, ESkillEffectType::DamageSingle, 4.0f, 2.5f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("whirlwind"), EClassId::Warrior, TEXT("회전베기"), ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.8f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("shield_up"), EClassId::Warrior, TEXT("방패 올리기"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.5f, 4.0f));
	Skills.Add(MakeSkill(TEXT("charge"), EClassId::Warrior, TEXT("돌진"), ESkillType::Active, ESkillEffectType::DashDamage, 10.0f, 2.0f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("weapon_mastery"), EClassId::Warrior, TEXT("무기 숙련"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("toughness"), EClassId::Warrior, TEXT("강인함"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("berserkers_fury"), EClassId::Warrior, TEXT("광전사의 분노"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 6.0f, 0.3f, 4.0f, 8.0f, 5.0f));
}

void USkillComponent::LoadDefaultMageSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("arcane_bolt"), EClassId::Mage, TEXT("Arcane Bolt"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.0f, 2.4f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Burn, 3.0f, 4.0f, ESkillElement::Fire));
	Skills.Add(MakeSkill(TEXT("chain_lightning"), EClassId::Mage, TEXT("Chain Lightning"), ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.7f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Lightning));
	Skills.Add(MakeSkill(TEXT("mana_shield"), EClassId::Mage, TEXT("Mana Shield"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.35f, 4.0f));
	Skills.Add(MakeSkill(TEXT("meteor"), EClassId::Mage, TEXT("Meteor"), ESkillType::Active, ESkillEffectType::DamageAoe, 14.0f, 2.8f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Freeze, 2.0f, 0.25f, ESkillElement::Ice));
	Skills.Add(MakeSkill(TEXT("spell_mastery"), EClassId::Mage, TEXT("Spell Mastery"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("mana_flow"), EClassId::Mage, TEXT("Mana Flow"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("arcane_overload"), EClassId::Mage, TEXT("Arcane Overload"), ESkillType::Ultimate, ESkillEffectType::DamageAoe, 0.0f, 5.5f, 0.25f, 4.0f, 9.0f, 3.0f));
}

void USkillComponent::LoadDefaultArcherSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("precision_shot"), EClassId::Archer, TEXT("Precision Shot"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.5f, 2.2f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("arrow_rain"), EClassId::Archer, TEXT("Arrow Rain"), ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.6f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("focus"), EClassId::Archer, TEXT("Focus"), ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.2f, 4.0f));
	Skills.Add(MakeSkill(TEXT("piercing_arrow"), EClassId::Archer, TEXT("Piercing Arrow"), ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.0f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("critical_eye"), EClassId::Archer, TEXT("Critical Eye"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f));
	Skills.Add(MakeSkill(TEXT("quick_draw"), EClassId::Archer, TEXT("Quick Draw"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.1f, 0.0f));
	Skills.Add(MakeSkill(TEXT("eagle_eye"), EClassId::Archer, TEXT("Eagle Eye"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 5.0f, 0.25f, 4.0f, 10.0f, 2.0f));
}

void USkillComponent::LoadDefaultThiefSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("shadow_stab"), EClassId::Thief, TEXT("Shadow Stab"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.0f, 2.3f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Poison, 3.0f, 3.0f, ESkillElement::None));
	Skills.Add(MakeSkill(TEXT("smoke_bomb"), EClassId::Thief, TEXT("Smoke Bomb"), ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Poison, 3.0f, 2.0f, ESkillElement::None));
	Skills.Add(MakeSkill(TEXT("evasion_stance"), EClassId::Thief, TEXT("Evasion Stance"), ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.2f, 4.0f));
	Skills.Add(MakeSkill(TEXT("backstab"), EClassId::Thief, TEXT("Backstab"), ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.1f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("nimble_hands"), EClassId::Thief, TEXT("Nimble Hands"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f));
	Skills.Add(MakeSkill(TEXT("lucky_instinct"), EClassId::Thief, TEXT("Lucky Instinct"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f));
	Skills.Add(MakeSkill(TEXT("assassinate"), EClassId::Thief, TEXT("Assassinate"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 5.3f, 0.25f, 4.0f, 11.0f, 1.0f));
}

void USkillComponent::LoadDefaultClericSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("holy_smite"), EClassId::Cleric, TEXT("Holy Smite"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.2f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy));
	Skills.Add(MakeSkill(TEXT("heal"), EClassId::Cleric, TEXT("Heal"), ESkillType::Active, ESkillEffectType::Heal, 6.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("blessing"), EClassId::Cleric, TEXT("Blessing"), ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.15f, 4.0f));
	Skills.Add(MakeSkill(TEXT("purify"), EClassId::Cleric, TEXT("Purify"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.25f, 4.0f));
	Skills.Add(MakeSkill(TEXT("wisdom_training"), EClassId::Cleric, TEXT("Wisdom Training"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.1f, 0.0f));
	Skills.Add(MakeSkill(TEXT("divine_vitality"), EClassId::Cleric, TEXT("Divine Vitality"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("sanctuary"), EClassId::Cleric, TEXT("Sanctuary"), ESkillType::Ultimate, ESkillEffectType::Heal, 0.0f, 0.0f, 0.4f, 0.0f, 6.0f, 6.0f));
}

void USkillComponent::LoadDefaultPaladinSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("holy_verdict"), EClassId::Paladin, TEXT("Holy Verdict"), ESkillType::Active, ESkillEffectType::DamageSingle, 4.0f, 2.3f, 0.0f, 0.0f, 1.0f, 1.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy));
	Skills.Add(MakeSkill(TEXT("radiant_sweep"), EClassId::Paladin, TEXT("Radiant Sweep"), ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.6f, 0.0f, 0.0f, 1.0f, 1.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy));
	Skills.Add(MakeSkill(TEXT("guardian_aegis"), EClassId::Paladin, TEXT("Guardian Aegis"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.45f, 5.0f, 0.0f, 2.0f));
	Skills.Add(MakeSkill(TEXT("lay_on_hands"), EClassId::Paladin, TEXT("Lay on Hands"), ESkillType::Active, ESkillEffectType::Heal, 10.0f, 0.0f, 0.18f, 0.0f));
	Skills.Add(MakeSkill(TEXT("sacred_oath"), EClassId::Paladin, TEXT("Sacred Oath"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("bulwark_training"), EClassId::Paladin, TEXT("Bulwark Training"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("divine_bastion"), EClassId::Paladin, TEXT("Divine Bastion"), ESkillType::Ultimate, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.35f, 5.0f, 5.0f, 8.0f));
}

void USkillComponent::LoadDefaultBerserkerSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("rage_cleave"), EClassId::Berserker, TEXT("Rage Cleave"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.5f, 2.35f, 0.0f, 0.0f, 2.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("blood_surge"), EClassId::Berserker, TEXT("Blood Surge"), ESkillType::Active, ESkillEffectType::DamageAoe, 7.5f, 1.65f, 0.0f, 0.0f, 2.0f, 0.0f, ESkillStatusEffect::Burn, 2.0f, 3.0f, ESkillElement::Fire));
	Skills.Add(MakeSkill(TEXT("frenzy_stance"), EClassId::Berserker, TEXT("Frenzy Stance"), ESkillType::Active, ESkillEffectType::SelfBuff, 11.0f, 0.0f, 0.3f, 4.0f));
	Skills.Add(MakeSkill(TEXT("savage_leap"), EClassId::Berserker, TEXT("Savage Leap"), ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.05f, 0.0f, 0.0f, 2.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("blood_frenzy"), EClassId::Berserker, TEXT("Blood Frenzy"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("pain_to_power"), EClassId::Berserker, TEXT("Pain to Power"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.08f, 0.0f));
	Skills.Add(MakeSkill(TEXT("berserk_apex"), EClassId::Berserker, TEXT("Berserk Apex"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 6.5f, 0.35f, 4.0f, 12.0f, 2.0f));
}

void USkillComponent::LoadDefaultSummonerSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("spirit_bolt"), EClassId::Summoner, TEXT("Spirit Bolt"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.2f, 1.9f, 0.0f, 0.0f, 1.0f, 0.0f, ESkillStatusEffect::Poison, 3.0f, 2.5f, ESkillElement::None));
	Skills.Add(MakeSkill(TEXT("familiar_swarm"), EClassId::Summoner, TEXT("Familiar Swarm"), ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.45f, 0.0f, 0.0f, 1.0f, 0.0f, ESkillStatusEffect::Poison, 4.0f, 2.0f, ESkillElement::None));
	Skills.Add(MakeSkill(TEXT("arcane_binding"), EClassId::Summoner, TEXT("Arcane Binding"), ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.22f, 4.0f));
	Skills.Add(MakeSkill(TEXT("void_call"), EClassId::Summoner, TEXT("Void Call"), ESkillType::Active, ESkillEffectType::DamageAoe, 12.0f, 2.0f, 0.0f, 0.0f, 1.5f, 0.0f, ESkillStatusEffect::Freeze, 2.0f, 0.2f, ESkillElement::Ice));
	Skills.Add(MakeSkill(TEXT("pact_mastery"), EClassId::Summoner, TEXT("Pact Mastery"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("spirit_reservoir"), EClassId::Summoner, TEXT("Spirit Reservoir"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("grand_familiar"), EClassId::Summoner, TEXT("Grand Familiar"), ESkillType::Ultimate, ESkillEffectType::DamageAoe, 0.0f, 5.7f, 0.25f, 4.0f, 10.0f, 3.0f, ESkillStatusEffect::Poison, 5.0f, 4.0f, ESkillElement::Lightning));
}

void USkillComponent::LoadSkillsForClass(EClassId ClassId)
{
	switch (ClassId)
	{
	case EClassId::Mage:
		LoadDefaultMageSkills();
		break;
	case EClassId::Archer:
		LoadDefaultArcherSkills();
		break;
	case EClassId::Thief:
		LoadDefaultThiefSkills();
		break;
	case EClassId::Cleric:
		LoadDefaultClericSkills();
		break;
	case EClassId::Paladin:
		LoadDefaultPaladinSkills();
		break;
	case EClassId::Berserker:
		LoadDefaultBerserkerSkills();
		break;
	case EClassId::Summoner:
		LoadDefaultSummonerSkills();
		break;
	case EClassId::Warrior:
	default:
		LoadDefaultWarriorSkills();
		break;
	}
}

void USkillComponent::TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets)
{
	UpdateTimedBuffs(Now);

	for (const FSkillDefinition& Skill : Skills)
	{
		if (Skill.Type == ESkillType::Ultimate && IsUltimateReady())
		{
			ExecuteSkill(Skill, Now, Target, AoeTargets);
			return;
		}
	}

	for (const FSkillDefinition& Skill : Skills)
	{
		if (Skill.Type == ESkillType::Active && IsReady(Skill.SkillId, Now))
		{
			ExecuteSkill(Skill, Now, Target, AoeTargets);
		}
	}
}

bool USkillComponent::IsReady(FName SkillId, float Now) const
{
	return FindSkill(SkillId) && GetCooldownRemaining(SkillId, Now) <= 0.0f;
}

float USkillComponent::GetCooldownRemaining(FName SkillId, float Now) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill)
	{
		return 0.0f;
	}

	const float* LastCastTime = LastCastTimeBySkill.Find(SkillId);
	if (!LastCastTime)
	{
		return 0.0f;
	}

	const float EffectiveCooldown = GetEffectiveCooldown(SkillId);
	return FMath::Max(0.0f, EffectiveCooldown - (Now - *LastCastTime));
}

float USkillComponent::GetCooldownRatio(FName SkillId, float Now) const
{
	const float EffectiveCooldown = GetEffectiveCooldown(SkillId);
	if (EffectiveCooldown <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetCooldownRemaining(SkillId, Now) / EffectiveCooldown, 0.0f, 1.0f);
}

void USkillComponent::GrantSkillPoint(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	SkillPoints += Amount;
}

int32 USkillComponent::GetSkillPoints() const
{
	return SkillPoints;
}

int32 USkillComponent::GetSkillRank(FName SkillId) const
{
	const int32* Rank = SkillRanks.Find(SkillId);
	return Rank ? FMath::Clamp(*Rank, 0, MaxRank) : 0;
}

bool USkillComponent::CanRankUp(FName SkillId) const
{
	return SkillPoints > 0 && FindSkill(SkillId) && GetSkillRank(SkillId) < MaxRank;
}

bool USkillComponent::RankUpSkill(FName SkillId)
{
	if (!CanRankUp(SkillId))
	{
		return false;
	}

	SkillRanks.FindOrAdd(SkillId) = GetSkillRank(SkillId) + 1;
	--SkillPoints;
	return true;
}

float USkillComponent::GetEffectiveDamageCoeff(FName SkillId) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill)
	{
		return 0.0f;
	}

	return Skill->DamageCoeff * (1.0f + static_cast<float>(GetSkillRank(SkillId)) * 0.1f);
}

float USkillComponent::GetEffectiveCooldown(FName SkillId) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill || Skill->Cooldown <= 0.0f)
	{
		return 0.0f;
	}

	const float ReducedCooldown = Skill->Cooldown * (1.0f - static_cast<float>(GetSkillRank(SkillId)) * 0.05f);
	return FMath::Max(0.1f, ReducedCooldown);
}

void USkillComponent::CaptureRankState(TMap<FName, int32>& OutRanks, int32& OutPoints) const
{
	OutRanks = SkillRanks;
	OutPoints = SkillPoints;
}

void USkillComponent::RestoreRankState(const TMap<FName, int32>& InRanks, int32 InPoints)
{
	SkillRanks.Reset();
	for (const TPair<FName, int32>& Pair : InRanks)
	{
		if (!FindSkill(Pair.Key))
		{
			continue;
		}

		const int32 ClampedRank = FMath::Clamp(Pair.Value, 0, MaxRank);
		if (ClampedRank > 0)
		{
			SkillRanks.Add(Pair.Key, ClampedRank);
		}
	}

	SkillPoints = FMath::Max(0, InPoints);
}

bool USkillComponent::MarkSkillCast(FName SkillId, float Now)
{
	if (!FindSkill(SkillId))
	{
		return false;
	}

	LastCastTimeBySkill.Add(SkillId, Now);
	return true;
}

void USkillComponent::AddGauge(float Amount)
{
	CurrentGauge = FMath::Clamp(CurrentGauge + Amount, 0.0f, 100.0f);
}

float USkillComponent::GetCurrentGauge() const
{
	return CurrentGauge;
}

bool USkillComponent::IsUltimateReady() const
{
	return CurrentGauge >= 100.0f;
}

bool USkillComponent::TryConsumeUltimateGauge()
{
	if (!IsUltimateReady())
	{
		return false;
	}

	CurrentGauge = 0.0f;
	return true;
}

void USkillComponent::ResetSkillState()
{
	Skills.Reset();
	LastCastTimeBySkill.Reset();
	CurrentGauge = 0.0f;
	LastAppliedDefBonus = 0.0f;
	DefBuffMagnitude = 0.0f;
	DefBuffEndTime = 0.0f;
	LastAppliedAtkSpeedBonus = 0.0f;
	AtkSpeedBuffMagnitude = 0.0f;
	AtkSpeedBuffEndTime = 0.0f;
}

void USkillComponent::ApplyPassivesToStats(FDerivedStats& InOutStats) const
{
	if (FindSkill(WeaponMasteryId))
	{
		InOutStats.PhysAtk = FMath::RoundToFloat(InOutStats.PhysAtk * 1.15f);
	}
	if (FindSkill(ToughnessId))
	{
		InOutStats.Hp = FMath::RoundToFloat(InOutStats.Hp * 1.2f);
	}
	if (FindSkill(SpellMasteryId))
	{
		InOutStats.MagicAtk = FMath::RoundToFloat(InOutStats.MagicAtk * 1.15f);
	}
	if (FindSkill(ManaFlowId))
	{
		InOutStats.Mp = FMath::RoundToFloat(InOutStats.Mp * 1.2f);
	}
	if (FindSkill(CriticalEyeId))
	{
		InOutStats.CritRate = FMath::Clamp(InOutStats.CritRate + 0.05f, 0.0f, 1.0f);
	}
	if (FindSkill(QuickDrawId))
	{
		InOutStats.AtkSpeed = FMath::RoundToFloat(InOutStats.AtkSpeed * 1.1f * 10.0f) / 10.0f;
	}
	if (FindSkill(NimbleHandsId))
	{
		InOutStats.Dodge = FMath::Clamp(InOutStats.Dodge + 0.05f, 0.0f, 1.0f);
	}
	if (FindSkill(LuckyInstinctId))
	{
		InOutStats.CritRate = FMath::Clamp(InOutStats.CritRate + 0.05f, 0.0f, 1.0f);
	}
	if (FindSkill(WisdomTrainingId))
	{
		InOutStats.MagicAtk = FMath::RoundToFloat(InOutStats.MagicAtk * 1.1f);
	}
	if (FindSkill(DivineVitalityId))
	{
		InOutStats.Hp = FMath::RoundToFloat(InOutStats.Hp * 1.2f);
	}
	if (FindSkill(SacredOathId))
	{
		InOutStats.Hp = FMath::RoundToFloat(InOutStats.Hp * 1.15f);
	}
	if (FindSkill(BulwarkTrainingId))
	{
		InOutStats.PhysDef = FMath::RoundToFloat(InOutStats.PhysDef * 1.15f);
	}
	if (FindSkill(BloodFrenzyId))
	{
		InOutStats.PhysAtk = FMath::RoundToFloat(InOutStats.PhysAtk * 1.2f);
	}
	if (FindSkill(PainToPowerId))
	{
		InOutStats.CritRate = FMath::Clamp(InOutStats.CritRate + 0.08f, 0.0f, 1.0f);
	}
	if (FindSkill(PactMasteryId))
	{
		InOutStats.MagicAtk = FMath::RoundToFloat(InOutStats.MagicAtk * 1.15f);
	}
	if (FindSkill(SpiritReservoirId))
	{
		InOutStats.Mp = FMath::RoundToFloat(InOutStats.Mp * 1.2f);
	}
}

float USkillComponent::GetGaugeGainOnHit() const
{
	const FSkillDefinition* Ultimate = FindSkill(BerserkersFuryId);
	if (!Ultimate)
	{
		Ultimate = FindSkill(ArcaneOverloadId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(EagleEyeId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(AssassinateId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(SanctuaryId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(DivineBastionId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(BerserkApexId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(GrandFamiliarId);
	}
	return Ultimate ? Ultimate->GaugeGainOnHit : 0.0f;
}

float USkillComponent::GetGaugeGainOnTakeDamage() const
{
	const FSkillDefinition* Ultimate = FindSkill(BerserkersFuryId);
	if (!Ultimate)
	{
		Ultimate = FindSkill(ArcaneOverloadId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(EagleEyeId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(AssassinateId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(SanctuaryId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(DivineBastionId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(BerserkApexId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(GrandFamiliarId);
	}
	return Ultimate ? Ultimate->GaugeGainOnTakeDamage : 0.0f;
}

const FSkillDefinition* USkillComponent::FindSkill(FName SkillId) const
{
	return Skills.FindByPredicate([SkillId](const FSkillDefinition& Skill)
	{
		return Skill.SkillId == SkillId;
	});
}

void USkillComponent::UpdateTimedBuffs(float Now)
{
	UCombatComponent* Combat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	if (!Combat)
	{
		return;
	}

	if (LastAppliedDefBonus != 0.0f)
	{
		Combat->Def = FMath::Max(0.0f, Combat->Def - LastAppliedDefBonus);
		LastAppliedDefBonus = 0.0f;
	}
	if (LastAppliedAtkSpeedBonus != 0.0f)
	{
		Combat->AtkSpeed = FMath::Max(0.1f, Combat->AtkSpeed - LastAppliedAtkSpeedBonus);
		LastAppliedAtkSpeedBonus = 0.0f;
	}

	if (DefBuffEndTime > Now)
	{
		LastAppliedDefBonus = Combat->Def * DefBuffMagnitude;
		Combat->Def += LastAppliedDefBonus;
	}
	if (AtkSpeedBuffEndTime > Now)
	{
		LastAppliedAtkSpeedBonus = Combat->AtkSpeed * AtkSpeedBuffMagnitude;
		Combat->AtkSpeed += LastAppliedAtkSpeedBonus;
	}
}

void USkillComponent::ExecuteSkill(const FSkillDefinition& Skill, float Now, AActor* Target, const TArray<AActor*>& AoeTargets)
{
	if (Skill.Type == ESkillType::Ultimate && !TryConsumeUltimateGauge())
	{
		return;
	}

	if (Skill.EffectType == ESkillEffectType::SelfBuff)
	{
		ApplySelfBuff(Skill, Now);
	}
	else if (Skill.EffectType == ESkillEffectType::Heal)
	{
		ApplyHeal(Skill);
	}
	else if (Skill.EffectType == ESkillEffectType::DamageAoe)
	{
		for (AActor* AoeTarget : AoeTargets)
		{
			ApplyDamageSkill(Skill, AoeTarget);
		}
	}
	else
	{
		ApplyDamageSkill(Skill, Target);
	}
	if (Skill.Type == ESkillType::Ultimate && Skill.BuffMagnitude > 0.0f)
	{
		ApplySelfBuff(Skill, Now);
	}

	MarkSkillCast(Skill.SkillId, Now);
}

void USkillComponent::ApplyDamageSkill(const FSkillDefinition& Skill, AActor* Target)
{
	UCombatComponent* OwnerCombat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	UCombatComponent* TargetCombat = Target ? Target->FindComponentByClass<UCombatComponent>() : nullptr;
	if (!OwnerCombat || !TargetCombat || TargetCombat->IsDead())
	{
		return;
	}

	const bool bMagicDamage = Skill.ClassId == EClassId::Mage || Skill.ClassId == EClassId::Cleric || Skill.ClassId == EClassId::Summoner;
	const float AttackPower = bMagicDamage ? OwnerCombat->MagicAtk : OwnerCombat->Atk;
	const float Defense = bMagicDamage ? TargetCombat->MagicDef : TargetCombat->Def;
	const float EffectiveDamageCoeff = GetEffectiveDamageCoeff(Skill.SkillId);
	const float BaseDamage = bMagicDamage
		? FCombatFormulas::ComputeMagicDamage(AttackPower * EffectiveDamageCoeff, Defense)
		: FCombatFormulas::ComputeDamage(AttackPower * EffectiveDamageCoeff, Defense);
	const bool bWasCrit = OwnerCombat->RollCrit();
	const ESkillElement TargetWeakElement = Target && Target->IsA<AIdleMonster>()
		? CastChecked<AIdleMonster>(Target)->GetWeakElement()
		: ESkillElement::None;
	const float ElementDamage = BaseDamage * FCombatFormulas::ComputeElementMultiplier(Skill.Element, TargetWeakElement);
	const float FinalDamage = FCombatFormulas::ApplyCrit(ElementDamage, bWasCrit, OwnerCombat->CritDmg);
	const EDamageKind DamageKind = bMagicDamage ? EDamageKind::Magic : EDamageKind::Physical;

	TargetCombat->TakeDamageTyped(FinalDamage, GetOwner(), bWasCrit, DamageKind);
	if (Skill.StatusEffect != ESkillStatusEffect::None)
	{
		TargetCombat->ApplyStatus(Skill.StatusEffect, Skill.StatusDuration, Skill.StatusMagnitude, GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
	}
	if (Skill.Type != ESkillType::Ultimate)
	{
		AddGauge(Skill.GaugeGainOnHit);
	}
}

void USkillComponent::ApplyHeal(const FSkillDefinition& Skill)
{
	UCombatComponent* Combat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	if (!Combat || Combat->IsDead())
	{
		return;
	}

	const float HealAmount = FMath::RoundToFloat(Combat->MaxHp * FMath::Max(0.0f, Skill.BuffMagnitude));
	Combat->CurrentHp = FMath::Clamp(Combat->CurrentHp + HealAmount, 0.0f, Combat->MaxHp);
	Combat->OnHpChanged.Broadcast(Combat->CurrentHp);
}

void USkillComponent::ApplySelfBuff(const FSkillDefinition& Skill, float Now)
{
	if (Skill.SkillId == ShieldUpId || Skill.SkillId == ManaShieldId || Skill.SkillId == PurifyId || Skill.SkillId == GuardianAegisId || Skill.SkillId == DivineBastionId)
	{
		DefBuffMagnitude = Skill.BuffMagnitude;
		DefBuffEndTime = Now + Skill.BuffDuration;
	}
	else if (
		Skill.SkillId == BerserkersFuryId ||
		Skill.SkillId == ArcaneOverloadId ||
		Skill.SkillId == FocusId ||
		Skill.SkillId == EagleEyeId ||
		Skill.SkillId == EvasionStanceId ||
		Skill.SkillId == AssassinateId ||
		Skill.SkillId == BlessingId ||
		Skill.SkillId == FrenzyStanceId ||
		Skill.SkillId == BerserkApexId ||
		Skill.SkillId == ArcaneBindingId ||
		Skill.SkillId == GrandFamiliarId)
	{
		AtkSpeedBuffMagnitude = Skill.BuffMagnitude;
		AtkSpeedBuffEndTime = Now + Skill.BuffDuration;
	}

	UpdateTimedBuffs(Now);
}
