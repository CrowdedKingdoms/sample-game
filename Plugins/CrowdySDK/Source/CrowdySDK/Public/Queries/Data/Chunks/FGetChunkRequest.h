#pragma once
#include "Core/GraphQL/Enums/EGraphQLQuery.h"
#include "Core/GraphQL/Interfaces/ICrowdyQueryRequest.h"

struct FGetChunkRequest : ICrowdyQueryRequest
{
	int64 MapID;
	int64 ChunkX;
	int64 ChunkY;
	int64 ChunkZ;
	int32 MaxDistance;
	int32 Limit;
	int32 Skip;
	
	virtual FName GetOperationName() const override
	{
		return "Get Chunk Request";
	}
	
	virtual void PrepareQuery() override
	{
		RuntimeVariables.Add(TEXT("input.mapId"), FString::Printf(TEXT("%lld"), MapID));
		RuntimeVariables.Add(TEXT("input.centerCoordinate.x"), FString::Printf(TEXT("%lld"), ChunkX));
		RuntimeVariables.Add(TEXT("input.centerCoordinate.y"), FString::Printf(TEXT("%lld"), ChunkY));
		RuntimeVariables.Add(TEXT("input.centerCoordinate.z"), FString::Printf(TEXT("%lld"), ChunkZ));
		RuntimeVariables.Add(TEXT("input.maxDistance"), FString::Printf(TEXT("%d"), MaxDistance));
		RuntimeVariables.Add(TEXT("input.limit"), FString::Printf(TEXT("%d"), Limit));
		RuntimeVariables.Add(TEXT("input.skip"), FString::Printf(TEXT("%d"), Skip));
		bIncludeAuthToken = true;
		bUseNestedJson = true;
		bIsValid = true;
	}
	
	virtual EGraphQLQuery GetQueryType() const override
	{
		return EGraphQLQuery::GetChunkByDistance;
	}
	virtual bool IsValid() const override
	{
		return MapID > -1 && RuntimeVariables.Num() == 6;
	}
};