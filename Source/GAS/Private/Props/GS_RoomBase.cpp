#include "Props/GS_RoomBase.h"

AGS_RoomBase::AGS_RoomBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Floor = CreateDefaultSubobject<UStaticMeshComponent>("Floor");
	SetRootComponent(Floor);
	
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

void AGS_RoomBase::UseDepthStencil()
{
	Floor->SetRenderCustomDepth(true);
	Wall->SetRenderCustomDepth(true);
}
