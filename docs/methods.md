# Methods

All methods are global and can be called at any moment of the game after the plugin is initialized by RED4ext. They require DLSS Enabler to be installed and Frame Generation enabled in the game.

## DLSSEnabler_GetFrameGenerationState()

### Description:
Retrieves the current state of Frame Generation in Cyberpunk 2077 with DLSS Enabler installed.

### Parameters:
None

### Returns:
Bool - True if DLSS Frame Generation is currently enabled, False if it's disabled or if the function fails to retrieve the state.

### Exemplary Usage (CET-lua):
```
local isEnabled = DLSSEnabler_GetFrameGenerationState()

if isEnabled then
    print("Frame Generation is currently enabled")
else
    print("Frame Generation is currently disabled or the state couldn't be retrieved")
end
```

## DLSSEnabler_SetFrameGeneration(bool shouldEnable)

### Description:
Sets the Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed to either enabled or disabled.

### Parameters:
shouldEnable (Bool) - True to enable Frame Generation, False to disable it.

### Returns:
Bool - True if the operation was successful, False otherwise.

### Exemplary Usage (CET-lua):
```
-- To enable Frame Generation
local result = DLSSEnabler_SetFrameGeneration(true)

if result then
    print("Frame Generation enabled successfully")
else
    print("Failed to enable Frame Generation")
end
```

```
-- To disable Frame Generation
local result = DLSSEnabler_SetFrameGeneration(false)

if result then
    print("Frame Generation disabled successfully")
else
    print("Failed to disable Frame Generation")
end
```

## DLSSEnabler_SetDynamicFrameGeneration(bool shouldEnable)

### Description:
Sets the Dynamic Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed to either enabled or disabled.

### Parameters:
shouldEnable (Bool) - True to enable Dynamic Frame Generation, False to disable it.

### Returns:
Bool - True if the operation was successful, False otherwise.

### Exemplary Usage (CET-lua):
```
-- To enable Dynamic Frame Generation
local result = DLSSEnabler_SetDynamicFrameGeneration(true)

if result then
    print("Dynamic Frame Generation enabled successfully")
else
    print("Failed to enable Dynamic Frame Generation")
end
```

```
-- To disable Dynamic Frame Generation
local result = DLSSEnabler_SetDynamicFrameGeneration(false)

if result then
    print("Dynamic Frame Generation disabled successfully")
else
    print("Failed to disable Dynamic Frame Generation")
end
```

## DLSSEnabler_ToggleFrameGeneration()

### Description:
Toggles the Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed. If Frame Generation is currently enabled, this function will disable it, and vice versa.

### Parameters:
None

### Returns:
Bool - True if the operation was successful, False otherwise.

### Exemplary Usage (CET-lua):
```
local result = DLSSEnabler_ToggleFrameGeneration()

if result then
    print("Frame Generation state toggled successfully")
else
    print("Failed to toggle Frame Generation state")
end
```

## Logging
Results codes are the same for all methods and follow DLSS Enabler's API and `DLSS_Enabler_Result` below:
```
﻿typedef enum DLSS_Enabler_Result
{
    DLSS_Enabler_Result_Success = 1, // get/set operation succeeded
    DLSS_Enabler_Result_Fail_Unsupported = 0, // user did not enable Frame Generation feature in game settings first
    DLSS_Enabler_Result_Fail_Bad_Argument = -1,
} DLSS_Enabler_Result;
```
