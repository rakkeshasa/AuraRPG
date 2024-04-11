// Chocolate Maniac

#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// �����͸� ���� ����� �θ���, Ư�� ������ ��带 ���� ��� �� �۾��� ������ �� �ֽ��ϴ�. 
	// �����Ϳ� �����Ϸ��� �̷��� ������ ��� �� �ϳ��� ���񽺶�� �մϴ�.
	// C++�� Ŀ���� ���� �����
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Owner�� BTService_BlueprintBase.h���� �����ϴ� �Լ��� ������ �� �ִ�.
	// GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, *AIOwner->GetName());
	// GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, *ActorOwner->GetName());
	
	APawn* OwningPawn = AIOwner->GetPawn();

	// BP_EnemyBase�� BP_AuraCharacter�� ���� �±׸� �ۼ������.
	// ������ �����ڰ� �÷��̾�� TargetTag�� Enemy
	// ������ �����ڰ� Enemy�̸� TargetTag�� �÷��̾�
	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");

	// ���忡�� �ش� TargetTag�� ���� Actor�� ã�´�
	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);

	float ClosestDistance = TNumericLimits<float>::Max(); // Float �ִ�ġ
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

	// ������ Ű�� �����ϱ�
	UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
