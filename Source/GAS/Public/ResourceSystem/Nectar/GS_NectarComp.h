#pragma once

#include "CoreMinimal.h"
#include "ResourceSystem/GS_ResourceBaseComp.h"
#include "Delegates/DelegateCombinations.h"
#include "GS_NectarComp.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNectarChanged, float, NewAmount);

UCLASS()
class GAS_API UGS_NectarComp : public UGS_ResourceBaseComp
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintAssignable, Category="Nectar")
	FOnNectarChanged OnNectarChanged;

	//virtual void InitializeMaxAmount(float Amount) override;
	virtual void AddResource(float Amount) override;
	virtual void SpendResource(float Amount) override;
	bool CanSpendResource(float Amount) const;
	void RetrieveNectar(AActor* TargetActor);
};
