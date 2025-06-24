#include "Character/Skill/Guardian/Drakhar/GS_EarthquakeEffect.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AGS_EarthquakeEffect::AGS_EarthquakeEffect()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GC"));
	GeometryCollectionComponent->SetupAttachment(RootComponent);
}

void AGS_EarthquakeEffect::BeginPlay()
{
	Super::BeginPlay();

	if (GeometryCollectionComponent)
	{
		GeometryCollectionComponent->SetSimulatePhysics(true);
		GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		GeometryCollectionComponent->SetEnableGravity(true);
	}
	SetLifeSpan(2.f);
}

void AGS_EarthquakeEffect::MulticastTriggerDestruction_Implementation(FVector ImpactLocation, float Radius, float Strength)
{
	if (GeometryCollectionComponent && !HasAuthority())
	{
		GeometryCollectionComponent->AddRadialImpulse(ImpactLocation, Radius, Strength, ERadialImpulseFalloff::RIF_Linear, true);
		//GeometryCollectionComponent->AddImpulse(FVector(0.f, 0.f, 300.f), NAME_None, true);
	}
}