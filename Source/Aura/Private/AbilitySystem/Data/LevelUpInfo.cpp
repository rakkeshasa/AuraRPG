// Chocolate Maniac


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		// LevelUpInformation[1] = Level 1 Information
		// LevelUpInformation[2] = Level 2 Information
		// 0는 아무것도 아닌걸로 하겠다.

		// 배열에 해당하는 레벨 요소가 있는지 확인
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			// 현재 XP가 400
			// 레벨 1일 때 경험치 통이 300이면
			// 레벨 업
			++Level;
		}
		else
		{
			// 경험치 통보다 현재 XP가 적다면 while문 탈출
			bSearching = false;
		}
	}
	return Level;
}
