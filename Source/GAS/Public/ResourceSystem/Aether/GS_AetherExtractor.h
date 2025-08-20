#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceSystem/Aether/GS_AetherComp.h"
#include "System/GS_PlayerState.h"
#include "GS_AetherExtractor.generated.h"

UCLASS()
class GAS_API AGS_AetherExtractor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_AetherExtractor();

protected:
	virtual void BeginPlay() override;

	FTimerHandle AetherExtractTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether")
	float ExtractionInterval = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether")
	float ExtractionAmount = 5.f;
	
	UPROPERTY()
	UGS_AetherComp* CachedAetherComp = nullptr;

public:
	UGS_AetherComp* FindGuardianAetherComp();

	UFUNCTION()
	void ExtractAether();
	void InitializeAetherComp();
};
