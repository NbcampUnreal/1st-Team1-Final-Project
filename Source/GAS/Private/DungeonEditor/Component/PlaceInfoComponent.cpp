#include "DungeonEditor/Component/PlaceInfoComponent.h"

#include "DungeonEditor/Data/GS_DungeonEditorTypes.h"
#include "Net/UnrealNetwork.h"
#include "Props/Trap/GS_TrapData.h"
#include "RuneSystem/GS_EnumUtils.h"

UPlaceInfoComponent::UPlaceInfoComponent()
	: ObjectType(EObjectType::None)
	, TrapPlacement(ETrapPlacement::Ceiling)
	, ConstructionCost(0.0f)
	, Is_ObjectTypeSynchronization(false)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPlaceInfoComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPlaceInfoComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner actor '%s' is being destroyed. Cost : %f"), *GetOwner()->GetName(), ConstructionCost);
	}
}

void UPlaceInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UPlaceInfoComponent::OnRep_UpdateObjectType()
{
	Is_ObjectTypeSynchronization = true;
	UE_LOG(LogTemp, Warning, TEXT("[방 숨김] %s : ObjectType 동기화 완료, Type : %s"), *GetOwner()->GetName(), *UGS_EnumUtils::GetEnumAsString<EObjectType>(ObjectType));
}

void UPlaceInfoComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UPlaceInfoComponent, CellCoord);
	DOREPLIFETIME(UPlaceInfoComponent, ObjectType);
	DOREPLIFETIME(UPlaceInfoComponent, TrapPlacement);
	DOREPLIFETIME(UPlaceInfoComponent, ConstructionCost);
}
