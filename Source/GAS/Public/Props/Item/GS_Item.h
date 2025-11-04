// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_Item.generated.h"

UCLASS()
class GAS_API AGS_Item : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_Item();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SetItemData(UGS_ItemData* InputItemData);

	UFUNCTION()
	void SetMesh(FName StaticMeshName);

	UFUNCTION()
	UStaticMeshComponent* GetMeshComp();

	UFUNCTION()
	void ItemDestroy();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UGS_ItemData* ItemData;


};
