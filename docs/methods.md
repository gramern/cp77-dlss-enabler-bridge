# Methods

All methods are global and can be called at any moment of the game after the plugin is initialized by RED4ext. They require DLSS Enabler to be installed and Frame Generation enabled in the game.

**NOTE:** _All methods retrieve or set Frame Generation Mode/State as it is in the current running instance of DLSS Enabler. The game's Frame Generation State remains the same as set in the game's settings._

**NOTE:** _All methods follow and pass DLSS Enabler's API logic without modifying it._

# DLSS Enabler DLL

## `DLSSEnabler_GetVersionAsString()`

### Description:
Retrieves the version of the loaded `dlss-enabler.dll` file.

### Required DLSS Enabler Version:
All

### Parameters:
None

### Returns:
`string` - The version of DLSS Enabler's DLL (e.g., "2.90.0.0"). If the file is found but a version cannot be obtained, returns "Unknown".

### Exemplary Usage (CET-lua):
```
local version = DLSSEnabler_GetVersionAsString()

print("DLSS Enabler version: " .. version)
```

# Frame Generation Mode

Modes are the same for all methods and follow DLSS Enabler's API and `DLSS_Enabler_FrameGeneration_Mode` below:
```
typedef enum DLSS_Enabler_FrameGeneration_Mode
{
    DLSS_Enabler_FrameGeneration_Disabled = 0,
    DLSS_Enabler_FrameGeneration_Enabled = 1,
    DLSS_Enabler_FrameGeneration_DFG_Disabled = 2,
    DLSS_Enabler_FrameGeneration_DFG_Enabled = 3,
} DLSS_Enabler_FrameGeneration_Mode;
```

## `DLSSEnabler_GetFrameGenerationMode()`

### Description:
Retrieves the current Frame Generation mode in Cyberpunk 2077 with DLSS Enabler installed. 

### Required DLSS Enabler Version:
3.01.000.0-b12+

### Parameters:
None

### Returns:
`int32` - The current Frame Generation mode:

`0`: Frame Generation Disabled; Dynamic Frame Generation Disabled  
`1`: Frame Generation Enabled; Dynamic Frame Generation Disabled  
`3`: Frame Generation Unknown; Dynamic Frame Generation Enabled. 

**NOTE:** _Always returns `3`, if Dynamic Frame Generation is enabled in the API **and** DLSS Enabler's/OptiScaler's `*.ini` or UI._

Returns `-1` if the operation fails.

### Exemplary Usage (CET-lua):
```
local mode = DLSSEnabler_GetFrameGenerationMode()

if mode >= 0 then
    print("Frame Generation Mode set to: " .. mode)
else
    print("Failed to retrieve Frame Generation Mode")
end
```


## `DLSSEnabler_SetFrameGenerationMode(int32 newMode)`

### Description:
Sets the Frame Generation mode in Cyberpunk 2077 with DLSS Enabler installed.  

**NOTE:** _Doesn't change Frame Generation State in the game's settings._

### Required DLSS Enabler Version:
3.01.000.0-b12+

### Parameters:
`newMode` (`int32`) - The desired Frame Generation mode:

`0`: Disable Frame Generation and Dynamic Frame Generation  
`1`: Enable Frame Generation (Dynamic Frame Generation state unchanged)  
`2`: Disable Dynamic Frame Generation (Frame Generation state unchanged)  
`3`: Enable Dynamic Frame Generation (Frame Generation state unchanged)  

### Returns:
`bool` - `true` if the operation was successful, `false` otherwise.

### Exemplary Usage (CET-lua):
To enable Frame Generation
```
local result = DLSSEnabler_SetFrameGenerationMode(1)

if result then
    print("Frame Generation Mode set successfully:", result)
else
    print("Failed to set Frame Generation mode:", result)
end
```

To enable Dynamic Frame Generation
```
local result = DLSSEnabler_SetFrameGenerationMode(3)

if result then
    print("Dynamic Frame Generation Mode set successfully:", result)
else
    print("Failed to set Dynamic Frame Generation mode:", result)
end
```

# Frame Generation State

## `DLSSEnabler_GetFrameGenerationState()`

### Description:
Retrieves the current state of Frame Generation in Cyberpunk 2077 with DLSS Enabler installed.

### Required DLSS Enabler Version:
2.90.800.0+

### Parameters:
None

### Returns:
`bool` - `true` if Frame Generation is currently enabled, `false` if it's disabled or if the function fails to retrieve the state.

**NOTE:** _Returns `false` if Dynamic Frame Generation is enabled in the API **and** DLSS Enabler's/OptiScaler's `*.ini` or UI._

### Exemplary Usage (CET-lua):
```
local isEnabled = DLSSEnabler_GetFrameGenerationState()

if isEnabled then
    print("Frame Generation is currently enabled")
else
    print("Frame Generation is currently disabled or the state couldn't be retrieved")
end
```

## `DLSSEnabler_SetFrameGenerationState(bool shouldEnable)`

### Description:
Sets the Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed to either enabled or disabled.  

**NOTE:** _Doesn't change Frame Generation State in the game's settings._

### Required DLSS Enabler Version:
2.90.800.0+

### Parameters:
`shouldEnable` (`bool`) - `true` to enable Frame Generation, `false` to disable it.

### Returns:
`bool` - `true` if the operation was successful, `false` otherwise.

### Exemplary Usage (CET-lua):
```
-- To enable Frame Generation
local result = DLSSEnabler_SetFrameGenerationState(true)

if result then
    print("Frame Generation enabled successfully")
else
    print("Failed to enable Frame Generation")
end
```

```
-- To disable Frame Generation
local result = DLSSEnabler_SetFrameGenerationState(false)

if result then
    print("Frame Generation disabled successfully")
else
    print("Failed to disable Frame Generation")
end
```

## `DLSSEnabler_ToggleFrameGenerationState()`

### Description:
Toggles the Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed. If Frame Generation is currently enabled, this function will disable it, and vice versa.

### Required DLSS Enabler Version:
2.90.800.0+

### Parameters:
None

### Returns:
`bool` - `true` if the operation was successful, `false` otherwise.

### Exemplary Usage (CET-lua):
```
local result = DLSSEnabler_ToggleFrameGenerationState()

if result then
    print("Frame Generation state toggled successfully")
else
    print("Failed to toggle Frame Generation state")
end
```

# Dynamic Frame Generation State

## `DLSSEnabler_GetDynamicFrameGenerationState()`

### Description:
Retrieves the current state of Dynamic Frame Generation in Cyberpunk 2077 with DLSS Enabler installed.

**NOTE:** _Dynamic Frame Generation needs to be enabled in DLSS Enabler's/OptiScaler's settings to return `true`._

### Required DLSS Enabler Version:
3.01.000.0-b12+

### Parameters:
None

### Returns:
`bool` - `true` if Dynamic Frame Generation is currently enabled, `false` if it's disabled or if the function fails to retrieve the state.

### Exemplary Usage (CET-lua):
```
local isEnabled = DLSSEnabler_GetDynamicFrameGenerationState()

if isEnabled then
    print("Dynamic Frame Generation is currently enabled")
else
    print("Dynamic Frame Generation is currently disabled or the state couldn't be retrieved")
end
```

## `DLSSEnabler_SetDynamicFrameGenerationState(bool shouldEnable)`

### Description:
Sets the Dynamic Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed to either enabled or disabled.

**NOTE:** _Dynamic Frame Generation needs to be enabled in DLSS Enabler's/OptiScaler's settings. Doesn't modify values shown in `*.ini` or UI of those._

### Required DLSS Enabler Version:
3.01.000.0-b10+

### Parameters:
`shouldEnable` (`bool`) - `true` to enable Dynamic Frame Generation, `false` to disable it.

### Returns:
`bool` - `true` if the operation was successful, `false` otherwise.

### Exemplary Usage (CET-lua):
```
-- To enable Dynamic Frame Generation
local result = DLSSEnabler_SetDynamicFrameGenerationState(true)

if result then
    print("Dynamic Frame Generation enabled successfully")
else
    print("Failed to enable Dynamic Frame Generation")
end
```

```
-- To disable Dynamic Frame Generation
local result = DLSSEnabler_SetDynamicFrameGenerationState(false)

if result then
    print("Dynamic Frame Generation disabled successfully")
else
    print("Failed to disable Dynamic Frame Generation")
end
```

# Logging
The plugin saves logs to the standard localization: `..\your Cybrepunk 2077 folder\red4ext\logs`.

Results codes are the same for all methods and follow DLSS Enabler's API and `DLSS_Enabler_Result` below:
```
﻿typedef enum DLSS_Enabler_Result
{
    DLSS_Enabler_Result_Success = 1, // get/set operation succeeded
    DLSS_Enabler_Result_Fail_Unsupported = 0, // user did not enable Frame Generation feature in game settings first OR the game disabled it temporarily (i.e. main and pause menu)
    DLSS_Enabler_Result_Fail_Bad_Argument = -1,
} DLSS_Enabler_Result;
```

For debug logging, use launch parameter `--de-bridge-debug` for the game (requires version 0.3.2.0+ of the plugin.).
For extended debug logging, use `--de-bridge-debug-ext` for the game (requires version 0.3.4.0+ of the plugin.).
