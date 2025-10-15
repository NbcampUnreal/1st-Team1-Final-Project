#pragma once

#include "CoreMinimal.h"
#include "ResourceSystem/GS_ResourceBaseComp.h"
#include "Delegates/DelegateCombinations.h"
#include "GS_AetherComp.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAetherChanged, float, NewAmount);

UCLASS()
class GAS_API UGS_AetherComp : public UGS_ResourceBaseComp
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedAmount)
	float ReplicatedAetherAmount = 123.f;

	UGS_AetherComp();

public:
	UPROPERTY(BlueprintAssignable, Category="Aether")
	FOnAetherChanged OnAetherChanged;

	UFUNCTION()
	void OnRep_ReplicatedAmount();

	UFUNCTION()
	void SetCurrentAether(float NewValue);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void InitializeMaxAmount(float Amount) override;
	virtual void AddResource(float Amount) override;
	//virtual void SpendResource(float Amount) override;
	bool CanAddResource(float Amount) const;
};
