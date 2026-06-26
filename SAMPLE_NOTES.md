# Crowdy SDK Sample - Work Notes

Status of the new-API examples added to this sample. Pick up from here.

## What was done

Rebuilt the sample's examples on the current CrowdySDK RPC API. All new code lives in
`Source/CrowdySDKTest/Public/Sample/` and `Private/Sample/`. The old-API files (see
"Legacy" below) were left in place.

### Interaction pattern
- `ISampleInteractable` - one method `Interact(APawn*)`.
- `ASampleSwitchBase` - trigger box; calls `Interact` on overlap (debounced). Every example subclasses it.
- `SampleTypes.h` - shared structs: `FSampleEntityState` (Dynamic snapshot), `FSampleSpawnInfo` (spawn InitialState), `FSampleProgress` (`meta=(CrowdyPersistent)`), `FSampleItem`. `ESampleAnimState` reused from `Core/Enums`.

### Examples (each = a switch)
| Switch | Shows |
| --- | --- |
| `ASampleMimicSwitch` + `ASampleMimicEntity` + `USampleTransformExecutor` | Dynamic entity, `UActorUpdateExecutor` (`GetActorState`/`GetStateStruct`), mirrored via `UMathOperations::GetMimicTransform` |
| `ASampleObjectSwitch` + `ASampleObjectEntity` | spawn -> RPC (`SetObjectLocation(FVector)`/`SetObjectRotation(FRotator)`, direct params) -> destroy |
| `ASampleGhostSwitch` | true round-trip ghost via `UCrowdyActorTracker::ToggleOwnerTracking` (needs player as a Dynamic entity + owner tracking on) |
| `ASampleVoiceSwitch` | voice toggle on `UCrowdySDKSubsystem` (`Start/Stop/Play/MuteVoiceChat`) |
| `ASampleChannelSwitch` | Multicast RPC over channel `SampleWorldChat`, rich params (`FString`, enum, `TArray<int32>`, `TSubclassOf`). Self-provisions the channel on first use: looks it up, creates it (Open policy) if missing, joins, registers it for reliable RPC (`UCrowdyChannels::RegisterReliableRpcChannel`), then announces. No manual Studio setup. |
| `ASampleHostSwitch` | host-gated spawn via `UCrowdyUtilities::CrowdyHasAuthority` |
| `ASamplePersistenceSwitch` | `PushState`/`PullState` of `FSampleProgress` (subject = player) |
| `ASampleTeamsSwitch` | `UCrowdyTeams`: self-provisions a team by `TeamName` (look up -> `CreateTeam` Open if missing -> `JoinTeam`); re-interact toggles `LeaveTeam`; cache-first membership via `IsPlayerInTeam` + `OnMyTeamsCacheChanged` |

Key point reflected from the docs: teams and channels are fully authorable at runtime
(create/update/delete/roles/policy on the subsystems), not just in CrowdyStudio.

### Config
- `CrowdySDKTest.Build.cs` - added private deps `CrowdySDK`, `CrowdyReplication`, `CrowdyServices`, `CrowdyNet`.
- `Config/DefaultGame.ini` - migrated to `[/Script/CrowdyReplication.CrowdySDKDeveloperSettings]`; removed old event-payload/executor-registry keys; added `DefaultProfile=/Game/Data/DA_SampleMapProfile.DA_SampleMapProfile`.

## Manual steps still needed (cannot be code-generated)
1. Compile the `CrowdySDKTest` module in-editor. Code was written against the bundled plugin headers but not built here.
2. Create `DA_SampleMapProfile` (a `UCrowdyMapProfile` data asset): under `ActorManagement`, set `BackendClass = UCrowdyActorPoolBackend` and `BackendConfig` to an instanced `UCrowdyActorPoolBackendConfig`; on that config set **`ReplicationPolicyClass = USampleTransformReplicationPolicy`** (the SDK base policy is Abstract, so this concrete sample subclass is the only thing that appears in the dropdown; if it's null the backend init bails and replication silently no-ops). `PoolPolicyClass` is optional, leave it empty (the pool subsystem falls back to the base `UCrowdyActorPoolPolicy`; that base was marked `Abstract` and crashed the fallback with a checkf, so it is now concrete). Turn `bUseAutoReplicator` on. Assign the profile as `DefaultProfile`. REQUIRED.
3. Place one of each switch actor in `Level_World`, spaced out. Each switch now draws itself (colored pedestal + floating label naming what it demos via `SwitchLabel`/`SwitchColor` on `ASampleSwitchBase`), so no marker setup is needed. Optional per-instance tweaks: `TeamName` (Teams), `Message` (Channel RPC), `SpawnOffset` (Object). `MimicClass`/`ObjectClass`/`WorldObjectClass` already default to working C++ classes; only set them if you make Blueprint subclasses with custom art.
4. For the ghost: ensure the player pawn carries a Dynamic `UCrowdyEntityComponent`. (The mimic does NOT need this — it spawns its own entity that bakes the player's mannequin mesh + anim BP as a class default, so the round-tripped reflection looks like your pawn on every client.)
5. Blueprint versions are now built in `Content/Interactions/` (BP_VoiceSwitch, BP_GhostSwitch,
   BP_HostSwitch, BP_ObjectSwitch, BP_MimicSwitch, BP_ChannelSwitch + BP_ObjectEntity,
   BP_MimicEntity). Voice/Ghost/Host are complete; the CrowdyEvent-bearing ones
   (Object/Mimic/Channel) have a few manual finishing steps the editor MCP can't do
   (typed custom-event input pins + the Crowdy Replicates toggle + channel-delegate
   wiring). See `SAMPLE_BLUEPRINT_MANUAL_STEPS.md` for the exact per-asset steps.

Visuals are code-driven now: switches self-draw, `ASampleObjectEntity` ships a default cube, and `ASampleMimicEntity` wears the interacting player's appearance. BP subclasses are only needed to override art.

## Legacy (remove once Content BPs are migrated)
Old-API C++ still present and compiling: `SamplePawnManager`, `ReplicatedActor` (interface),
and the `meta=(CrowdyRep=...)` structs (`FChangeAnimState`, `FSampleSetObjectLocation/Rotation`,
`FSampleActorState`). Content BPs (`BP_SampleReceptionLayer`, `BP_AUE_*`, `BP_MimicMe`, etc.)
still reference them, so do not delete the C++ until those BPs are reworked.

## Testing
- Single client: `crowdy.rpc.loopback 1` + `crowdy.rpc.trace 1` to exercise RPC round-trips.
- 2-client PIE (needs the map profile): walk into each switch and confirm it mirrors on the other client.
- Trace CVars per area: `crowdy.entity.trace`, `crowdy.rpc.trace`, `crowdy.services.trace`, `crowdy.voice.trace`, etc.

## Docs
Full guide lives in the sibling repo `cks-docs/docs-unreal-sdk/` (Docusaurus, builds green).
The sample tour is `cks-docs/docs-unreal-sdk/guides/sample-project.md`.
