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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Resource")
	float CurrentAmount = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float MaxAmount = 600.f;
	bool bIsInitialized = false;

public:
	UFUNCTION(Category = "Resource")
	virtual void InitializeMaxAmount(float Amount);


	UFUNCTION(Category = "Resource")
	virtual float GetCurrentAmount() const;
	UFUNCTION(Category = "Resource")
	virtual float GetMaxAmount() const;


	UFUNCTION(Category = "Resource")
	virtual void AddResource(float Amount);
	UFUNCTION(Category = "Resource")
	virtual void SpendResource(float Amount);
	//UFUNCTION(Category = "Resource")
	//virtual bool CanSpend(float Amount) const;

	bool IsResourceInBound(float Amount, bool bIsSpending) const;

};
