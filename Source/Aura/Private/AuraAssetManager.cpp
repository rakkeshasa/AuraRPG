// Chocolate Maniac

#include "AuraAssetManager.h"

#include "AuraGameplayTags.h"
#include "AbilitySystemGlobals.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);

	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager; // Ÿ���� &�̹Ƿ� �������� return
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FAuraGameplayTags::InitializeNativeGameplayTags();

	// This is required to use Target Data!
	UAbilitySystemGlobals::Get().InitGlobalData();
}
