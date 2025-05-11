// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_CharacterAudioComponent.h"

UGS_CharacterAudioComponent::UGS_CharacterAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UGS_CharacterAudioComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UGS_CharacterAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UGS_CharacterAudioComponent::PlaySkill()
{
	FOnAkPostEventCallback DummyCallback;

	SkillEventID = UAkGameplayStatics::PostEvent(SkillEvent,
		GetOwner(), // Post the event to the owner of this component 
		0, // No callback mask
		DummyCallback // No callback
	);
}

void UGS_CharacterAudioComponent::StopSkill()
{
	if (GetOwner())
	{
		UAkGameplayStatics::StopActor(GetOwner());
	}
}

