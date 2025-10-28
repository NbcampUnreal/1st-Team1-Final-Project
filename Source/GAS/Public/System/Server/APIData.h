#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DedicatedServersTags.h"
#include "APIData.generated.h"

UCLASS()
class GAS_API UAPIData : public UDataAsset
{
	GENERATED_BODY()

public:
	FString GetAPIEndpoint(const FGameplayTag& APIEndpoint);
protected:
	UPROPERTY(EditDefaultsOnly)
	FString Name;

	UPROPERTY(EditDefaultsOnly)
	FString InvokeURL;

	UPROPERTY(EditDefaultsOnly)
	FString Stage;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FString> Resources;
};
