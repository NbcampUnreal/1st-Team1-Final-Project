// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_ArrowVisualActor.generated.h"

UCLASS()
class GAS_API AGS_ArrowVisualActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_ArrowVisualActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ArrowMesh;

	void SetArrowMesh(UStaticMesh* Mesh);
};
