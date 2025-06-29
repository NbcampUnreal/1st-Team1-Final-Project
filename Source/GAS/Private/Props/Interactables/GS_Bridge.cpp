#include "Props/Interactables/GS_Bridge.h"

#include "Props/Interactables/GS_BridgePiece.h"

AGS_Bridge::AGS_Bridge()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	NotBrokenPieces1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh1"));
	NotBrokenPieces1->SetupAttachment(RootComponent);
	NotBrokenPieces2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh2"));
	NotBrokenPieces2->SetupAttachment(RootComponent);
	NotBrokenPieces3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh3"));
	NotBrokenPieces3->SetupAttachment(RootComponent);
	NotBrokenPieces4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh4"));
	NotBrokenPieces4->SetupAttachment(RootComponent);
	NotBrokenPieces5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh5"));
	NotBrokenPieces5->SetupAttachment(RootComponent);
}

void AGS_Bridge::BeginPlay()
{
	Super::BeginPlay();

	SetUpBridge();
}

void AGS_Bridge::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

}

void AGS_Bridge::SetUpBridge()
{
	if (HasAuthority())
	{
		HalfBridgeMeshSize = BridgeMeshAssets.Num() / 2;
		
		for (int32 i = 0; i < BridgeMeshAssets.Num(); ++i)
		{
			UStaticMesh* CurrentMesh = BridgeMeshAssets[i];
			
			if (!CurrentMesh)
			{
				continue;
			}
			
			UChildActorComponent* NewPieceComponent = NewObject<UChildActorComponent>(this);
			if (NewPieceComponent)
			{
				NewPieceComponent->SetChildActorClass(AGS_BridgePiece::StaticClass());
				NewPieceComponent->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
				NewPieceComponent->RegisterComponent();
				
				AGS_BridgePiece* PieceActor = Cast<AGS_BridgePiece>(NewPieceComponent->GetChildActor());
				if (PieceActor)
				{
					float weight = 1.f - (abs(i - HalfBridgeMeshSize) / (float)HalfBridgeMeshSize);
					
					FTransform Transform = FTransform::Identity;
					PieceActor->SetActorRelativeTransform(Transform);
					PieceActor->SetBridgeMesh(CurrentMesh, weight * 100.f + 100.f);
				}
				BrokenPieces.Add(NewPieceComponent);
			}
		}
	}
}



