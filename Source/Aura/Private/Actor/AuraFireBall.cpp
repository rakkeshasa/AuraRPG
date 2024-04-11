// Chocolate Maniac

#include "Actor/AuraFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include <AuraGameplayTags.h>

void AAuraFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

void AAuraFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;

			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			// 데미지 주기
			// AuraAbilitySystemLibrary에는 damage 외에도 호출자 크기에 따라 설정된 모든 debuff 파라미터가 있다
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

void AAuraFireBall::OnHit()
{
	// 해당 FireBall은 FireBlast 스킬에 나오는 파이어볼을 담당(chp. 374)
	// 현재 프로젝트는 업데이트 당 RPC제한을 10개로 두고 있는데 스킬은 12개의 파이어볼이 나간다.
	// 그럼...RPC를 아껴야겠죠?
	// RPC를 아끼기 위해 RPC를 보내지 않고 로컬에서 게임플레이 큐를 사용하는 방법이 있을까요?
	// 게임플레이 큐를 사용하기 위해서는 Gameplay Cue 태그 별로 실행되기 때문에 Tag를 알아야한다.
	// 해당 태그는 AuraGameplayTags.h에 생성함
	// GameplayCueManager에서는 로컬에서 게임플레이큐를 실행할 수 있는 함수를 제공

	// DamageEffectParams는 파이어볼이 스폰될 때 서버에서 설정, 클라에서는 설정하지 않는다.
	// 따라서 로컬에서는 쓸 수 없으므로 따로 Target을 설정해줘야한다.
	// 모든 액터의 소유자는 리플리케이트되므로 FireBlast에서 SetOwner세팅
	// DamageEffectParams.TargetAbilitySystemComponent;

	// 레플리케이션이 일어나기 전에 OnHit()가 호출될 경우를 대비하여 해당 소유자가 유효한지 체크
	if (GetOwner())
	{
		// 버스트 노티파이가 CueParameter의 위치에 파티클을 스폰하므로 
		// CueParams.Location를 설정하면 Fireball 안에 들어간다.
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();

		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
			GetOwner()/*Target*/, 
			FAuraGameplayTags::Get().GameplayCue_FireBlast/*Cue Tag*/,
			CueParams);

		// 이젠 게임플레이 큐가 어떤 RPC도 발생 안시킨다.
	}

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}

	bHit = true;
}
