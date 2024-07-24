# DLSS Enabler Bridge 2077

A RED4ext plugin for Cyberpunk 2077 to control Frame Generation state via DLSS Enabler's API.

## Features
Adds global methods that can be called from other mods/CET's console:
- `DLSSEnabler_GetFrameGenerationMode()`: get Frame Generation mode (0,1,3)
- `DLSSEnabler_SetFrameGenerationMode(int32 newMode)`: set Frame Generation to a specific mode (0,1,2,3)
- `DLSSEnabler_GetFrameGenerationState()`: get Frame Generation state (on/off)
- `DLSSEnabler_SetFrameGenerationState(bool shouldEnable)`: set Frame Generation to a specific state (on/off)
- `DLSSEnabler_ToggleFrameGenerationState()`: toggle Frame Generation (on/off)
- `DLSSEnabler_GetDynamicFrameGenerationState()`: get Dynamic Frame Generation state (on/off)
- `DLSSEnabler_SetDynamicFrameGenerationState(bool shouldEnable)`: set Dynamic Frame Generation to a specific state (on/off)

See [methods.md](docs/methods.md) for details.

## Requirements
+ Cyberpunk 2.12+
+ [DLSS Enabler](https://github.com/artur-graniszewski/DLSS-Enabler) 2.90.800.0+
+ [RED4ext](https://github.com/WopsS/RED4ext) 1.25.0+

## Installation
Place the `dlss-enabler-bridge-2077.dll` file in the `..\your Cybrepunk 2077 folder\red4ext\plugins\DLSSEnablerBridge2077` folder or unzip the archive to the game folder.

## Building
Place the repo next to your local [RED4ext.SDK](https://github.com/WopsS/RED4ext.SDK) repository location and add the project to SDK's `*.sln` file. Make sure to check "RED4ext.SDK" in the project's dependencies.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
