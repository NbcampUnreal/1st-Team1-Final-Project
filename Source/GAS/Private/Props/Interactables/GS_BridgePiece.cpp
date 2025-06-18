#include "Props/Interactables/GS_BridgePiece.h"

AGS_BridgePiece::AGS_BridgePiece()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bIsDestroyed = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BridgeMesh"));
	MeshComponent->SetIsReplicated(true);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetMassOverrideInKg(NAME_None, 70000.0f, true);
}

void AGS_BridgePiece::SetBridgeMesh(UStaticMesh* InMesh, float InValue)
{
	if (MeshComponent && InMesh)
	{
		MeshComponent->SetStaticMesh(InMesh);
		MaxHealth = InValue;
	}
}

void AGS_BridgePiece::BrokeBridge(float InDamage)
{
	if (HasAuthority() && !bIsDestroyed)
	{
		CurrentHealth -= InDamage;
		
		if (CurrentHealth <= KINDA_SMALL_NUMBER)
		{
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->bApplyImpulseOnDamage = false;
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		}
	}
}

void AGS_BridgePiece::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
}
