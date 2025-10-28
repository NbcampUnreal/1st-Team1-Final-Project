#include "System/Server/APIData.h"

FString UAPIData::GetAPIEndpoint(const FGameplayTag& APIEndpoint)
{
	const FString ResourceName = Resources.FindChecked(APIEndpoint);
	return InvokeURL + "/" + Stage + "/" + ResourceName;
}
