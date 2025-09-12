#include "Props/Interactables/GS_BridgePiece.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGS_BridgePiece::AGS_BridgePiece()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bIsDestroyed = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BridgeMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetIsReplicated(true);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BridgeMaterialAsset(TEXT("/Game/BossLevel/Materials/Props_Materials/Boss_Bridge/Boss_Bridege.Boss_Bridege"));

	if (BridgeMaterialAsset.Succeeded())
	{
		MeshComponent->SetMaterial(0, BridgeMaterialAsset.Object);
	}
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
			
			bIsDestroyed = true;
			FTimerHandle AfterBrokenTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(AfterBrokenTimerHandle, this, &AGS_BridgePiece::StopSimulate, 5.f);
		}
	}
}

void AGS_BridgePiece::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;

	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetMassOverrideInKg(NAME_None, 70000.0f, true);
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
