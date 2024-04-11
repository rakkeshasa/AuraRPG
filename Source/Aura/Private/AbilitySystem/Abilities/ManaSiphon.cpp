
#include "AbilitySystem/Abilities/ManaSiphon.h"

FString UManaSiphon::GetDescription(int32 Level)
{
	const float Regen = FMath::Clamp((Level / 10.0f) * 5, 0.0f, 10.0f);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>MANA SIPHON</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaRegen: </><ManaCost>%.1f</>\n\n"

			"<Default>Mana regen per second </>"),

			// Values
			Level,
			Regen);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>MANA SIPHON</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaRegen: </><ManaCost>%.1f</>\n\n"),

			// Values
			Level,
			Regen);
	}
}

FString UManaSiphon::GetNextLevelDescription(int32 Level)
{
	const float Regen = FMath::Clamp((Level / 10.0f) * 5, 0.0f, 10.0f);

	return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL:</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaRegen: </><ManaCost>%.1f</>\n\n"),

		// Values
		Level,
		Regen);
}