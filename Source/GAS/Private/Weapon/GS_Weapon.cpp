#include "Weapon/GS_Weapon.h"

// Sets default values
AGS_Weapon::AGS_Weapon()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGS_Weapon::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void AGS_Weapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...
}
