#include "Props/Marker/GS_MarkerActor.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

AGS_MarkerActor::AGS_MarkerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	RootComponent = DecalComponent;
	
	// 기본 데칼 설정 (블루프린트에서 조정 가능)
	DecalComponent->DecalSize = FVector(64.0f, 64.0f, 64.0f);
	DecalComponent->SetVisibility(true);
}

void AGS_MarkerActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateDecalMaterial();
}

void AGS_MarkerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_MarkerActor, MarkerType);
	DOREPLIFETIME(AGS_MarkerActor, OwnerPlayerState);
}

void AGS_MarkerActor::OnRep_MarkerType()
{
	UpdateDecalMaterial();
}

void AGS_MarkerActor::UpdateDecalMaterial()
{
	if (DecalComponent && MarkerMaterials.Contains(MarkerType))
	{
		if (UMaterialInterface* Mat = MarkerMaterials[MarkerType])
		{
			DecalComponent->SetDecalMaterial(Mat);
		}
	}
}

