#include "DungeonEditor/Component/PlaceInfoComponent.h"

UPlaceInfoComponent::UPlaceInfoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UPlaceInfoComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPlaceInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

