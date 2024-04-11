// Chocolate Maniac

#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Aura/Public/AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	

	// �������Ʈ �� �̺�Ʈ �׷������� ����ϴ� PrintString�� �ٷ� �̰�
	// UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (C++)"), true, true, FLinearColor::Yellow, 3);
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	// ������ �ִ���, ���� �� true
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// �𸮾� ���������� Execute_ �Լ��� ����Ͽ� ����Ƽ�� �̺�Ʈ�� ȣ��
	// Execute_ �Լ��� ����Ƽ�� �̺�Ʈ�� ȣ���� �� ���Ǵ� ���� �Լ�
	// Weapon Socket�� ã�´�
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	// ���� ���Ͽ� ��ġ ����
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	// SpawnActorDeferred -> BeginPlay ������ ��� ���Ϳ� �߰� �۾�
	// FinishSpawning�� ȣ�����ֳ� �����ֳ��� ����
	// �� spawn�� 2�ܰ�� �����°�?
	// ��ȯ�ϰ� ��ȯ ������ �� ���̿� ���� ó���� ���ֱ� ����
	// ���� ���� ��쿡�� ������ ó���� ���ֱ� ���� ����
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	{
		//// EffectSpecHandle ����(ASC�� ���Ͽ�)
		//const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

		//// Chapter149) EffectContextHandle�� �� ���� ������ �ְ� �ʹ�
		//// �������� Instigator, EffectCauser, InstigatorASC�� ä������
		//// �� ���� MakeEffectContext()�� ä���� ������ �������� ���� ä�����Ѵ�.
		//FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();

		//// AbilityCDO, AbilityInstanceNotReplicated�� (GA_FireBolt)�� ä���
		//// SetAbility�� AbilityLevel�� ä���ֳ� ���� �߿��Ѱ� �ƴѰ� ����.
		//// UAuraGameplayAbility��� ������ this ����
		//EffectContextHandle.SetAbility(this);

		//// SourceObject�� (BP_FireBolt)�� ä���
		//EffectContextHandle.AddSourceObject(Projectile);

		//// Actors �迭 ä���ֱ�
		//// ex) 0�� �迭�� BP_FireBolt
		//TArray<TWeakObjectPtr<AActor>> Actors;
		//Actors.Add(Projectile);
		//EffectContextHandle.AddActors(Actors);

		//// HitResult ä���
		//FHitResult HitResult;
		//HitResult.Location = ProjectileTargetLocation;
		//EffectContextHandle.AddHitResult(HitResult);

		//const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		//// Chapter149 ��

		//// �±� ����
		//const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

		//const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
		//UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);

		//// �����ڵ��� damage�±� Ű�� 50 ��ġ�� Ű���� ���´�.
		//Projectile->DamageEffectSpecHandle = SpecHandle;
	}
	
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	// ��ȯ ������
	Projectile->FinishSpawning(SpawnTransform);

}
