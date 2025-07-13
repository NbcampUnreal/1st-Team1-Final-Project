#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GS_LobbyAnimInstance.generated.h"

UCLASS()
class GAS_API UGS_LobbyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby State")
	bool bIsReady = false;
};
