#include "System/Server/HTTPRequestTypes.h"

void FGASMetaData::Dump() const
{
	UE_LOG(LogTemp, Log, TEXT("MetaData:"));
	
	UE_LOG(LogTemp, Log, TEXT("httpStatusCode: %d"), httpStatusCode);
	UE_LOG(LogTemp, Log, TEXT("requestId: %s"), *requestId);
	UE_LOG(LogTemp, Log, TEXT("attempts: %d"), attempts);
	UE_LOG(LogTemp, Log, TEXT("totalRetryDelay: %f"), totalRetryDelay);
}