#include "Props/Interactables/GS_BridgePiece.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGS_BridgePiece::AGS_BridgePiece()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bIsDestroyed = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BridgeMesh"));
	MeshComponent->SetIsReplicated(true);
}

void AGS_BridgePiece::SetBridgeMesh(UStaticMesh* InMesh, UMaterialInterface* InMaterial, float InValue)
{
	if (MeshComponent && InMesh)
	{
		MeshComponent->SetStaticMesh(InMesh);

		if (InMaterial)
		{
			BridgeMaterial = InMaterial;
		}
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
			
			bIsDestroyed = true;
			FTimerHandle AfterBrokenTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(AfterBrokenTimerHandle, this, &AGS_BridgePiece::StopSimulate, 5.f);
		}
	}
}

void AGS_BridgePiece::OnRep_BridgeMaterial()
{
	if (MeshComponent && BridgeMaterial)
	{
		MeshComponent->SetMaterial(0, BridgeMaterial);
	}
}

void AGS_BridgePiece::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;

	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetMassOverrideInKg(NAME_None, 70000.0f, true);
}

void AGS_BridgePiece::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BridgeMaterial);
}

void AGS_BridgePiece::StopSimulate()
{
	if (HasAuthority())
	{
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
		SetLifeSpan(2.f);
	}
}
