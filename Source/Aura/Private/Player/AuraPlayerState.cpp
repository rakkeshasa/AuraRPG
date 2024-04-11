// Chocolate Maniac

#include "Player/AuraPlayerState.h"
#include <AbilitySystem/AuraAbilitySystemComponent.h>
#include <AbilitySystem/AuraAttributeSet.h>
#include "Net/UnrealNetwork.h"

AAuraPlayerState::AAuraPlayerState()
{
	// https://kkadalg.tistory.com/29
	// MOBA�� ����ó�� ���Ͱ� �������ǰ� ���� ���̿� Attribute �Ǵ� GameplayEffect�� ���Ӽ��� �ʿ��� ���, 
	// ASC�� �̻����� ��ġ�� PlayerState Ŭ�����Դϴ�.
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); // �������� ������Ʈ ����ȭ(����)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	// ReplicationMode 3����
	// Minimal : ��Ƽ�÷���, AI��Ʈ�ѿ�(���� ����Ʈ�� �������� �ʰ� ť�� �±׸� ��� Ŭ������ ������)
	// Mixed : ��Ƽ�÷���, �÷��̾� ��Ʈ�ѿ�(�ڱ� Ŭ�� ���� ����Ʈ�� ������, ť�� �±״� ��� Ŭ������ ����)
	// Full : �̱��÷��̿�(���� ����Ʈ�� ��� Ŭ������ ������)

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet"); // �ش� ������Ʈ �Ӽ�set

	// ASC�� PlayerState�� �ִ� ���, PlayerState�� NetUpdateFrequency�� �÷��� �մϴ�. 
	NetUpdateFrequency = 100.f; // ������ �󸶳� ������Ʈ ����(1�� �� 100��)
}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// Replicated�� �Ӽ����� ������ֱ�
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
	// XP�� ���������� Ȱ�� �����Ǹ�, ���ø�����Ʈ�� �����̹Ƿ� 
	// Ŭ���̾�Ʈ�� ���ø�����Ʈ�ǰ� �� ������ OnRep_XP()�� ȣ���.
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
