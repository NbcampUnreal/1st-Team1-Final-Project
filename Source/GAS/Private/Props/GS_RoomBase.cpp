#include "Props/GS_RoomBase.h"

AGS_RoomBase::AGS_RoomBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Floor = CreateDefaultSubobject<UStaticMeshComponent>("Floor");
	Floor->SetupAttachment(RootComponent);
	
	Wall = CreateDefaultSubobject<UStaticMeshComponent>("Wall");
	Wall->SetupAttachment(Floor);

	Ceiling = CreateDefaultSubobject<UStaticMeshComponent>("Ceiling");
	Ceiling->SetupAttachment(Floor);
	
}

void AGS_RoomBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGS_RoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGS_RoomBase::HideCeiling()
{
	if (Ceiling)
	{
		Ceiling->SetVisibility(false);
	}
}

void AGS_RoomBase::ShowCeiling()
{
	if (Ceiling)
	{
		Ceiling->SetVisibility(true);
	}
}