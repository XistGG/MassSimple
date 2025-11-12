# UE5 Mass Simple C++ Example

| [<img src="MassSimple.png">](https://github.com/XistGG/MassSimple) | `MassSimple` is a simple example of how to do basic things in UE5 Mass C++.<br/><br/>Target UE Version: **5.7** |
|---|---|

This simple example of a UE5 Mass C++ project:

- Does:
	- Demonstrate methods to:
		- Build Mass Entities in any Gameplay code on any thread
		- Safely receive data ingress from multithreaded Mass processors
	- Conceptualize a Game "Entity Registry"
		- Entities with the Registry Tag receive "On Created" and "On Destroyed" Gameplay Events
		- Entities can optionally be assigned metadata, like Meta Type *(rock, tree, wisp, ...etc...)*
	- Provide a functional, *non-performant* alternative Representation system
		- Entities tagged for Representation are drawn as tiles on a Render Target
			- they otherwise exist purely as Mass Entity data with no other World representation
	- Utilize some brute-force methodologies in the interest of code simplicity
- Does **NOT**:
	- use **Any** default UE5 Mass Fragments or Gameplay Processors
		- or the default UE5 Mass Representation system
	- use Blueprints, Data Assets or any other way to move interesting code/config out of c++/ini

Detailed performance profiling, analysis and optimization is beyond the scope of this example.

There are many ways you can improve the efficiency of this code, each of which will make it more
complex and more difficult to understand.
I expect that you will optimize your code to your own specifications.

## Entity Registry

The Entity Registry concept is implemented using a subsystem and some Mass Observer processors.

In this implementation, the Meta Data is a Const Shared Fragment.

Every Entity that wants to participate in the Registry **must** have a
`FXmsT_Registry` Tag.

### Entity Meta Type: `EXmsEntityMetaType`
Source Code: [ [h](Source/Xms/EntityRegistry/XmsEntityMetaData.h) ]

An example Meta Type. Here we have just Rock, Tree and Wisp.

Currently we don't use the Rock.

### Entity Meta Data: `FXmsCSF_MetaData`
Source Code: [ [h](Source/Xms/EntityRegistry/XmsEntityMetaData.h) ]

### Registry Tag: `FXmsT_Registry`
Source Code: [ [h](Source/Xms/EntityRegistry/XmsEntityRegistry.h) ]

### Registry Subsystem: `UXmsRegistrySubsystem`
Source Code:
[ [h](Source/Xms/EntityRegistry/XmsEntityRegistry.h)
| [cpp](Source/Xms/EntityRegistry/XmsEntityRegistry.cpp)
]

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

### Entity Registry Listeners

#### Abstract Base Class: `UXmsEntityRegistryListener`
Source Code:
[ [h](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener.h)
| [cpp](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener.cpp)
]

#### Example Implementation: `UXmsEntityRegistryListener_Wisp`
Source Code:
[ [h](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener_Wisp.h)
| [cpp](Source/Xms/Gameplay/RegistryListener/XmsEntityRegistryListener_Wisp.cpp)
]

## Entity Builders

### Actor Component: UXmsEntityBuilderComponent
Source Code:
[ [h](Source/Xms/EntityBuilders/XmsEntityBuilderComponent.h)
| [cpp](Source/Xms/EntityBuilders/XmsEntityBuilderComponent.cpp)
]

### Actor: AXmsEntityTreeBuilder
Source Code:
[ [h](Source/Xms/EntityBuilders/XmsEntityTreeBuilder.h)
| [cpp](Source/Xms/EntityBuilders/XmsEntityTreeBuilder.cpp)
]

## Entity Attributes

### Lifespan Attribute

#### Lifespan Flags: `EXmsEntityLifespanFlags`

#### Lifespan Fragment: `FXmsF_Lifespan`

#### Lifespan Processors
Source Code:
[ [h](Source/Xms/Attributes/Lifespan/XmsLifespanProcessors.h)
| [cpp](Source/Xms/Attributes/Lifespan/XmsLifespanProcessors.cpp)
]

- `UXmsLifespanEnforcer`
	- Adds simulation `DeltaTime` to Entity age each tick
		- Kills Entities that are past their `MaxAge`
	- Executes in `FrameEnd` so old Entities live until the end of the tick

## Entity Representation

### Subsystem:`UXmsRepSubsystem`
Source Code:
[ [h](Source/Xms/Representation/XmsRepSubsystem.h)
| [cpp](Source/Xms/Representation/XmsRepSubsystem.cpp)
]

#### Representation Tag: `FXmsT_Represent`

#### Representation Data: `FXmsEntityRepresentationData`

### Representation Processors

- `UXmsRepresentationProcessor`
  [ [h](Source/Xms/Representation/XmsRepresentationProcessor.h)
  | [cpp](Source/Xms/Representation/XmsRepresentationProcessor.cpp)
  ]
	- Executes in `FrameEnd`, copies data to be displayed next frame
	- Copy `FXmsEntityRepresentationData` for each Entity with `FXmsT_Represent` Tag
		- **Parallel execution** uses a thread-safe method for data egress to `UXmsRepSubsystem`

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

### Player Controller: `XmsPlayerController`
Source Code:
[ [h](Source/Xms/Game/XmsPlayerController.h)
| [cpp](Source/Xms/Game/XmsPlayerController.cpp)
]

- Mostly-default UE5 top-down Player Controller
	- Added camera zoom input using `AXmsCharacter` interface
- Allow for BP-based class assignments via `DefaultXms.ini`

--------------------------------------------------------------------------------

I'm not going for lots of features here, I'm going for practical examples in minimalist C++
to hopefully make it as easy as possible to observe coding patterns and methodologies
when working in multi-threaded Mass C++.

Note that some of Epic's default processors and subsystems are also multi-threaded, and
some of them are not. The patterns you see them using will differ accordingly.
I have tried to generalize their approaches for this example.

Note also that I am not perfect *(I know it is shocking)* and so if I have made any mistakes
or if you know a better way to do something, please do share your expertise,
I will appreciate your input.

## Note on Render Target usage

I know I am completely abusing Render Targets.  The Render Target usage here is brute force
and full of inefficiencies.  The RenderTarget abuse causes some hitches if/when there are
too many Entities being visualized, and currently the project is set up to visualize **all**
Entities, so...  `:-)`

The purpose of the Render Target isn't to demonstrate best practices for drawing with the CPU,
instead it is simply to show the data pipeline from `Mass -> Game -> Render`, with the
expectation that you would replace the Representation system with your own custom implementation
doing whatever is appropriate for your game.

## Use `DebugGame` Build Configuration

This project is intended to be developed in either the `DebugGame` build configuration
*(with additional debug code enabled)* or in `Development`.

### Notice: `MassGameplay` Dependency

This repository doesn't use *anything* from `MassGameplay`,
but for some reason the `UMassSimulationSubsystem` is in that module,
and Mass is essentially non-functional without that subsystem,
as no processors will ever execute other than Observers.

If Mass Processors aren't working in your project even though you are sure they should be,
make sure you enable the `MassGameplay` plugin.

### Uses XistGG Tools

This repository began its instantiation as a refactored
[XistGame-Template](https://github.com/XistGG/XistGame-Template)
and follows Xist's typical
[UE5 Git Repository Setup](https://github.com/XistGG/UE5-Git-Init).
Check out those repos too if you're interested.
