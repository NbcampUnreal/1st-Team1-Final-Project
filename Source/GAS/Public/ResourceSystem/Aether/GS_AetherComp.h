#pragma once

#include "CoreMinimal.h"
#include "ResourceSystem/GS_ResourceBaseComp.h"
#include "GS_AetherComp.generated.h"

UCLASS()
class GAS_API UGS_AetherComp : public UGS_ResourceBaseComp
{
	GENERATED_BODY()

public:
	virtual void InitializeMaxAmount(float Amount) override;
	virtual void AddResource(float Amount) override;
	//virtual void SpendResource(float Amount) override;
	bool CanAddResource(float Amount) const;
};
