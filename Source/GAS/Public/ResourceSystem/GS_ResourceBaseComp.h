#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_ResourceBaseComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_ResourceBaseComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_ResourceBaseComp();

protected:
	virtual void BeginPlay() override;

};
