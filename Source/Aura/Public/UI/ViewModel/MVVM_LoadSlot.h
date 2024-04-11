// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/*
 * 위젯 스위처에 세 개의 자식 중 인덱스별로 전환할 위젯을 어떻게든 알려줘야 함
 * 위젯 스위처에는 이 LoadSlot 뷰모델로 설정된 뷰모델이 없지만, 
 * 자식에 대한 뷰모델을 설정할 시점에 액세스할 수 있다
 * 그 시점에 델리게이트를 바인드 할 수 있다
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
	*  필드 알림은 이 뷰 모델을 사용하여 위젯과 연결할 수 있는 
	*  무언가를 만드는 방법이다.
	*/

	// 플레이어를 직접 설정하고 싶지 않기 때문에 Set와 Get가 중요하다
	void SetPlayerName(FString InPlayerName);
	void SetMapName(FString InMapName);
	void SetPlayerLevel(int32 InLevel);

	FString GetPlayerName() const { return PlayerName; }
	FString GetMapName() const { return MapName; }
	int32 GetPlayerLevel() const { return PlayerLevel; }

private:

	// Field Notifies가 되려면 이 EditAnywhere, BlueprintReadWrite도 만들어야 합니다.
	// Field Notifies를 변경하고 BP측에서 정보를 가져오려면 
	// Field Notifies에 대한 읽기 쓰기 권한이 필요하기 때문
	// meta = (AllowPrivateAccess = "true")를 쓰면 ReadWrite여도 private에서 선언가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"));
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"));
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"));
	int32 PlayerLevel;
};
