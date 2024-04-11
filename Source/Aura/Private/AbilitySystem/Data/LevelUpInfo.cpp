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
		// 0�� �ƹ��͵� �ƴѰɷ� �ϰڴ�.

		// �迭�� �ش��ϴ� ���� ��Ұ� �ִ��� Ȯ��
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			// ���� XP�� 400
			// ���� 1�� �� ����ġ ���� 300�̸�
			// ���� ��
			++Level;
		}
		else
		{
			// ����ġ �뺸�� ���� XP�� ���ٸ� while�� Ż��
			bSearching = false;
		}
	}
	return Level;
}
