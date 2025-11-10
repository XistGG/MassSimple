# Mass Simple C++ Example

Target UE Version: **5.7**

`MassSimple` is intended to be a simple example of how to do some basic things in UE5 Mass C++.

I'm not going for lots of features here, I'm going for practical examples in minimalist C++
to hopefully make it as easy as possible to observe coding patterns and methodologies
when working in multi-threaded Mass C++.

Note that some of Epic's default processors and subsystems are also multi-threaded, and
some of them are not. The patterns you see them using will differ accordingly.
I have tried to generalize their approaches for this multi-threaded example.

Note also that I am not perfect *(I know it is shocking)* and so if I have made any mistakes
or if you know a better way to do something, please do share your expertise,
I will appreciate your input.

## Use `DebugGame` Build Configuration

This project is intended to be developed in the `DebugGame` build configuration.

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
