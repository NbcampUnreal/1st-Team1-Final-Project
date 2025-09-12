#include "DungeonEditor/Component/PlaceInfoComponent.h"

UPlaceInfoComponent::UPlaceInfoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UPlaceInfoComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPlaceInfoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner actor '%s' is being destroyed. Cost : %f"), *GetOwner()->GetName(), ConstructionCost);
	}
}

void UPlaceInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

