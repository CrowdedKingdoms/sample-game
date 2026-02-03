// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/GraphQL/CrowdyQuerySubsystem.h"
#include "HttpModule.h"
#include "Core/GraphQL/DataAssets/GraphQLQueryDatabase.h"
#include "Core/GraphQL/Enums/EQueryResponseType.h"
#include "Interfaces/IHttpResponse.h"
#include "Internal/FCrowdyDataRegistry.h"
#include "Queries/Data/Version/FVersionInfoResponse.h"
#include "Queries/Authentication/FLoginResponse.h"
#include "Queries/Authentication/FRegisterResponse.h"
#include "Queries/Data/Chunks/FGetChunkResponse.h"
#include "Queries/Data/Chunks/FUpdateChunkResponse.h"
#include "Queries/Data/User/FGetUserStateResponse.h"
#include "Queries/Data/User/FUpdateUserStateResponse.h"
#include "Queries/Data/Voxels/FVoxelListByDistanceResponse.h"
#include "Queries/Data/Voxels/FVoxelListResponse.h"
#include "Queries/Permissions/FTeleportResponse.h"
#include "Queries/UDP/FUDPAddressNotify.h"
#include "Serialization/FCrowdyQueryParser.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "Queries/Data/Avatar/FAvatarCreateResponse.h"
#include "Queries/Data/Avatar/FAvatarDeleteResponse.h"
#include "Queries/Data/Avatar/FAvatarNameUpdateResponse.h"
#include "Queries/Data/Avatar/FAvatarStateUpdateResponse.h"
#include "Queries/Data/Avatar/FFetchAvatarsResponse.h"

void UCrowdyQuerySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCrowdyQuerySubsystem::Deinitialize()
{
	ClearAuthToken();
	delete QueryParser;
	QueryParser = nullptr;
	GetWorld()->GetTimerManager().ClearTimer(StatsTimerHandle);
	Super::Deinitialize();
}

void UCrowdyQuerySubsystem::InitializeQuerySubsystem(FCrowdyDataRegistry* InDataRegistry)
{
	DataRegistry = InDataRegistry;
	bIsDevelopment = true;

	QueryDatabase = LoadObject<UGraphQLQueryDatabase>(nullptr, TEXT("/CrowdySDK/Data/DA_CrowdyQueries.DA_CrowdyQueries"));

	ensure(QueryDatabase);
	
	if (!QueryDatabase)
	{
		UE_LOG(LogTemp, Fatal, TEXT("[CrowdySDK] Failed to load GraphQL query database"));
	}

	QueryParser = new FCrowdyQueryParser();
	
	UE_LOG(LogTemp, Log, TEXT("[CrowdySDK]: Query Subsystem Initialized."))
	
	GetWorld()->GetTimerManager().SetTimer(
		StatsTimerHandle, this, &UCrowdyQuerySubsystem::UpdateStats, 1.0f, true);
}

void UCrowdyQuerySubsystem::SetAuthToken(const FString& InAuthToken)
{
	AuthToken = InAuthToken;
}

void UCrowdyQuerySubsystem::ClearAuthToken()
{
	AuthToken.Empty();
}

bool UCrowdyQuerySubsystem::HasAuthToken() const
{
	return !AuthToken.IsEmpty();
}

void UCrowdyQuerySubsystem::ExecuteQueryByID(const EGraphQLQuery QueryID,
                                             const TMap<FString, FString>& RuntimeVariables,
                                             const bool bIncludeAuthToken, const bool bUseNestedJson)
{
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, QueryID, RuntimeVariables, bIncludeAuthToken, bUseNestedJson]()
	{
		if (!IsValid(QueryDatabase))
		{
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
			{
				OnComplete.ExecuteIfBound(false, TEXT("{\"error\": \"Internal error: query database missing\"}"));
			}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
			return;
		}

		FGraphQLQueryDef QueryDef;
		if (!QueryDatabase->GetQueryByID(QueryID, QueryDef))
		{
			UE_LOG(LogTemp, Error, TEXT("GraphQL Service: Query not found for ID %d"),
			       static_cast<int32>(QueryID));
			UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
			{
				OnComplete.ExecuteIfBound(false, TEXT("{\"error\": \"Query not found\"}"));
			}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
			return;
		}

		// Merge variables
		const TSharedPtr<FJsonObject> FinalVariables = MakeShareable(new FJsonObject);

		// Start with default vars from the asset
		for (const auto& Pair : QueryDef.DefaultVariables)
		{
			FinalVariables->SetStringField(Pair.Key, Pair.Value);
		}

		if (!bUseNestedJson)
		{
			// Overwrite/append with runtime vars
			for (const auto& Pair : RuntimeVariables)
			{
				FinalVariables->SetStringField(Pair.Key, Pair.Value);
			}
		}
		else
		{
			for (const auto& Pair : RuntimeVariables)
			{
				const FString& DotKey = Pair.Key;
				const FString& Value = Pair.Value;

				TArray<FString> Keys;
				DotKey.ParseIntoArray(Keys, TEXT("."));

				TSharedPtr<FJsonObject> Current = FinalVariables;

				for (int32 i = 0; i < Keys.Num(); ++i)
				{
					const FString& Key = Keys[i];

					if (i == Keys.Num() - 1)
					{
						// Last key: set the value as number or string
						if (Value.IsNumeric())
						{
							// Try parse as int first, fallback to double if needed
							if (Value.IsNumeric() && !Value.Contains(TEXT(".")))
							{
								int64 IntVal = FCString::Atoi64(*Value);
								Current->SetNumberField(Key, static_cast<double>(IntVal));
							}
							else
							{
								double DoubleVal = FCString::Atod(*Value);
								Current->SetNumberField(Key, DoubleVal);
							}
						}
						else
						{
							Current->SetStringField(Key, Value);
						}
					}
					else
					{
						// Intermediate keys: descend or create nested object
						TSharedPtr<FJsonObject> Next;
						const TSharedPtr<FJsonObject>* ExistingPtr = nullptr;
						if (Current->TryGetObjectField(Key, ExistingPtr) && ExistingPtr && ExistingPtr->IsValid())
						{
							Next = *ExistingPtr;
						}
						else
						{
							Next = MakeShared<FJsonObject>();
							Current->SetObjectField(Key, Next);
						}
						Current = Next;
					}
				}
			}
		}

		// Delegate to the existing ExecuteGraphQLQuery
		ExecuteQuery(QueryDef.QueryBody, bIncludeAuthToken, FinalVariables);
	}, LowLevelTasks::ETaskPriority::BackgroundNormal);
}

void UCrowdyQuerySubsystem::SetEndpoint(const FString& InEndpoint)
{
	GraphQLEndpoint = InEndpoint;
	UE_LOG(LogTemp, Log, TEXT("[CrowdySDK]: GraphQL Endpoint set to %s"), *InEndpoint);
}

FString UCrowdyQuerySubsystem::GetCurrentEndpoint() const
{
	return GraphQLEndpoint;
}

FQueryStats UCrowdyQuerySubsystem::GetQueryStats() const
{
	FQueryStats QueryStats;
	QueryStats.bIsReceivingData = bIsReceivingData.load();
	QueryStats.QueriesSentPerSecond = QueriesSentPerSecond.load();
	QueryStats.QueryBytesSentPerSecond = QueryBytesSentPerSecond.load();
	QueryStats.ResponseBytesReceivedPerSecond = ResponseBytesReceivedPerSecond.load();
	QueryStats.ResponseReceivedPerSecond = ResponseReceivedPerSecond.load();
	return QueryStats;
}

void UCrowdyQuerySubsystem::UpdateStats()
{
	QueriesSentPerSecond.store(0, std::memory_order_relaxed);
	QueryBytesSentPerSecond.store(0, std::memory_order_relaxed);
	ResponseBytesReceivedPerSecond.store(0, std::memory_order_relaxed);
	ResponseReceivedPerSecond.store(0, std::memory_order_relaxed);
	bIsReceivingData.store(false);
}

void UCrowdyQuerySubsystem::ExecuteQuery(const FString& Query, const bool bIncludeAuthToken,
                                         const TSharedPtr<FJsonObject>& Variables)
{
	if (Query.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] GraphQL: Empty query provided"));
		UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
		{
			OnComplete.ExecuteIfBound(false, TEXT("{\"error\": \"Empty query provided\"}"));
		}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
		return;
	}

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] GraphQL: Failed to get HTTP module"));
		UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
		{
			OnComplete.ExecuteIfBound(false, TEXT("{\"error\": \"HTTP module not available\"}"));
		}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
		return;
	}

	const TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	
	if (GetCurrentEndpoint().IsEmpty())
	{
		const FString DevEndpoint = TEXT("https://dev-webapi.crowd.rocks/graphql");
		Request->SetURL(DevEndpoint);
	}
	else
	{
		Request->SetURL(GetCurrentEndpoint());
	}
	
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(120.0f);

	if (bIncludeAuthToken && HasAuthToken())
	{
		const FString AuthHeader = FString::Printf(TEXT("Bearer %s"), *AuthToken);
		Request->SetHeader(TEXT("Authorization"), AuthHeader);
		//UE_LOG(LogTemp, Log, TEXT("[CrowdySDK] GraphQL: Added authorization header"));
	}
	else if (bIncludeAuthToken && !HasAuthToken())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CrowdySDK] GraphQL: Auth requested but no token available"));
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("query"), Query);

	if (Variables.IsValid())
	{
		JsonObject->SetObjectField(TEXT("variables"), Variables);
	}

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	Request->SetContentAsString(OutputString);
	Request->OnProcessRequestComplete().BindUObject(this, &UCrowdyQuerySubsystem::OnHttpsRequestComplete);

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] GraphQL: Failed to process HTTP request"));
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnComplete.ExecuteIfBound(false, TEXT("{\"error\": \"Failed to process HTTP request\"}"));
		});
		return;
	}

	QueriesSentPerSecond.fetch_add(1, std::memory_order_relaxed);
	int32 QuerySize = 0;
	QuerySize = Request->GetVerb().Len();

	TArray<FString> Headers = Request->GetAllHeaders();
	for (const FString& Header : Headers)
	{
		QuerySize += Header.Len();
	}

	QueryBytesSentPerSecond.fetch_add(QuerySize, std::memory_order_relaxed);
}

void UCrowdyQuerySubsystem::OnHttpsRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                   bool bWasSuccessful)
{
	TWeakObjectPtr<UCrowdyQuerySubsystem> WeakThis = this;

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [WeakThis, bWasSuccessful, Response]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		if (!bWasSuccessful || !Response.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] GraphQL: HTTP request failed"));
			AsyncTask(ENamedThreads::GameThread, [WeakThis]()
			{
				WeakThis->OnComplete.ExecuteIfBound(false, TEXT("{\"error\": \"HTTP request failed\"}"));
			});
			return;
		}

		const int32 ResponseCode = Response->GetResponseCode();
		FString ResponseContent = Response->GetContentAsString();

		if (!WeakThis.IsValid())
			return;

		const bool bSuccess = (ResponseCode >= 200 && ResponseCode < 300);

		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] GraphQL: Server returned error code: %d"), ResponseCode);
			AsyncTask(ENamedThreads::GameThread, [WeakThis, ResponseContent]()
			{
				WeakThis->OnComplete.ExecuteIfBound(false, ResponseContent);
			});
			return;
		}

		WeakThis->ResponseReceivedPerSecond.fetch_add(1, std::memory_order_relaxed);
		WeakThis->ResponseBytesReceivedPerSecond.store(
			WeakThis->ResponseBytesReceivedPerSecond.load(std::memory_order_relaxed) + ResponseContent.Len(),
			std::memory_order_relaxed);
		WeakThis->bIsReceivingData.store(true, std::memory_order::memory_order_relaxed);


		WeakThis->ParseAndDispatchToServices(ResponseContent);

		AsyncTask(ENamedThreads::GameThread, [WeakThis, ResponseContent]()
		{
			WeakThis->OnComplete.ExecuteIfBound(true, ResponseContent);
		});
	}, LowLevelTasks::ETaskPriority::BackgroundNormal);
}

void UCrowdyQuerySubsystem::ParseAndDispatchToServices(const FString& ResponseContent) const
{
	TSharedPtr<FJsonObject> ParsedData;
	const EQueryResponseType ResponseType = QueryParser->ParseResponse(ResponseContent, ParsedData);

	if (ResponseType == EQueryResponseType::Error)
	{
		TArray<FString> ErrorMessages = QueryParser->GetErrorMessages(ParsedData);
		TArray<int32> ErrorCodes = QueryParser->GetErrorCodes(ParsedData);
		TArray<FString> ErrorPaths = QueryParser->GetErrorPaths(ParsedData);

		const EQueryResponseType ErrorResponseType = QueryParser->DetermineErrorResponseType(ParsedData);

		UE_LOG(LogTemp, Log, TEXT("[CrowdySDK] GraphQL Handling error for operation type: %d"),
		       static_cast<int32>(ErrorResponseType));

		// Log error details
		for (int32 i = 0; i < ErrorMessages.Num(); ++i)
		{
			FString ErrorMessage = ErrorMessages[i];
			const int32 ErrorCode = (i < ErrorCodes.Num()) ? ErrorCodes[i] : 0;
			FString ErrorPath = (i < ErrorPaths.Num()) ? ErrorPaths[i] : TEXT("unknown");

			UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] GraphQL Error - Path: %s, Code: %d, Message: %s"),
			       *ErrorPath, ErrorCode, *ErrorMessage);
		}

		const FString CleanMsg = ErrorMessages[0].TrimStartAndEnd();

		// Dispatch to the appropriate service based on an error type
		switch (ErrorResponseType)
		{
		case EQueryResponseType::Login:
			{
				const TSharedPtr<FLoginResponse> LoginResponse;
				LoginResponse->ParseResponse(ParsedData);
				// Pass To Service Handler
				break;
			}
		case EQueryResponseType::Register:
			{
				const TSharedPtr<FRegisterResponse> RegisterResponse;
				RegisterResponse->ParseResponse(ParsedData);
				// Pass to Service Handler
				break;
			}
		case EQueryResponseType::UDP_Info:
			{
				const TSharedPtr<FUDPAddressNotify> UDPInfoResponse;
				if (CleanMsg.Contains(TEXT("Insufficient"), ESearchCase::IgnoreCase))
				{
					UDPInfoResponse->bGateKeep = true;
				}
				if (UDPInfoResponse.IsValid())
				{
					UDPInfoResponse->ParseResponse(ParsedData);
				}
				// Pass to Handler
				break;
			}

		case EQueryResponseType::UpdateChunk: break;
		case EQueryResponseType::GetChunkByDistance: break;
		case EQueryResponseType::CreateAvatar: break;
		case EQueryResponseType::MyAvatars: break;
		case EQueryResponseType::UpdateAvatar: break;
		case EQueryResponseType::UpdateAvatarState: break;
		case EQueryResponseType::TeleportRequest:
			{
				const TSharedPtr<FTeleportResponse> TeleportResponse;
				TeleportResponse->ParseResponse(ParsedData);
				// Pass to handler
				break;
			}

		case EQueryResponseType::UpdateUserState:
			{
				//UE_LOG(LogGraphQLService, Error, TEXT("UpdateUserState error reponse received!"));
				//UserStateService->HandleUpdateUserStateResponse(ParsedData);
				break;
			}

		case EQueryResponseType::GetUserState:
			{
				//UE_LOG(LogGraphQLService, Error, TEXT("GetUserState error reponse received!"));
				//UserStateService->HandleGetUserStateResponse(ParsedData);
				break;
			}

		case ListVoxelUpdatesByDistance:
			{
				const TSharedPtr<FVoxelListByDistanceResponse> VoxelListByDistanceResponse;
				VoxelListByDistanceResponse->ParseResponse(ParsedData);
				//VoxelService->HandleVoxelListByDistanceResponse(ParsedData);
				break;
			}

		default:
			UE_LOG(LogTemp, Error, TEXT("Unknown error response type: %d"),
			       static_cast<int32>(ErrorResponseType));
			break;
		}

		return; // Exit early since we handled the error
	}

	TSharedPtr<ICrowdyQueryResponse> Response;

	switch (ResponseType)
	{
	case EQueryResponseType::Login:
		{
			TSharedPtr<FLoginResponse> LoginResponse = MakeShared<FLoginResponse>();
			LoginResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(LoginResponse);
			break;
		}
	case EQueryResponseType::Register:
		{
			TSharedPtr<FRegisterResponse> RegisterResponse = MakeShared<FRegisterResponse>();
			RegisterResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(RegisterResponse);
			break;
		}
	case EQueryResponseType::UDP_Info:
		{
			TSharedPtr<FUDPAddressNotify> UDPInfoResponse = MakeShared<FUDPAddressNotify>();
			UDPInfoResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UDPInfoResponse);
			break;
		}
	case EQueryResponseType::VoxelList:
		{
			TSharedPtr<FVoxelListResponse> VoxelListResponse = MakeShared<FVoxelListResponse>();
			VoxelListResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(VoxelListResponse);
			break;
		}
	case EQueryResponseType::ListVoxelUpdatesByDistance:
		{
			TSharedPtr<FVoxelListByDistanceResponse> VoxelListByDistanceResponse = MakeShared<
				FVoxelListByDistanceResponse>();
			VoxelListByDistanceResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(VoxelListByDistanceResponse);
			break;
		}
	case EQueryResponseType::TeleportRequest:
		{
			TSharedPtr<FTeleportResponse> TeleportResponse = MakeShared<FTeleportResponse>();
			TeleportResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(TeleportResponse);
			break;
		}
	case EQueryResponseType::VersionInfo:
		{
			TSharedPtr<FVersionInfoResponse> VersionInfoResponse = MakeShared<FVersionInfoResponse>();
			VersionInfoResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(VersionInfoResponse);
			break;
		}
	case EQueryResponseType::GetUserState:
		{
			TSharedPtr<FGetUserStateResponse> UserStateResponse = MakeShared<FGetUserStateResponse>();
			UserStateResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UserStateResponse);
			break;
		}
	case EQueryResponseType::UpdateUserState:
		{
			TSharedPtr<FUpdateUserStateResponse> UserStateResponse = MakeShared<FUpdateUserStateResponse>();
			UserStateResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UserStateResponse);
			break;
		}
	case EQueryResponseType::GetChunkByDistance:
		{
			TSharedPtr<FGetChunkResponse> GetChunkResponse = MakeShared<FGetChunkResponse>();
			GetChunkResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(GetChunkResponse);
			break;
		}
	case EQueryResponseType::UpdateChunk:
		{
			TSharedPtr<FUpdateChunkResponse> UpdateChunkResponse = MakeShared<FUpdateChunkResponse>();
			UpdateChunkResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UpdateChunkResponse);
			break;
		}
	
	// Not implemented right now
	case EQueryResponseType::CreateAvatar:
		{
			TSharedPtr<FAvatarCreateResponse> UpdateAvatarResponse = MakeShared<FAvatarCreateResponse>();
			UpdateAvatarResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UpdateAvatarResponse);
			break;
		}
	case EQueryResponseType::MyAvatars:
		{
			TSharedPtr<FFetchAvatarsResponse> FetchAvatarsResponse = MakeShared<FFetchAvatarsResponse>();
			FetchAvatarsResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(FetchAvatarsResponse);
			break;
		}
	case EQueryResponseType::UpdateAvatar:
		{
			TSharedPtr<FAvatarNameUpdateResponse> UpdateAvatarNameResponse = MakeShared<FAvatarNameUpdateResponse>();
			UpdateAvatarNameResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UpdateAvatarNameResponse);
			break;
		}
	case EQueryResponseType::UpdateAvatarState:
		{
			TSharedPtr<FAvatarStateUpdateResponse> UpdateAvatarStateResponse = MakeShared<FAvatarStateUpdateResponse>();
			UpdateAvatarStateResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(UpdateAvatarStateResponse);
			break;
		}
	case EQueryResponseType::DeleteAvatar:
		{
			TSharedPtr<FAvatarDeleteResponse> DeleteAvatarResponse = MakeShared<FAvatarDeleteResponse>();
			DeleteAvatarResponse->ParseResponse(ParsedData);
			Response = StaticCastSharedPtr<ICrowdyQueryResponse>(DeleteAvatarResponse);
			break;
		}
	
	default:
		break;
	}

	if (Response.IsValid() && DataRegistry)
	{
		DataRegistry->DispatchResponse(Response);
	}
}


