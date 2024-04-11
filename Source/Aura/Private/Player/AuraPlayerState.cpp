// Chocolate Maniac

#include "Player/AuraPlayerState.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include <AbilitySystem/AuraAttributeSet.h>
#include "Net/UnrealNetwork.h"

AAuraPlayerState::AAuraPlayerState()
{
	// https://kkadalg.tistory.com/29
	// MOBA의 영웅처럼 엑터가 리스폰되고 스폰 사이에 Attribute 또는 GameplayEffect의 지속성이 필요한 경우, 
	// ASC의 이상적인 위치는 PlayerState 클래스입니다.
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); // 서버에서 컴포넌트 동기화(복제)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	// ReplicationMode 3가지
	// Minimal : 멀티플레이, AI컨트롤용(게임 이펙트는 복제되지 않고 큐와 태그만 모든 클라한테 복제됨)
	// Mixed : 멀티플레이, 플레이어 컨트롤용(자기 클라만 게임 이펙트가 복제됨, 큐와 태그는 모든 클라한테 복제)
	// Full : 싱글플레이용(게임 이펙트가 모든 클라한테 복제됨)

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet"); // 해당 오브젝트 속성set

	// ASC가 PlayerState에 있는 경우, PlayerState의 NetUpdateFrequency를 늘려야 합니다. 
	NetUpdateFrequency = 100.f; // 서버가 얼마나 업데이트 할지(1초 당 100번)
}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Replicated할 속성값을 등록해주기
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraPlayerState, Level);
	DOREPLIFETIME(AAuraPlayerState, XP);
	DOREPLIFETIME(AAuraPlayerState, AttributePoints);
	DOREPLIFETIME(AAuraPlayerState, SpellPoints);
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


void AAuraPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	// XP는 서버에서만 활성 설정되며, 리플리케이트된 변수이므로 
	// 클라이언트로 리플리케이트되고 그 시점에 OnRep_XP()가 호출됨.
	OnXPChangedDelegate.Broadcast(XP);
}

void AAuraPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void AAuraPlayerState::AddToAttributePoints(int32 InPoints)
{
	AttributePoints += InPoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AAuraPlayerState::AddToSpellPoints(int32 InPoints)
{
	SpellPoints += InPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void AAuraPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void AAuraPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level, false);
}

void AAuraPlayerState::SetAttributePoints(int32 InPoints)
{
	AttributePoints = InPoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AAuraPlayerState::SetSpellPoints(int32 InPoints)
{
	SpellPoints = InPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void AAuraPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangedDelegate.Broadcast(XP);
}

void AAuraPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AAuraPlayerState::OnRep_SpellPoints(int32 OldSpellPoints)
{
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}
