#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "GS_SmallClaw.generated.h"

UCLASS()
class GAS_API AGS_SmallClaw : public AGS_Monster
{
	GENERATED_BODY()

public:
	AGS_SmallClaw();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	UAkAudioEvent* SmallClawClickSound;

protected:
	virtual void BeginPlay() override;
}; 