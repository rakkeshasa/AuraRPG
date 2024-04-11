// Chocolate Maniac

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AuraGameInstance.generated.h"

/**
 * 언리얼 엔진에서의 GameInstance는 게임의 실행 중에 지속되는 데이터나 상태를 관리하기 위한 클래스입니다. 
 * 각각의 게임은 하나의 GameInstance를 가지며, 이는 게임이 실행되는 동안 유지됩니다.
 * GameInstance의 역할은 다음과 같습니다:
 * 전역 상태 유지: 게임의 전역적인 상태를 유지하고 관리합니다. 예를 들어, 플레이어의 점수, 게임 설정, 플레이어의 인벤토리 등 게임 전반에 걸쳐 유지되어야 하는 데이터를 저장합니다.
 * 레벨 간 데이터 전달: 여러 레벨 사이에서 데이터를 전달하거나 공유하는 데 사용됩니다. 예를 들어, 한 레벨에서 획득한 아이템을 다음 레벨에서 사용하려면 GameInstance를 통해 데이터를 전달할 수 있습니다.
 * 최초 실행 초기화: 게임이 시작될 때 필요한 초기화 작업을 수행합니다. 예를 들어, 게임 설정 로드, 플레이어 데이터 로드 등의 작업을 수행할 수 있습니다.
 * 온라인 기능 관리: 게임에서 온라인 기능을 구현할 때 사용될 수 있습니다. 예를 들어, 온라인 플레이 세션을 관리하거나 플레이어 간의 통신을 조정하는 데 사용될 수 있습니다.
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
