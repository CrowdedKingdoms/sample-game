// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/CrowdySDKSubsystem.h"
#include "Core/Audio/VoiceChat/VoiceChatSubsystem.h"
#include "Core/Audio/VoiceChat/Service/FVoiceChatService.h"
#include "Core/GraphQL/Enums/EQueryResponseType.h"
#include "Core/GraphQL/Interfaces/ICrowdyQueryRequest.h"
#include "Core/GraphQL/Interfaces/ICrowdyQueryResponse.h"
#include "Core/GraphQL/Interfaces/ICrowdyQueryTransmissionLayer.h"
#include "Core/UDP/Interfaces/ICrowdyMessage.h"
#include "Internal/FCrowdyServiceRegistry.h"
#include "Internal/FCrowdyDataRegistry.h"
#include "Messages/FPingTestMessage.h"
#include "Network/GraphQL/CrowdyQuerySubsystem.h"
#include "Network/UDP/FCrowdyTransmissionLayerUDP.h"
#include "Serialization/FCrowdyMessageParser.h"
#include "Subsystem/CrowdyGameSession.h"
#include "Subsystem/CrowdyWorkerThreadsSubsystem.h"
#include "Utils/FMessageBufferPool.h"
#include "Network/GraphQL/FCrowdyQueryTransmissionLayerGQL.h"
#include "Network/UDP/CrowdyUDPSubsystem.h"
#include "Queries/Authentication/FLoginRequest.h"
#include "Queries/Authentication/FLoginResponse.h"
#include "Queries/Authentication/FRegisterRequest.h"
#include "Queries/Authentication/FRegisterResponse.h"
#include "Queries/Data/Version/FVersionInfoRequest.h"
#include "Queries/Data/Version/FVersionInfoResponse.h"
#include "Queries/Permissions/FTeleportRequest.h"
#include "Queries/Permissions/FTeleportResponse.h"
#include "Queries/UDP/FUDPAddressNotify.h"
#include "Queries/UDP/FUDPAddressRequest.h"

void UCrowdySDKSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Dependencies first
	GameSession            = Collection.InitializeDependency<UCrowdyGameSession>();
	WorkerThreadsSubsystem = Collection.InitializeDependency<UCrowdyWorkerThreadsSubsystem>();
	QuerySubsystem         = Collection.InitializeDependency<UCrowdyQuerySubsystem>();
	UdpSubsystem           = Collection.InitializeDependency<UCrowdyUDPSubsystem>();

	// Pure allocations (no world required)
	ServiceRegistry = new FCrowdyServiceRegistry();
	DataRegistry    = new FCrowdyDataRegistry();
	Parser          = new FCrowdyMessageParser(ServiceRegistry, UdpSubsystem);
	BufferPool      = new FMessageBufferPool();

	const UWorld* World = GetWorld();
	if (!World || World->GetGameInstance() != GetGameInstance())
	{
		UE_LOG(LogTemp, Fatal, TEXT("UWorld is invalid while initializing."));
		return;
	}
		

	// Create anything that may depend on world/online/sockets
	TransmissionLayer      = new FCrowdyTransmissionLayerUDP(GameSession);
	QueryTransmissionLayer = new FCrowdyQueryTransmissionLayerGQL(QuerySubsystem);
	VoiceChatService       = new FVoiceChatService(this, GameSession);

	auto LogNull = [](const TCHAR* Name)
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] %s is null or invalid"), Name);
	};

	bool bFailed = false;
	bFailed |= !ServiceRegistry && (LogNull(TEXT("ServiceRegistry")), true);
	bFailed |= !DataRegistry && (LogNull(TEXT("DataRegistry")), true);
	bFailed |= !Parser && (LogNull(TEXT("Parser")), true);
	bFailed |= !TransmissionLayer && (LogNull(TEXT("TransmissionLayer")), true);
	bFailed |= !QueryTransmissionLayer && (LogNull(TEXT("QueryTransmissionLayer")), true);
	bFailed |= !BufferPool && (LogNull(TEXT("BufferPool")), true);
	bFailed |= !IsValid(WorkerThreadsSubsystem) && (LogNull(TEXT("WorkerThreadsSubsystem")), true);
	bFailed |= !IsValid(QuerySubsystem) && (LogNull(TEXT("QuerySubsystem")), true);
	bFailed |= !IsValid(UdpSubsystem) && (LogNull(TEXT("UdpSubsystem")), true);
	bFailed |= !IsValid(GameSession) && (LogNull(TEXT("GameSession")), true);
	bFailed |= !VoiceChatService && (LogNull(TEXT("VoiceChatService")), true);

	if (bFailed) return;

	WorkerThreadsSubsystem->InitializeWorkerThreadPool(BufferPool, Parser, ServiceRegistry, UdpSubsystem, GameSession);
	QuerySubsystem->InitializeQuerySubsystem(DataRegistry);

	RegisterQueryReceptionLayer(this);

	UdpSubsystem->OnUDPConnectionSuccessful.AddDynamic(this, &UCrowdySDKSubsystem::OnUDPConnectionSuccessful);
	UdpSubsystem->OnUDPTimeout.AddDynamic(this, &UCrowdySDKSubsystem::OnUDPTimeout);

	UE_LOG(LogTemp, Log, TEXT("[CrowdySDK] CrowdySDK Subsystem Initialized (PostWorldInit)"));
}

void UCrowdySDKSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	ServiceRegistry = nullptr;
	Parser = nullptr;
	TransmissionLayer = nullptr;
	Super::Deinitialize();
}

void UCrowdySDKSubsystem::Login(const FString Email, const FString Password) const
{
	FLoginRequest LoginRequest;
	LoginRequest.Email = Email;
	LoginRequest.Password = Password;
	LoginRequest.PrepareQuery();
	ExecuteQuery(LoginRequest);
}

void UCrowdySDKSubsystem::Register(const FString Email, const FString Password) const
{
	FRegisterRequest RegisterRequest;
	RegisterRequest.Email = Email;
	RegisterRequest.Password = Password;
	RegisterRequest.PrepareQuery();
	ExecuteQuery(RegisterRequest);
}

void UCrowdySDKSubsystem::Logout() const
{
	QuerySubsystem->ClearAuthToken();
	GameSession->ClearCurrentSessionData();
	OnLogout.Broadcast(true);
}

void UCrowdySDKSubsystem::SetGameSessionInfo(const FGameSessionInfo GameSessionInfo)
{
	GameSession->SetUserID(GameSessionInfo.UserID);
	GameSession->SetGameTokenID(GameSessionInfo.GameTokenID);
	GameSession->SetGameToken(GameSessionInfo.GameToken);
	QuerySubsystem->SetAuthToken(GameSessionInfo.GameToken);
}

void UCrowdySDKSubsystem::RequestUDPAccess() const
{
	FUDPAddressRequest UDPAddressRequest;
	UDPAddressRequest.PrepareQuery();
	ExecuteQuery(UDPAddressRequest);
}

void UCrowdySDKSubsystem::RequestVersionInfo() const
{
	FVersionInfoRequest VersionInfoRequest;
	VersionInfoRequest.PrepareQuery();
	ExecuteQuery(VersionInfoRequest);
}

void UCrowdySDKSubsystem::SetQueryEndpoint(const FString InEndpoint) const
{
	QuerySubsystem->SetEndpoint(InEndpoint);
}

void UCrowdySDKSubsystem::StartUDPTimeoutMonitoring(const float ThresholdSeconds)
{
	UdpSubsystem->StartTimeoutMonitoring(ThresholdSeconds);
	
	if (!bIsRegistered)
	{
		RegisterReceptionLayer(this);
		bIsRegistered = true;
	}
	
	if (!PingMessageTimerHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(PingMessageTimerHandle, 
			this, 
			&UCrowdySDKSubsystem::SendPingTestMessage, 
			5.0f, true, 5.0f);
}

void UCrowdySDKSubsystem::StopUDPTimeoutMonitoring()
{
	UdpSubsystem->StopTimeoutMonitoring();
	
	if (PingMessageTimerHandle.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(PingMessageTimerHandle);
}

void UCrowdySDKSubsystem::StopNetworkOperations() const
{
	UdpSubsystem->StopUDPOperations();
}

void UCrowdySDKSubsystem::ToggleNetworkMessageProcessing() const
{
	UdpSubsystem->ToggleUDPMessageProcessing();
}

void UCrowdySDKSubsystem::TriggerUdpHeartbeat() const
{
	UdpSubsystem->OnMessageReceived();
}

void UCrowdySDKSubsystem::StartVoiceChat()
{
	if (ValidateVoiceChatSubsystem())
		VoiceChatSubsystem->StartVoiceChat();
	
	if (!IsLayerRegistered(VoiceChatService))
		RegisterReceptionLayer(VoiceChatService);
}

void UCrowdySDKSubsystem::StopVoiceChat()
{
	if (ValidateVoiceChatSubsystem())
		VoiceChatSubsystem->StopVoiceChat();
	
	if (!IsLayerRegistered(VoiceChatService))
		RegisterReceptionLayer(VoiceChatService);
}

void UCrowdySDKSubsystem::PlayVoiceChat()
{
	if (ValidateVoiceChatSubsystem())
		VoiceChatSubsystem->PlayVoiceChat();
	
	if (!IsLayerRegistered(VoiceChatService))
		RegisterReceptionLayer(VoiceChatService);
}

void UCrowdySDKSubsystem::MuteVoiceChat()
{
	if (ValidateVoiceChatSubsystem())
		VoiceChatSubsystem->MuteVoiceChat();
	
	if (!IsLayerRegistered(VoiceChatService))
		RegisterReceptionLayer(VoiceChatService);
}

void UCrowdySDKSubsystem::SetVoiceChatStreamTimeoutThreshold(const float InSeconds)
{
	if (ValidateVoiceChatSubsystem())
		VoiceChatSubsystem->SetStreamTimeoutThreshold(InSeconds);
}

void UCrowdySDKSubsystem::ToggleOwnerEcho(const bool bEnable) const
{
	VoiceChatService->ToggleOwnerEcho(bEnable);
}

void UCrowdySDKSubsystem::RequestTeleportPermission(const int64 ChunkX, const int64 ChunkY, const int64 ChunkZ,
                                                    const int32 VoxelX, const int32 VoxelY, const int32 VoxelZ) const
{
	FTeleportRequest TeleportRequest;
	TeleportRequest.MapID = GameSession->GetAppID();
	TeleportRequest.UUID = GameSession->GetUUID();
	TeleportRequest.ChunkX = ChunkX;
	TeleportRequest.ChunkY = ChunkY;
	TeleportRequest.ChunkZ = ChunkZ;
	TeleportRequest.VoxelX = VoxelX;
	TeleportRequest.VoxelY = VoxelY;
	TeleportRequest.VoxelZ = VoxelZ;
	TeleportRequest.PrepareQuery();
	ExecuteQuery(TeleportRequest);
}

void UCrowdySDKSubsystem::DeregisterAllReceptionLayers()
{
	ServiceRegistry->DeregisterAllReceptionLayers();
	bIsRegistered = false;
}

void UCrowdySDKSubsystem::SetExpectedActorUpdateStateSize(const int32 InSize) const
{
	UE_LOG(LogTemp, Log, TEXT("[CrowdySDK] Expected Actor Update State Size: %d"), InSize);
	Parser->SetExpectedActorStateSize(InSize);
}	

void UCrowdySDKSubsystem::HandleLogin(const FLoginResponse& LoginResponse) const
{
	GameSession->SetUserID(LoginResponse.UserID);
	GameSession->SetGameTokenID(LoginResponse.GameTokenID);
	GameSession->SetGameToken(LoginResponse.GameToken);
	QuerySubsystem->SetAuthToken(LoginResponse.GameToken);

	FUDPAddressRequest UDPAddressRequest;
	UDPAddressRequest.PrepareQuery();
	ExecuteQuery(UDPAddressRequest);

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, LoginResponse]()
	{
		OnLogin.Broadcast(true, LoginResponse.GameToken);
	}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
}

void UCrowdySDKSubsystem::HandleRegister(const FRegisterResponse& RegisterResponse) const
{
	GameSession->SetUserID(RegisterResponse.UserID);
	GameSession->SetGameToken(RegisterResponse.GameToken);
	QuerySubsystem->SetAuthToken(RegisterResponse.GameToken);

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, RegisterResponse]()
	{
		OnRegister.Broadcast(true, RegisterResponse.GameToken);
	}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
}

void UCrowdySDKSubsystem::HandleUDPAddressNotify(const FUDPAddressNotify& UDPAddressNotify) const
{
	if (UDPAddressNotify.bGateKeep)
	{
		UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]()
		{
			OnUDPAddressNotify.Broadcast(true, true);
		}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);

		return;
	}

	bool bSuccess = UdpSubsystem->InitializeUDP(UDPAddressNotify);

	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, bSuccess]()
	{
		OnUDPAddressNotify.Broadcast(bSuccess, false);
	}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
}

void UCrowdySDKSubsystem::HandleVersionInfoResponse(const FVersionInfoResponse& VersionInfoResponse) const
{
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, VersionInfoResponse]()
	{
		OnVersionInfo.Broadcast(VersionInfoResponse.ServerVersion, VersionInfoResponse.MinimumClientVersion);
	}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
}

void UCrowdySDKSubsystem::HandleTeleportPermissionResponse(const FTeleportResponse& TeleportResponse) const
{
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, TeleportResponse]()
	{
		OnTeleportPermission.Broadcast(TeleportResponse.bTeleportAllowed);
	}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);
}

bool UCrowdySDKSubsystem::ValidateVoiceChatSubsystem()
{
	if (IsValid(VoiceChatSubsystem))
	{
		return true;
	}

	if (const UWorld* World = GetWorld())
	{
		VoiceChatSubsystem = World->GetSubsystem<UVoiceChatSubsystem>();
	}

	VoiceChatSubsystem->InitializeVoiceChatSubsystem(VoiceChatService);
	VoiceChatService->SetVoiceChatManagerReference(VoiceChatSubsystem);
	return IsValid(VoiceChatSubsystem);
}

void UCrowdySDKSubsystem::OnUDPTimeout()
{
	OnUDPTimedOut.Broadcast();
}

void UCrowdySDKSubsystem::OnUDPConnectionSuccessful()
{
	OnUDPConnectionSuccess.Broadcast();
}

void UCrowdySDKSubsystem::SendPingTestMessage()
{
	const FInt64Vector ChunkCoordinate = GameSession->GetPlayerCurrentChunkCoordinates();
	
	FPingTestMessage PingTestMessage;
	PingTestMessage.UUID = GameSession->GetUUID();
	PingTestMessage.AppID = GameSession->GetAppID();
	PingTestMessage.ChunkX = ChunkCoordinate.X;
	PingTestMessage.ChunkY = ChunkCoordinate.Y;
	PingTestMessage.ChunkZ = ChunkCoordinate.Z;
	PingTestMessage.DecayRate = ECrowdyDecayRate::Exponential_Decay;
	PingTestMessage.ReplicationDistance = ECrowdyReplicationDistance::One_Chunk;
	SendMessage(PingTestMessage);
}

void UCrowdySDKSubsystem::RegisterReceptionLayer(ICrowdyReceptionLayer* Layer) const
{
	if (!Layer)
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] Candidate Reception Layer is null."));
		return;
	}

	if (!ServiceRegistry)
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] Service Registry is null."));
		return;
	}

	ServiceRegistry->RegisterReceptionLayer(Layer);
}

void UCrowdySDKSubsystem::RegisterQueryReceptionLayer(ICrowdyQueryReceptionLayer* LayerToRegister) const
{
	if (!LayerToRegister)
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] Candidate Query Reception Layer is null."));
		return;
	}

	if (!DataRegistry)
	{
		UE_LOG(LogTemp, Error, TEXT("[CrowdySDK] Data Registry is null."));
		return;
	}

	DataRegistry->RegisterLayer(LayerToRegister);
}

bool UCrowdySDKSubsystem::IsLayerRegistered(const ICrowdyReceptionLayer* Layer) const
{
	return ServiceRegistry->IsLayerRegistered(Layer);
}

void UCrowdySDKSubsystem::SendMessage(const ICrowdyMessage& Message) const
{
	TArray<uint8> Bytes = Message.Serialize();

	if (Bytes.IsEmpty())
		return;

	if (TransmissionLayer)
	{
		TransmissionLayer->SendBytes(MoveTemp(Bytes), Message.bContainsAuth, Message.SequenceNumber);
	}
}

void UCrowdySDKSubsystem::ExecuteQuery(ICrowdyQueryRequest& Query) const
{
	UE_LOG(LogTemp, Log, TEXT("[CrowdySDK]: Execute Query:%s"), *Query.GetOperationName().ToString());
	QueryTransmissionLayer->ExecuteQuery(Query);
}

void UCrowdySDKSubsystem::SetTransmissionLayer(ICrowdyTransmissionLayer* Layer)
{
	TransmissionLayer = Layer;
}

void UCrowdySDKSubsystem::SetQueryTransmissionLayer(ICrowdyQueryTransmissionLayer* Layer)
{
	QueryTransmissionLayer = Layer;
}

void UCrowdySDKSubsystem::OnResponseReceived(TSharedPtr<ICrowdyQueryResponse> Response)
{
	if (!Response->IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *Response->GetError())
		UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Response]()
		{
			switch (Response->GetResponseType())
			{
			case EQueryResponseType::Register:
				{
					OnRegister.Broadcast(false, "");
					break;
				}
			case EQueryResponseType::UDP_Info:
				{
					OnUDPAddressNotify.Broadcast(false, false);
					break;
				}
			case EQueryResponseType::Login:
				{
					OnLogin.Broadcast(false, "");
					break;
				}
			case EQueryResponseType::TeleportRequest:
				{
					OnTeleportPermission.Broadcast(false);
					break;
				}
			default:
				break;
			}
		}, LowLevelTasks::ETaskPriority::Normal, UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri);

		return;
	}

	switch (Response->GetResponseType())
	{
	case EQueryResponseType::Login:
		{
			const FLoginResponse LoginResponse = static_cast<FLoginResponse&>(*Response);
			HandleLogin(LoginResponse);
			break;
		}
	case EQueryResponseType::Register:
		{
			const FRegisterResponse RegisterResponse = static_cast<FRegisterResponse&>(*Response);
			HandleRegister(RegisterResponse);
			break;
		}
	case EQueryResponseType::UDP_Info:
		{
			const FUDPAddressNotify UDPAddressNotify = static_cast<FUDPAddressNotify&>(*Response);
			HandleUDPAddressNotify(UDPAddressNotify);
			break;
		}
	case EQueryResponseType::VersionInfo:
		{
			const FVersionInfoResponse VersionInfoResponse = static_cast<FVersionInfoResponse&>(*Response);
			HandleVersionInfoResponse(VersionInfoResponse);
			break;
		}
	case EQueryResponseType::TeleportRequest:
		{
			const FTeleportResponse TeleportResponse = static_cast<FTeleportResponse&>(*Response);
			HandleTeleportPermissionResponse(TeleportResponse);
			break;
		}
		
	default:
		UE_LOG(LogTemp, Warning, TEXT("[CrowdySDK] Unknown response received:%s"),
		       *Response->GetOperationName().ToString())
	}
}

TArray<EQueryResponseType> UCrowdySDKSubsystem::GetSupportedResponseType() const
{
	return
	{
		EQueryResponseType::Register,
		EQueryResponseType::Login,
		EQueryResponseType::UDP_Info,
		EQueryResponseType::VersionInfo,
		EQueryResponseType::TeleportRequest
	};
}

void UCrowdySDKSubsystem::OnMessageReceived(TSharedRef<ICrowdyMessage> Message)
{
	switch (Message->GetType())
	{
	case ECrowdyMessageType::GENERIC_SPATIAL_1:
		{
			const auto& PingTestMessage = static_cast<const FPingTestMessage&>(*Message);
			
			if (PingTestMessage.UUID != GameSession->GetUUID())
			{
				return;
			}
			
			const int64 PingTimeMs = PingTestMessage.ReceiveTime - PingTestMessage.SendTime;
			UdpSubsystem->UpdatePingTime(PingTimeMs);
			break;
		}
	default:
		break;
	}
}

TArray<ECrowdyMessageType> UCrowdySDKSubsystem::GetSupportedResponseTypes() const
{
	return {ECrowdyMessageType::GENERIC_SPATIAL_1};
}
