#include "Weapon/GS_Weapon.h"

// Sets default values
AGS_Weapon::AGS_Weapon()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = false;

	
	
}

// Called when the game starts or when spawned
void AGS_Weapon::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void AGS_Weapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called every frame
void AGS_Weapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ...
}
