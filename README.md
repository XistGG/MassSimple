# UE5 Mass Simple C++ Example

| [<img src="MassSimple.png">](https://github.com/XistGG/MassSimple) | `MassSimple` is a simple example of how to do basic things in UE5 Mass C++.<br/><br/>Target UE Version: **5.7** |
|---|---|

This is a flat 2D Top-Down Third Person Game.

The player can move the Pawn around the small plane by clicking the mouse on the ground.

Ephemeral Wisps follow the Player Pawn around.

Trees are born, grow larger over time, and die of old age.

This simple example of a UE5 Mass C++ project:

- Conceptualizes a Game "Entity Registry"
	- Entities Tagged as Registry-relevant receive "On Created" and "On Destroyed" Gameplay Events
	- Entities can optionally be assigned metadata, like Meta Type *(rock, tree, wisp, ...etc...)*
- Demonstrates some methods to:
	- Build and Destroy Mass Entities
    - Read and Write Mass Entity data
	- Safely Receive data ingress from multithreaded Mass processors
- Provides a functional, *non-performant* alternative Representation system
	- Entities tagged for Representation are drawn as tiles on a Render Target
		- they otherwise exist purely as Mass Entity data with no other World representation
- Utilizes some brute-force methodologies in the interest of code simplicity

Does **NOT**:

- use **Any** default UE5 Mass Fragments
	- or Gameplay Processors
	- or the default UE5 Mass Representation system
- use Blueprints, Data Assets or any other way to move relevant learning code/config out of c++/ini
	- If you read the C++ you see everything that's happening here

Future Roadmap:

- Demonstrate Writing Mass Entity Data from Gameplay Code
- Add more Reading Mass Entity Data from Gameplay Code examples
- Demonstrate Adding/Removing Entity Representation Tags at Runtime
- Demonstrate Building Entities in Processors

Detailed performance profiling, analysis and optimization is beyond the scope of this example.

There are many ways you can improve the efficiency of this code, each of which will make it more
complex and more difficult to understand.
I expect that you will optimize your code to your own specifications.

# Project Overview

- [Entity Registry](#entity-registry)
	- [Registry Subsystem](#registry-subsystem-uxmsregistrysubsystem)
    - [Registry Processors](#entity-registry-processors)
    - [Registry Listeners](#entity-registry-listeners)
- [Entity Builders](#entity-builders)
	- [UXmsEntityBuilderComponent](#actor-component-uxmsentitybuildercomponent)
	- [AXmsEntityBuilder](#actor-axmsentitytreebuilder)
- [Entity Attributes](#entity-attributes)
    - [Lifespan Attribute](#lifespan-attribute)
- [Entity Representation](#entity-representation)
	- [Representation Subsystem](#representation-subsystemuxmsrepsubsystem)
	- [Representation Processors](#representation-processors)
- [Game Setup](#game-setup)
- [Miscellaneous Thoughts](#miscellaneous-thoughts)
    - [Naming Conventions](#naming-conventions)

## Entity Registry

The Xms Entity Registry is a project-specific look at how your
Game code might keep track of interesting Entities that it needs to be aware of
for whatever reason.

This allows you to create and destroy Entities from any code, Gameplay or processor,
on any thread. It will automatically detect these events and provide access to them
for the Game thread.

There may be many more thousands of Entities existing internally in Mass that the Game code doesn't care about,
and none of those should be configured to be tracked by the Registry.

For each Entity that we care to Register, we will remember its `EntityHandle` and some Meta Data.

- The Subsystem is effectively the persistent memory container for the Registry
  - Provides thread-safe reads via `GetEntitiesByMetaType`
- We register `CreateEntity` and `DestroyEntity` Observer Processors
  - only for Entities tagged with `FXmsT_Registry`

### Registry Subsystem: `UXmsRegistrySubsystem`
Source Code:
[ [h](Source/Xms/EntityRegistry/XmsEntityRegistry.h)
| [cpp](Source/Xms/EntityRegistry/XmsEntityRegistry.cpp)
]

Using `TMassExternalSubsystemTraits`:

| Trait | Value | Notes |
|---|---|---|
| `GameThreadOnly` | `false` | YES read thread-safe |
| `ThreadSafeWrite` | `true` | YES write thread-safe |

#### Registry Subsystem Requirements

Notice that this subsystem imposes some requirements on its use.

1. You must only subscribe/unsubscribe while on the Game thread
2. Any code executed in response to events broadcast by this Subsystem **must not** try to subscribe/unsubscribe
   Event delegates or **the game will deadlock**.
   - TLDR: make sure to subcribe/unsubscribe using non-Event-triggered code

Currently there is only one simple accessor to get Entity data, `GetEntitiesByMetaType`,
which is read-safe from any thread.

### Entity Registry Processors
Source Code:
[ [h](Source/Xms/EntityRegistry/XmsEntityRegistryProcessors.h)
| [cpp](Source/Xms/EntityRegistry/XmsEntityRegistryProcessors.cpp)
]

These processors DO NOT use parallelization,
because they are typically executed with small numbers of Entities in very small Contexts.

Thus, the overhead of parallelization is likely not worth it here.
Your mileage may vary.

- `UXmsEntityCreated`
	- Observes Entities created with `FXmsT_Registry` Tag
	- Executes `UXmsRegistrySubsystem::MassOnEntitiesCreated` **from ANY thread**
- `UXmsEntityDestroyed`
	- Observes Entities created with `FXmsT_Registry` Tag
	- Executes `UXmsRegistrySubsystem::MassOnEntitiesDestroyed` **from ANY thread**

### Entity Meta Type: `EXmsEntityMetaType`
Source Code: [ [h](Source/Xms/EntityRegistry/XmsEntityMetaData.h) ]

An example Meta Type. Here we have just Rock, Tree and Wisp.

Currently we don't use the `Rock`.

`Tree` Entities are automatically built periodically by the
[AXmsEntityTreeBuilder](#actor-axmsentitytreebuilder).

`Wisp` Entities are automatically built periodically by a
[UXmsEntityBuilderComponent](#actor-component-uxmsentitybuildercomponent)
attached to the Player Pawn.

Replace these ideas with whatever is relevant for your game.

### Entity Meta Data: `FXmsCSF_MetaData`
Source Code: [ [h](Source/Xms/EntityRegistry/XmsEntityMetaData.h) ]

For now the Meta Data consists of only a single piece of information: the Meta Type.

You can easily imagine adding other meta-properties as needed for your game.

### Registry Tag: `FXmsT_Registry`
Source Code: [ [h](Source/Xms/EntityRegistry/XmsEntityRegistry.h) ]

- Presence of this tag identifies an Entity that the Registry will monitor
    - The tag **must be present** in Entity Build configuration to trigger the Entity `OnCreated` event

### Entity Registry Listeners

The idea here is you may have some Gameplay Actors or Widgets that need to listen for
Entity Registry events and react to them in some way.

We provide the base class `AXmsEntityRegistryListener`
and a concrete example `AXmsEntityRegistryListener_Wisp`.

You can use this same methodology in `UObject` or other code as needed.

#### Abstract Base Class: `AXmsEntityRegistryListener`
Source Code:
[ [h](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener.h)
| [cpp](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener.cpp)
]

- Implements base listener functionality
    - Any Registry-wide Entity Create or Destroy Events matching `ObservedMetaTypes` will trigger "On Observed" events
- Provides pure virtual methods derived classes must implement:
    - `NativeOnObservedEntitiesCreated`
    - `NativeOnObservedEntitiesDestroyed`

#### Example Implementation: `AXmsEntityRegistryListener_Wisp`
Source Code:
[ [h](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener_Wisp.h)
| [cpp](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener_Wisp.cpp)
]

An Actor of this class is present in the `L_Default` World Level.
*(See the UEditor Scene Outliner).*

When its `bObserveEntities == true`, the Actor responds to "Wisp Entity Created" events by spawning
a Niagara system at the `Wisp` World Location.

- Implements `NativeOnObservedEntitiesCreated` to spawn a Niagara System at the `Wisp` World Location
- Empty implementation of `NativeOnObservedEntitiesDestroyed` with comments explaining what you could do there
- Demonstrates using `FMassEntityView` to read Mass Entity data from Gameplay code
  - see `AXmsEntityRegistryListener_Wisp::NativeOnObservedEntitiesCreated`

## Entity Builders

We currently show two similar methods to procedurally generate Entities in Gameplay code.
By far, the simplest way to do so is using `UE::Mass::FEntityBuilder`.

In the [Actor Component](#actor-component-uxmsentitybuildercomponent) example,
the component ticks and automatically builds new entities on an interval.
Entities are always created at the current Player Pawn World Location.
This Component must be attached to the Player Pawn for this to work.

In the [Actor](#actor-axmsentitytreebuilder) example,
which also ticks and automatically builds on an interval,
Entities are randomly located inside the area of the Actor bounds in the Level.
This Actor must be placed in the Level for this to work.

### Actor Component: `UXmsEntityBuilderComponent`
Source Code:
[ [h](Source/Xms/EntityBuilders/XmsEntityBuilderComponent.h)
| [cpp](Source/Xms/EntityBuilders/XmsEntityBuilderComponent.cpp)
]

The [`AXmsCharacter`](#player-character-xmscharacter)
Player Pawn has one of these components on it.

When Active, this component causes `Wisp` Entities to be built
at a periodic interval, at the current owner Actor's location
(the Player Pawn location).

See `UXmsEntityBuilderComponent::SetupEntityBuilder`
for the exact configuration used to build a `Wisp`.

### Actor: `AXmsEntityTreeBuilder`
Source Code:
[ [h](Source/Xms/EntityBuilders/XmsEntityTreeBuilder.h)
| [cpp](Source/Xms/EntityBuilders/XmsEntityTreeBuilder.cpp)
]

An Actor of this class is present in the `L_Default` World Level.
*(See the UEditor Scene Outliner).*

When Active, this Actor causes `Tree` Entities to be built
at a periodic interval, at a random World Location within the
Actor bounds.
(The base Actor is `AVolume` to provide the UEditor bounds controls).

See `AXmsEntityTreeBuilder::SetupEntityBuilder`
for the exact configuration used to build a `Tree`.

## Entity Attributes

Given the basic foundation set forth in this example project, you should be able to add
additional optional Entity Attributes quite easily.

For now, I've only added a single attribute, the `Lifespan`.

### Lifespan Attribute

The Lifespan is an optional Attribute.

Any Entity that has a `FXmsF_Lifespan` will have its `CurrentAge` auto-incremented each Frame
by the simulation `DeltaTime`.
Unless Immortal, Entities will be Destroyed at the end of the tick when they reach their `MaxAge`.

#### Lifespan Flags: `EXmsEntityLifespanFlags`
Source Code: [ [h](Source/Xms/Attributes/Lifespan/XmsLifespan.h) ]

| Bit | Meaning            |
|-----|--------------------|
| 0   | Entity is Immortal |

Add more bits as needed for your Lifespan needs.

#### Lifespan Fragment: `FXmsF_Lifespan`
Source Code: [ [h](Source/Xms/Attributes/Lifespan/XmsLifespan.h) ]

The presence of this Fragment on an Entity will cause it to participate in the
Lifespan system.

Every Entity without this Fragment is effectively Immortal
as far as the Lifespan system is concerned.

#### Lifespan Enforcer

- `UXmsLifespanEnforcer`
  [ [h](Source/Xms/Attributes/Lifespan/XmsLifespanEnforcer.h)
  | [cpp](Source/Xms/Attributes/Lifespan/XmsLifespanEnforcer.cpp)
  ]
	- Adds simulation `DeltaTime` to Entity age each tick
		- Kills Entities that are past their `MaxAge`
	- Executes in `FrameEnd` so old Entities live until the end of the tick

## Entity Representation

UE5 Mass Default Representation is **disabled** in this project.
This means no Entities appear in the world at all unless we explicitly put them there.

For now, the Representation system exists purely as an example of how to retrieve the data
from Mass into your Game and/or Render thread.
It is expected that you will implement your own Entity Representation.

#### Beware the CPU Abuse

As a fun way to abuse the CPU and make it do graphics work, the current Representation system
uses the CPU to draw tiles onto a Render Target.

It does this up to every single Frame,
for every single Entity with a `FXmsT_Represent` tag.

As you can imagine, this destroys the frame rate when there are many Entities being visualized
in this way. Use with caution.

This is an intentional design choice for this example project, since it is **very easy**
to show how the data gets from point A to point B and you can see it happen every frame
for every Entity.

Before you ship a game with something like this, you'll want to optimize it.

### Representation Subsystem:`UXmsRepSubsystem`
Source Code:
[ [h](Source/Xms/Representation/XmsRepSubsystem.h)
| [cpp](Source/Xms/Representation/XmsRepSubsystem.cpp)
]

Using `TMassExternalSubsystemTraits`:

| Trait | Value | Notes |
|---|---|---|
| `GameThreadOnly` | `false` | YES read thread-safe |
| `ThreadSafeWrite` | `true` | YES write thread-safe |

#### Representation Subsystem Requirements

Notice that this subsystem imposes some requirements on its use.

1. ONLY ONE Processor may write to it: `UXmsRepresentationProcessor`
    - Parallel threads from this one processor are supported

##### Representation Subsystem Caveats

- We need to know the "World Bounds" for representation
    - We get this from the Actor with the Scene Outliner Label = `WorldPlane`
        - This methodology only works in Development

### Representation Processors

- `UXmsRepresentationProcessor`
  [ [h](Source/Xms/Representation/XmsRepresentationProcessor.h)
  | [cpp](Source/Xms/Representation/XmsRepresentationProcessor.cpp)
  ]
	- Executes in `PostPhysics`, copies data to be displayed next frame
	- Copy `FXmsEntityRepresentationData` for each Entity with `FXmsT_Represent` Tag
		- **Parallel execution** uses a thread-safe method for data egress to `UXmsRepSubsystem`

#### Representation Tag: `FXmsT_Represent`
Source Code: [ [h](Source/Xms/Representation/XmsRepSubsystem.h) ]

- Presence of this tag identifies an Entity that will be Represented in the World
    - The tag may be added or removed at runtime

#### Representation Data: `FXmsEntityRepresentationData`
Source Code: [ [h](Source/Xms/Representation/XmsRepSubsystem.h) ]

- This is the `struct` that gets copied from Mass for every Entity we want to Represent
- You can easily imagine granting access to more/less/different data

## Game Setup

### Game Mode: `XmsGameMode`
Source Code:
[ [h](Source/Xms/Game/XmsGameMode.h)
| [cpp](Source/Xms/Game/XmsGameMode.cpp)
]

- Use Player Pawn: `AXmsCharacter`
- Use Player Controller: `AXmsPlayerController`
- Allow for BP-based class assignments via `DefaultXms.ini`

### Player Character: `XmsCharacter`
Source Code:
[ [h](Source/Xms/Game/XmsCharacter.h)
| [cpp](Source/Xms/Game/XmsCharacter.cpp)
]

- Mostly-default UE5 top-down Character with a camera boom on a spring arm
	- Gives easy access to the camera and its boom
- Add [`UXmsEntityRegistryListener_Wisp`](#example-implementation-axmsentityregistrylistener_wisp) component
  to build Wisps at a regular interval

### Player Controller: `XmsPlayerController`
Source Code:
[ [h](Source/Xms/Game/XmsPlayerController.h)
| [cpp](Source/Xms/Game/XmsPlayerController.cpp)
]

- Mostly-default UE5 top-down Player Controller
	- Added camera zoom input using `AXmsCharacter` interface
- Allow for BP-based class assignments via `DefaultXms.ini`

# Miscellaneous Thoughts

I'm not going for lots of features here, I'm going for practical examples in minimalist C++
to hopefully make it as easy as possible to observe coding patterns and methodologies
when working in multi-threaded Mass C++.

Note that some of Epic's default processors and subsystems are also multi-threaded, and
some of them are not. The patterns you see them using will differ accordingly.
I have tried to generalize their approaches for this example.

Note also that I am not perfect *(I know it is shocking)* and so if I have made any mistakes
or if you know a better way to do something, please do share your expertise,
I will appreciate your input.

### Naming Conventions

| Pattern       | Data Type                        |
|---------------|----------------------------------|
| `FXmsCSF_Foo` | Mass Const Shared Fragment `Foo` |
| `FXmsF_Foo`   | Mass Fragment `Foo`              |
| `FXmsT_Foo`   | Mass Tag `Foo`                   |

Generally I'm not a fan of using `_` in names in UE.
In this case, I make an exception, since I find it challenging to know at a glance
what type of Const Shared Fragment `FXmsFooConstSharedFragment` really is.
Conversely, `FXmsCSF_Foo` is quite obviously a `Foo` Const Shared Fragment.

Thus, I prefix these special Mass type names with `CSF_`, `F_`, `T_` or others as needed.

### Note on Render Target usage

I know I am completely abusing Render Targets.  The Render Target usage here is brute force
and full of inefficiencies.  The RenderTarget abuse causes some hitches if/when there are
too many Entities being visualized, and currently the project is set up to visualize **all**
Entities, so...  `:-)`

The purpose of the Render Target is **not** to demonstrate best practices for drawing with the CPU,
instead it is simply to show the data pipeline from `Mass -> Game -> Render`, with the
expectation that you would replace the Representation system with your own custom implementation
doing whatever is appropriate for your game.

### Notice: `MassGameplay` Dependency

This repository doesn't use *anything* from `MassGameplay`,
but for some reason the `UMassSimulationSubsystem` is in that module,
and Mass is essentially non-functional without that subsystem,
as no processors will ever execute other than Observers.

If Mass Processors aren't working in your project even though you are sure they should be,
make sure you enable the `MassGameplay` plugin.

### Use `DebugGame` Build Configuration

This project is intended to be developed in the `DebugGame` build configuration
*(with additional debug code enabled)*.

It also supports the `Development` configuration.

This example project is not intended to package or ship.

### Uses XistGG C++ Dev Tools

This repository began its existance as a refactored
[XistGame-Template](https://github.com/XistGG/XistGame-Template)
and follows Xist's typical
[UE5 Git Repository Setup](https://github.com/XistGG/UE5-Git-Init).
Check out those repos too if you're interested.
