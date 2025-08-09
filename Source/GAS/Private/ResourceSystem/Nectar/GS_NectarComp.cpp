#include "ResourceSystem/Nectar/GS_NectarComp.h"
#include "DungeonEditor/Component/PlaceInfoComponent.h"

void UGS_NectarComp::InitializeMaxAmount(float Amount)
{
	Super::InitializeMaxAmount(Amount);
	OnNectarChanged.Broadcast(GetCurrentAmount());
}


void UGS_NectarComp::AddResource(float Amount)
{
	Super::AddResource(Amount);
	OnNectarChanged.Broadcast(GetCurrentAmount());

}

void UGS_NectarComp::SpendResource(float Amount)
{
	Super::SpendResource(Amount);
	OnNectarChanged.Broadcast(GetCurrentAmount());
}

bool UGS_NectarComp::CanSpendResource(float Amount) const
{
	return IsResourceInBound(Amount, true);
}


void UGS_NectarComp::RetrieveNectar(AActor* TargetActor)
{
	if (!IsValid(TargetActor)) return;
	if (UPlaceInfoComponent* PlaceComp = TargetActor->FindComponentByClass<UPlaceInfoComponent>())
	{
		AddResource(PlaceComp->ConstructionCost);
		UE_LOG(LogTemp, Warning, TEXT("Destroyed. Retrived Nectar Amount : %f"), PlaceComp->ConstructionCost);
	}
}