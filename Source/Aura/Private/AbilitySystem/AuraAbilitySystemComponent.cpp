// Chocolate Maniac

#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	// �� ���͸��� �ѹ��� ����->�� ���ʹ� ASC�� ������ ���� �ִ�.
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);

	// �̱���
	// const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
}

void UAuraAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	// ����� GA �̾ƿ���
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);
		
		// ��ų���Կ�
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);

		// �нú�� �ٷ� �۵��ϰ�, ��Ƽ��� ������ �ǵ����Ѵ�.
		if (Data.AbilityType == FAuraGameplayTags::Get().Abilities_Type_Offensive)
		{
			GiveAbility(LoadedAbilitySpec);
		}
		else if (Data.AbilityType == FAuraGameplayTags::Get().Abilities_Type_Passive)
		{
			if (Data.AbilityStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);
				MulticastActivatePassiveEffect(Data.AbilityTag, true);
			}
			else
			{
				GiveAbility(LoadedAbilitySpec);
			}
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	// AuraCharacterBase���� ȣ��
	// ���	�ɷ� �ο��ϱ�
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		// �Է� �±��� ������ ������ �ϵ�, ��Ÿ�ӿ� �̸� ������ �� �ֵ���
		// AbilitySpec�� �����Ͽ� �ش� �ɷ��� ���� ������ 1�� ����
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		// StartupInputTag�� AuraGameplayAbility�� �����Ƿ� ĳ����
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			// DynamicAbilityTags�� ��Ÿ�� �� �±װ� ������ �� �ֵ��� ��.
			// AuraGameplayAbility���� �Ļ��� ���� ������ ���� �����Ƽ �±׿� �߰�
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			
			// �ɷ�â���� ������ ����(icon���̰� ���� ���̰�)�� ����
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			
			GiveAbility(AbilitySpec);
			// GiveAbilityAndActivateOnce() -> �����Ƽ�� ���ڸ��� �ߵ�
		}
	}
	bStartupAbilitiesGiven = true; // �ɷ��� �־�����.
	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec);
		// AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
	}
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	// �� �Լ��� Ŭ���� ������ �� ���� ȣ��
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				// �ش� �±װ� Ȱ��ȭ ������ �� ���콺 Ŭ�� ���ΰ�?
				// InvokeRelicatedEvent�� ��Ʈ��ũ ����� ���� Ŭ���̾�Ʈ�� ���� �̺�Ʈ�� ȣ���ϴ� ���
				// �ַ� ��Ƽ�÷��̾� ���ӿ��� ���Ǹ�, �������� �߻��� �̺�Ʈ�� Ŭ���̾�Ʈ�� �����Ͽ� 
				// ��� �÷��̾ ������ ���� ���¸� ������ �� �ֵ��� �մϴ�. 
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// GetActivatableAbilities() -> ASC���� Ȱ��ȭ ������ �ɷ��� �ִ���
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// ������ ���̿��ٸ�
			AbilitySpecInputPressed(AbilitySpec);

			// Ȱ��ȭ ���� �ƴ϶��
			if (!AbilitySpec.IsActive())
			{
				// Ȱ��ȭ��Ű��
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			// ���ø�����Ʈ�� �̺�Ʈ�� ȣ�� -> WaitInputRelease�� ���� ����
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// ���� ���� ���� ���� ���� �ʰ� lock�ϱ�
	// �����ϰų� �߰��Ϸ��� ��� �����Ƽ�� �����ϰ� 
	// �� ������ �Ϸ�� ������ ��ٷȴٰ� �ش� ������� �̵��Ͽ� �����Ѵ�.
	FScopedAbilityListLock ActiveScopeLock(*this);

	// Ȱ�������� �ɷµ� ���� ������
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// �־��� ��������Ʈ�� �����մϴ�. ExecuteIfBound �Լ��� ��������Ʈ�� ���ε��Ǿ� �ִ� ��� �����մϴ�. 
		// �־��� �����Ƽ ������ ��������Ʈ�� �����ϰ� �����մϴ�.
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// Ability�� nullptr�� �ƴ϶��
	if (AbilitySpec.Ability)
	{
		// �±׸��� ���� ����
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			// �±׳��� Abilities��� �ܾ �ִٸ� �ش� �±� return
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	// InputTag�� ����AbilityTag�� ���������.
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}

	return FGameplayTag();
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	// �ش� ���Կ� ��ų�� ���ִ��� ������?
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}

	return true;
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	// �ش� ��ų�� ����â�� �����ϰ� �ִ���
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	// �нú� ��ų����
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.DynamicAbilityTags.AddTag(Slot);
}

void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	// ������ ���Ϳ��� �̺�Ʈ�� �����ϱ⸸ �ϸ� 
	// 'Gameplay Ability'�� �ش� �̺�Ʈ�� Ư�� �±װ� �ִ� �̺�Ʈ�� �����ϰ� 
	// �ش� �̺�Ʈ�� ������ ����Ͽ� �����÷��� ����Ʈ�� ������ �� �ֽ��ϴ�.
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	// �̺�Ʈ ����~
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	// �� �̺�Ʈ�� ������ �� GA_ListenForEvent�� �����ϸ�  
	// Gameplay Effect�� �����ϰ� �⺻ �Ӽ� �� �ϳ��� �����ϵ��� ������ �� �ִ�
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		// ���� ����Ʈ ���� -1 ���ֱ�
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;

		// ���� ������ ��ų �رݿ� �䱸�ϴ� �������� ���ٸ�
		if (Level < Info.LevelRequirement) continue;

		// �ɷ��� ���� ���� �ʴٸ�
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			// �������� �Ǿ� ��ų�ر��� �����ϳ� ���� ��ų ����Ʈ�� ���� ���� ����
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);

			// ���� Update���� ��ٸ��� �ʰ� ���� ������ ���ø�����Ʈ
			// client�� �� ���� ������ �߻��ߴ��� ��� �� �� �ֵ��� �ϴ� ���� �߿��ϹǷ�
			// SpellMenuWidgetController�� ��ε�ĳ��Ʈ�� �����Ͽ� ������ ������Ʈ �� �� �ֵ��� �Ѵ�.
			MarkAbilitySpecDirty(AbilitySpec);
			ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		// ��ų ����Ʈ 1 ����
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}

		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			// �رݻ��¿��� ��ų ���� ������ ���� ������ �Ǿ����Ƿ� �±� �������ֱ�
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			// �±� �Ӹ� �ƴ϶� ��ų ���µ� �ٲ��ֱ�
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			// �̹� ������ �ϳ��� ������ ��� ���� ���� ���ֱ�
			AbilitySpec->Level += 1;
			// �̹� �߻�� ����ü���� ����x
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		// ���� �����ϱ� ���� �� ��ų�� � ���Կ� �־������� ���� ���� ������
		// �ٸ� ���Կ� ��ų�� �����Ѵٸ� ���� ������ clear�� �� �ִ�.
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		// ��ų ���ڸ� �ϰų� �̹� �����Ǿ��ٸ� ��ų ���Կ� ��� �����ϴ�.
		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
		if (bStatusValid)
		{
			// �ش� ������ �̹� ��ų�� ä�����ִ�.
			if (!SlotIsEmpty(Slot))
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// ���Կ� ������ ��ų�� �������� ��ų�� ���� ��� return
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
						return;
					}

					// �нú� ��ų�̸� ��Ȱ��ȭ�ϱ�
					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					ClearSlot(SpecWithSlot);
				}
			}

			/* ���� ��ų �о������ �� ��ų �־��ֱ� */
			
			// �ش� ��ų�� �ƹ� ���Ե� �����ϰ� ���� �ʴٸ�
			if (!AbilityHasAnySlot(*AbilitySpec))
			{
				// �־��� ��ų�� �нú��� Ȱ��ȭ��Ű��
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				// 408
				AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}

			AssignSlotToAbility(*AbilitySpec, Slot);
			// �ش� �ɷ� ����(ability spec)�� ����Ǿ����� Ability System�� ����
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	// ��� ��ų�� ���
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		// �ƹ� ��ų�� ���� ���� �����Ͻ� �� string �����ֱ�
		OutDescription = FString();
	}
	else
	{
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);

	// ActivatableAbilities�� ��� �����Ƽ Spec�� �ݺ� 
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		// �ش� ��ų�� ������ ������ ���� ��
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	// �ش� ��ų�� ������ ������ �ִ���?
	for (FGameplayTag Tag : Spec->DynamicAbilityTags)
	{
		if (Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	// �ɷ��� ó�� �ο��� ���� ��ε�ĳ����
	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}
