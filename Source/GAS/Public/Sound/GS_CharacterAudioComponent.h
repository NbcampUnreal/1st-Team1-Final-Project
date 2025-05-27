// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AkGameplayStatics.h"
#include "GS_CharacterAudioComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_CharacterAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_CharacterAudioComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Character")
	class UAkAudioEvent* SkillEvent;

	int32 SkillEventID;

	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void PlaySkill();
	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void StopSkill();

		
};
