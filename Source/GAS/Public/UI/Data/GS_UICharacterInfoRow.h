#pragma once
#include "CoreMinimal.h"
#include "GS_UICharacterInfoRow.generated.h"

USTRUCT(BlueprintType)
struct FGS_UICharacterInfoRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture> Portrait;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};