# DLSS Enabler Bridge 2077

A RED4ext plugin for Cyberpunk 2077 to control Frame Generation state via DLSS Enabler's API.

**NOTE:** _All methods follow and pass DLSS Enabler's API logic without modifying it._

## Features
Adds global methods that can be called from other mods/CET's console:
- `DLSSEnabler_GetVersionAsString()`: get `dlss-enabler.dll` version as a string
- `DLSSEnabler_GetFrameGenerationMode()`: get Frame Generation mode (0,1,3)
- `DLSSEnabler_SetFrameGenerationMode(int32 newMode)`: set Frame Generation to a specific mode (0,1,2,3)
- `DLSSEnabler_GetFrameGenerationState()`: get Frame Generation state (on/off)
- `DLSSEnabler_SetFrameGenerationState(bool shouldEnable)`: set Frame Generation to a specific state (on/off)
- `DLSSEnabler_ToggleFrameGenerationState()`: toggle Frame Generation (on/off)
- `DLSSEnabler_GetDynamicFrameGenerationState()`: get Dynamic Frame Generation state (on/off)
- `DLSSEnabler_SetDynamicFrameGenerationState(bool shouldEnable)`: set Dynamic Frame Generation to a specific state (on/off)

See [methods.md](docs/methods.md) for details.

You can use launch parameters for the game: `--de-bridge-debug` for the plugin's debug logging or `--de-bridge-debug-ext` for extended debug logging.

## Requirements
+ Cyberpunk 2.12+
+ [DLSS Enabler](https://github.com/artur-graniszewski/DLSS-Enabler) 3.01.000.0+
+ [RED4ext](https://github.com/WopsS/RED4ext) 1.25.0+

## Installation
Place the `dlss-enabler-bridge-2077.dll` file in the `..\your Cyberpunk 2077 folder\red4ext\plugins\DLSSEnablerBridge2077` folder or unzip the archive to the game folder.

## Building
1. Clone the repository: (`git clone https://github.com/gramern/cp77-dlss-enabler-bridge.git`).
2. Navigate to the project directory (`cd cp77-dlss-enabler-bridge`)
2. Clone dependencies  (`git submodule update --init --recursive`).
3. Build [RED4ext.SDK](https://github.com/WopsS/RED4ext.SDK) projects.
4. Build this project.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
