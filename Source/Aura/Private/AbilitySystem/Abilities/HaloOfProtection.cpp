
#include "AbilitySystem/Abilities/HaloOfProtection.h"

FString UHaloOfProtection::GetDescription(int32 Level)
{
	const float Resist = FMath::Clamp(Level * 4.0f, 0.0f, 40.0f);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>HALO OF PROTECTION</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>Element Registant: </><ManaCost>%.1f</>\n\n"

			"<Default>Get resistant of Fire, Electrocue, Arcane </>"),

			// Values
			Level,
			Resist);
	}

	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>LIFE SIPHON</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>Element Registant: </><ManaCost>%.1f</>\n"),

			// Values
			Level,
			Resist);
	}
}

FString UHaloOfProtection::GetNextLevelDescription(int32 Level)
{
	const float Resist = FMath::Clamp(Level * 4.0f, 0.0f, 40.0f);

	return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL:</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>Element Registant: </><ManaCost>%.1f</>\n\n"),

		// Values
		Level,
		Resist);
}
