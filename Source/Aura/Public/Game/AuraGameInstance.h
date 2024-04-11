// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AuraGameInstance.generated.h"

/**
 * �𸮾� ���������� GameInstance�� ������ ���� �߿� ���ӵǴ� �����ͳ� ���¸� �����ϱ� ���� Ŭ�����Դϴ�. 
 * ������ ������ �ϳ��� GameInstance�� ������, �̴� ������ ����Ǵ� ���� �����˴ϴ�.
 * GameInstance�� ������ ������ �����ϴ�:
 * ���� ���� ����: ������ �������� ���¸� �����ϰ� �����մϴ�. ���� ���, �÷��̾��� ����, ���� ����, �÷��̾��� �κ��丮 �� ���� ���ݿ� ���� �����Ǿ�� �ϴ� �����͸� �����մϴ�.
 * ���� �� ������ ����: ���� ���� ���̿��� �����͸� �����ϰų� �����ϴ� �� ���˴ϴ�. ���� ���, �� �������� ȹ���� �������� ���� �������� ����Ϸ��� GameInstance�� ���� �����͸� ������ �� �ֽ��ϴ�.
 * ���� ���� �ʱ�ȭ: ������ ���۵� �� �ʿ��� �ʱ�ȭ �۾��� �����մϴ�. ���� ���, ���� ���� �ε�, �÷��̾� ������ �ε� ���� �۾��� ������ �� �ֽ��ϴ�.
 * �¶��� ��� ����: ���ӿ��� �¶��� ����� ������ �� ���� �� �ֽ��ϴ�. ���� ���, �¶��� �÷��� ������ �����ϰų� �÷��̾� ���� ����� �����ϴ� �� ���� �� �ֽ��ϴ�.
 */
UCLASS()
class AURA_API UAuraGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UPROPERTY()
	FName PlayerStartTag = FName();

	UPROPERTY()
	FString LoadSlotName = FString();

	UPROPERTY()
	int32 LoadSlotIndex = 0;
};
