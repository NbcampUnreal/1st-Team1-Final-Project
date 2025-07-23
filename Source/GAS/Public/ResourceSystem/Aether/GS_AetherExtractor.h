#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_AetherExtractor.generated.h"

UCLASS()
class GAS_API AGS_AetherExtractor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_AetherExtractor();

protected:
	virtual void BeginPlay() override;
};
