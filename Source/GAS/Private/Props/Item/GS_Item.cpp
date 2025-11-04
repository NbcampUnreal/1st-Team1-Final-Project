// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Item/GS_Item.h"

#include "ContentBrowserItemData.h"
#include "Props/Item/GS_ItemData.h"


// Sets default values
AGS_Item::AGS_Item()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void AGS_Item::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGS_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGS_Item::SetItemData(UGS_ItemData* InputItemData)
{
	ItemData = InputItemData;
}

void AGS_Item::SetMesh(FName StaticMeshName)
{
	if (!ItemData)
	{
		return;
	}
	UStaticMesh** StaticMesh = ItemData->ItemMeshs.Find(StaticMeshName);
	if (*StaticMesh)
	{
		MeshComp->SetStaticMesh(*StaticMesh);
	}
}

UStaticMeshComponent* AGS_Item::GetMeshComp()
{
	return MeshComp;
}

void AGS_Item::ItemDestroy()
{
	Destroy();
}
