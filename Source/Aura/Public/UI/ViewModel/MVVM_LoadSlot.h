// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/*
 * ���� ����ó�� �� ���� �ڽ� �� �ε������� ��ȯ�� ������ ��Ե� �˷���� ��
 * ���� ����ó���� �� LoadSlot ��𵨷� ������ ����� ������, 
 * �ڽĿ� ���� ����� ������ ������ �׼����� �� �ִ�
 * �� ������ ��������Ʈ�� ���ε� �� �� �ִ�
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	void InitializeSlot();

	UPROPERTY()
	FString LoadSlotName;

	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	UPROPERTY()
	FName PlayerStartTag;

	UPROPERTY()
	FString MapAssetName;

	/* Field Notifies
	*  �ʵ� �˸��� �� �� ���� ����Ͽ� ������ ������ �� �ִ� 
	*  ���𰡸� ����� ����̴�.
	*/

	// �÷��̾ ���� �����ϰ� ���� �ʱ� ������ Set�� Get�� �߿��ϴ�
	void SetPlayerName(FString InPlayerName);
	void SetMapName(FString InMapName);
	void SetPlayerLevel(int32 InLevel);

	FString GetPlayerName() const { return PlayerName; }
	FString GetMapName() const { return MapName; }
	int32 GetPlayerLevel() const { return PlayerLevel; }

private:

	// Field Notifies�� �Ƿ��� �� EditAnywhere, BlueprintReadWrite�� ������ �մϴ�.
	// Field Notifies�� �����ϰ� BP������ ������ ���������� 
	// Field Notifies�� ���� �б� ���� ������ �ʿ��ϱ� ����
	// meta = (AllowPrivateAccess = "true")�� ���� ReadWrite���� private���� ���𰡴�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"));
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"));
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"));
	int32 PlayerLevel;
};
