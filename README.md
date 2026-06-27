# crowdy-sdk-sample

A minimal Unreal Engine 5 project that demonstrates the CrowdySDK plugin end to end.
Each feature is one self-contained "switch" example: walk into a pedestal, it runs one
SDK flow. There is a C++ set and a parallel Blueprint set kept at feature parity.

## Prerequisites

- Unreal Engine 5.8
- Visual Studio 2022 (Desktop C++ workload)
- CrowdySDK credentials (server endpoint or just use the one provided. Contact support if it doesn't work)

## Quick Start

**1. Generate project files**

Right-click `CrowdySDKTest.uproject` and select "Generate Visual Studio project files",
then open `CrowdySDKTest.sln` and build the project.

**2. Create an account on Crowded Kingdoms**

- We already have provided a default server endpoint in `Config/DefaultGame.ini`.
- You can reach us out to in the support to sign up for a free account. The dev boxes are ephemeral and we change them from time to time.

Without this step you will not be able to connect to the server.

**3. Place example switches in the level**

The example switches are already placed in the level. You can modify them, create your own logic, test new ones with it. 100% of the code is in the `Sample` folder.

**4. Configure credentials**

Use the Crowdy Studio to configure through your own endpoints, or use the one that is already provided. You just have to sign up for a free account first. 

## Examples

| Switch | What it shows                                                                                                                           |
|---|-----------------------------------------------------------------------------------------------------------------------------------------|
| `ASampleMimicSwitch` | Dynamic entity streaming: continuous actor state via `UActorUpdateExecutor` + mirrored reflection                                       |
| `ASampleObjectSwitch` | Spawn -> RPC -> destroy flow with direct-param RPCs (`FVector`, `FRotator`)                                                             |
| `ASampleGhostSwitch` | Round-trip ghost via `UCrowdyActorTracker::ToggleOwnerTracking`                                                                         |
| `ASampleVoiceSwitch` | Voice chat toggle (`Start/Stop/Play/MuteVoiceChat` on `UCrowdySDKSubsystem`)                                                            |
| `ASampleChannelSwitch` | Multicast RPC over a named channel; self-provisions the channel on first use                                                            |
| `ASampleHostSwitch` | Host-gated spawn via `UCrowdyUtilities::CrowdyHasAuthority`                                                                             |
| `ASamplePersistenceSwitch` | `PushState`/`PullState` of a persistent struct (`FSampleProgress`). This might fail if you don't have permissions to do so on the grid. |
| `ASampleTeamsSwitch` | Self-provisioning team join/leave via `UCrowdyTeams`                                                                                    |

Blueprint equivalents live in `Content/Interactions/` (prefixed `BP_`).

## Two replication paths

CrowdySDK has two transports within the realtime plane. Do not mix them.

- **Actor State (Dynamic entity):** `UCrowdyEntityComponent` in Dynamic mode plus a
  `UActorUpdateExecutor` subclass. Use for continuous state (movement, locomotion).
- **Event RPC (`CrowdyEvent`):** `UFUNCTION(meta=(CrowdyEvent, CrowdyRecipient=...))` +
  the `CROWDY_EVENT(Fn)` macro in C++, or a Custom Event with "Crowdy Replicates" on in
  Blueprint. Use for discrete one-shot actions.

The rule: continuous/self-healing state goes in Actor State; discrete one-shots go in RPCs.

## Testing

Single-client round-trip:

```
crowdy.rpc.loopback 1
crowdy.rpc.trace 1
```

Two-client PIE exercises full routing and state replication. Walk into each switch and
confirm the effect mirrors on the other client.

Useful trace CVars:

```
crowdy.entity.trace 1
crowdy.rpc.trace 1
crowdy.services.trace 1
crowdy.voice.trace 1
```

## Project layout

```
Source/CrowdySDKTest/
  Public/Sample/     -- example headers
  Private/Sample/    -- example implementations
Plugins/CrowdySDK/   -- local SDK copy compiled by this project
Content/Interactions/ -- Blueprint examples
Config/DefaultGame.ini -- SDK settings + map profile registration
```

Key source files:

| File | Purpose |
|---|---|
| `Sample/SampleTypes.h` | Shared structs: `FSampleEntityState`, `FSampleSpawnInfo`, `FSampleProgress`, `FSampleItem` |
| `Sample/SampleTransformExecutor.*` | Player pawn executor: snapshots transform + movement inputs |
| `Sample/SampleTransformReplicationPolicy.*` | Remote apply: interpolates transform, latches anim inputs |
| `Sample/SampleAnimReceiver.h` | Interface from replication policy to character AnimBP |
| `Sample/SampleMimicEntity.*` | Invisible mirror-source entity (puppeteer) |
| `Sample/SampleBlueprintHelpers.*` | BP bridges for struct type literals and proxy class registration |

## For Further reading, troubleshooting, support or reporting bugs
- [Crowded Kingdoms Documentation](https://docs.crowdedkingdoms.com/)
- [CrowdySDK Documentation](https://docs.crowdedkingdoms.com/unreal-sdk/intro)
- [Crowded Kingdoms Discord](https://discord.gg/crowdedkingdoms)

## Future work

The CrowdySDK v2.0 provides a big leap in terms of usability with Unreak Engine 5. We plan on expanding this idea further by adding more features and improving the developer experience.
You can provide feedback and suggestions on our [Discord](https://discord.gg/crowdedkingdoms).
With that said, we are currently working hard on bringing new features to the SDK that include:
- Mass Entity Based RPCs, this can act as a direct replacement for the Actor Pool System and the default rendering backend.
- Crowdy State which would allow us to do direct property replication
- Game Model integration, which allows us to have server-authoritative game logic
- Developer and Designer friendly tools which help with authoring and debugging
