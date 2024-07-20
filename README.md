# DLSS Enabler Control Interface 2077

A mod for Cyberpunk 2077 to control Frame Generation state via DLSS Enabler's API.

## Features
Adds global methods that can be called from other mods/CET's console:
- `DLSSEnabler_GetFrameGeneration()`: get Frame Generation state (on/off)
- `DLSSEnabler_SetFrameGeneration(bool)`: set Frame Generation to a specific state (on/off)
- `DLSSEnabler_ToggleFrameGeneration()`: toggle Frame Generation on/off

See [methods.md](docs/methods.md) for details.

## Requirements
+ Cyberpunk 2.12+
+ [DLSS Enabler](https://github.com/artur-graniszewski/DLSS-Enabler) 2.90.800.0+
+ [RED4ext](https://github.com/WopsS/RED4ext) 1.25.0+

## Installation
Place the `dlss-enabler-control-interface-2077.dll` file in the `..\Cyberpunk 2077\red4ext\plugins\DLSSEnablerControlInterface2077` folder or unzip the archive to the game folder.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
