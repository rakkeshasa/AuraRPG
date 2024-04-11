// Chocolate Maniac

#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 셀렉터를 복합 노드라고 부르며, 특정 유형의 노드를 복합 노드 및 작업에 연결할 수 있습니다. 
	// 셀렉터에 연결하려는 이러한 유형의 노드 중 하나를 서비스라고 합니다.
	// C++로 커스텀 서비스 만들기
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Owner은 BTService_BlueprintBase.h에서 제공하는 함수로 가져올 수 있다.
	// GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, *AIOwner->GetName());
	// GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, *ActorOwner->GetName());
	
	APawn* OwningPawn = AIOwner->GetPawn();

	// BP_EnemyBase와 BP_AuraCharacter에 따로 태그를 작성해줬다.
	// 서비스의 소유자가 플레이어면 TargetTag는 Enemy
	// 서비스의 소유자가 Enemy이면 TargetTag는 플레이어
	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");

	// 월드에서 해당 TargetTag를 가진 Actor를 찾는다
	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);

	float ClosestDistance = TNumericLimits<float>::Max(); // Float 최대치
	AActor* ClosestActor = nullptr;

	for (AActor* Actor : ActorsWithTag)
	{
		// GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Orange, *Actor->GetName());

		if (IsValid(Actor) && IsValid(OwningPawn))
		{
			const float Distance = OwningPawn->GetDistanceTo(Actor);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = Actor;
			}
		}
	}

	// 블랙보드 키에 세팅하기
	UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
