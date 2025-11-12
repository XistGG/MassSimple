# Mass Simple C++ Example

| [<img src="MassSimple.png">](https://github.com/XistGG/MassSimple) | Read the Docs: https://x157.github.io/UE5/Mass/MassSimple<br/><br/>`MassSimple` is a simple example of how to do basic things in UE5 Mass C++.<br/><br/>Target UE Version: **5.7** |
|---|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

MAKE SURE YOU READ THE [OFFICIAL DOCUMENTATION](https://x157.github.io/UE5/Mass/MassSimple):
https://x157.github.io/UE5/Mass/MassSimple

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
