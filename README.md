# DLSS Enabler Control Interface 2077

A mod for Cyberpunk 2077 to control Frame Generation via DLSS Enabler API.

## Features
Adds global methods that can be called from other mods/CET's console:
- `DLSSEnablerGetFrameGeneration()`: get Frame Generation state (on/ff)
- `DLSSEnablerSetFrameGeneration(bool)`: set Frame Generation to a specific state (on/ff)
- `DLSSEnablerToggleFrameGeneration()`: toggle Frame Generation on/off

See [methods.md](docs/methods.md) for details.

## Requirements
+ Cyberpunk 2.12+
+ [DLSS Enabler](https://github.com/artur-graniszewski/DLSS-Enabler) 2.90.800.0+
+ [RED4ext](https://github.com/WopsS/RED4ext) 1.25.0+

## Installation
Place the `DLSSEnablerControlInterface.dll` file in the `..\Cyberpunk 2077\red4ext\plugins\DLSSEnablerControlInterface` folder or unzip the archive to the game folder.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
