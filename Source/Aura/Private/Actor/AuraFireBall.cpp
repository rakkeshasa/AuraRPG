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
			// ������ �ֱ�
			// AuraAbilitySystemLibrary���� damage �ܿ��� ȣ���� ũ�⿡ ���� ������ ��� debuff �Ķ���Ͱ� �ִ�
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

void AAuraFireBall::OnHit()
{
	// �ش� FireBall�� FireBlast ��ų�� ������ ���̾�� ���(chp. 374)
	// ���� ������Ʈ�� ������Ʈ �� RPC������ 10���� �ΰ� �ִµ� ��ų�� 12���� ���̾�� ������.
	// �׷�...RPC�� �Ʋ��߰���?
	// RPC�� �Ƴ��� ���� RPC�� ������ �ʰ� ���ÿ��� �����÷��� ť�� ����ϴ� ����� �������?
	// �����÷��� ť�� ����ϱ� ���ؼ��� Gameplay Cue �±� ���� ����Ǳ� ������ Tag�� �˾ƾ��Ѵ�.
	// �ش� �±״� AuraGameplayTags.h�� ������
	// GameplayCueManager������ ���ÿ��� �����÷���ť�� ������ �� �ִ� �Լ��� ����

	// DamageEffectParams�� ���̾�� ������ �� �������� ����, Ŭ�󿡼��� �������� �ʴ´�.
	// ���� ���ÿ����� �� �� �����Ƿ� ���� Target�� ����������Ѵ�.
	// ��� ������ �����ڴ� ���ø�����Ʈ�ǹǷ� FireBlast���� SetOwner����
	// DamageEffectParams.TargetAbilitySystemComponent;

	// ���ø����̼��� �Ͼ�� ���� OnHit()�� ȣ��� ��츦 ����Ͽ� �ش� �����ڰ� ��ȿ���� üũ
	if (GetOwner())
	{
		// ����Ʈ ��Ƽ���̰� CueParameter�� ��ġ�� ��ƼŬ�� �����ϹǷ� 
		// CueParams.Location�� �����ϸ� Fireball �ȿ� ����.
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();

		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
			GetOwner()/*Target*/, 
			FAuraGameplayTags::Get().GameplayCue_FireBlast/*Cue Tag*/,
			CueParams);

		// ���� �����÷��� ť�� � RPC�� �߻� �Ƚ�Ų��.
	}

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}

	bHit = true;
}
