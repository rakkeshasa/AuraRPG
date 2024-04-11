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

	

	// 블루프린트 내 이벤트 그래프에서 사용하는 PrintString이 바로 이것
	// UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (C++)"), true, true, FLinearColor::Yellow, 3);
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	// 서버에 있는지, 있을 시 true
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 언리얼 엔진에서는 Execute_ 함수를 사용하여 네이티브 이벤트를 호출
	// Execute_ 함수는 네이티브 이벤트를 호출할 때 사용되는 헬퍼 함수
	// Weapon Socket을 찾는다
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	// 무기 소켓에 위치 지정
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	// SpawnActorDeferred -> BeginPlay 이전에 어떠한 액터에 추가 작업
	// FinishSpawning을 호출해주냐 안해주냐의 차이
	// 왜 spawn이 2단계로 나뉘는가?
	// 소환하고 소환 끝나기 그 사이에 무언가 처리를 해주기 위해
	// 지금 같은 경우에는 데미지 처리를 해주기 위해 나눔
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	{
		//// EffectSpecHandle 생성(ASC를 통하여)
		//const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

		//// Chapter149) EffectContextHandle에 더 많은 정보를 넣고 싶다
		//// 기존에는 Instigator, EffectCauser, InstigatorASC만 채워졌다
		//// 이 것은 MakeEffectContext()가 채워준 것으로 나머지는 따로 채워야한다.
		//FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();

		//// AbilityCDO, AbilityInstanceNotReplicated를 (GA_FireBolt)로 채우기
		//// SetAbility가 AbilityLevel도 채워주나 딱히 중요한건 아닌거 같다.
		//// UAuraGameplayAbility상속 중으로 this 가능
		//EffectContextHandle.SetAbility(this);

		//// SourceObject를 (BP_FireBolt)로 채우기
		//EffectContextHandle.AddSourceObject(Projectile);

		//// Actors 배열 채워주기
		//// ex) 0번 배열에 BP_FireBolt
		//TArray<TWeakObjectPtr<AActor>> Actors;
		//Actors.Add(Projectile);
		//EffectContextHandle.AddActors(Actors);

		//// HitResult 채우기
		//FHitResult HitResult;
		//HitResult.Location = ProjectileTargetLocation;
		//EffectContextHandle.AddHitResult(HitResult);

		//const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		//// Chapter149 끝

		//// 태그 생성
		//const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

		//const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
		//UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);

		//// 스펙핸들은 damage태그 키와 50 수치의 키값을 갖는다.
		//Projectile->DamageEffectSpecHandle = SpecHandle;
	}
	
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	// 소환 끝내기
	Projectile->FinishSpawning(SpawnTransform);

}
