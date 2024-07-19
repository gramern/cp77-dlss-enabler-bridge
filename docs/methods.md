# Methods

All methods are global and can be called at any moment of the game after the mod is initilized by RED4ext. They require DLSS Enabler to be installed and Frame Generation enabled in the game.

## DLSSEnablerGetFrameGenerationState()

### Description:
Retrieves the current state of Frame Generation in Cyberpunk 2077 with DLSS Enabler installed.

### Parameters:
None

### Returns:
Bool - True if DLSS Frame Generation is currently enabled, False if it's disabled or if the function fails to retrieve the state.

### Exemplary Usage (CET-lua):
```
local is Enabled = DLSSEnablerGetFrameGenerationState()

if isEnabled then
    print("Frame Generation is currently enabled")
else
    print("Frame Generation is currently disabled or the state couldn't be retrieved")
end
```

## DLSSEnablerSetFrameGeneration(bool shouldEnable)

### Description:
Sets the Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed to either enabled or disabled.

### Parameters:
shouldEnable (Bool) - True to enable Frame Generation, False to disable it.

### Returns:
Bool - True if the operation was successful, False otherwise.

### Exemplary Usage (CET-lua):
```
-- To enable Frame Generation
local result = DLSSEnablerSetFrameGeneration(true)

if result then
    print("Frame Generation enabled successfully")
else
    print("Failed to enable Frame Generation")
end
```

```
-- To disable Frame Generation
local result = DLSSEnablerSetFrameGeneration(false)

if result then
    print("Frame Generation disabled successfully")
else
    print("Failed to disable Frame Generation")
end
```

## DLSSEnablerToggleFrameGeneration()

### Description:
Toggles the Frame Generation state in Cyberpunk 2077 with DLSS Enabler installed. If Frame Generation is currently enabled, this function will disable it, and vice versa.

### Parameters:
None

### Returns:
Bool - True if the operation was successful, False otherwise.

### Exemplary Usage (CET-lua):
```
local result = DLSSEnablerToggleFrameGeneration()

if result then
    print("Frame Generation state toggled successfully")
else
    print("Failed to toggle Frame Generation state")
end
```

## Logging
Errors codes below are the same for all methods and follow DLSS Enabler's API and `DLSS_Enabler_Result`:
```
﻿typedef enum DLSS_Enabler_Result
{
    DLSS_Enabler_Result_Success = 1, // get/set operation succeeded
    DLSS_Enabler_Result_Fail_Unsupported = 0, // user did not enable Frame Generation feature in game settings first
    DLSS_Enabler_Result_Fail_Bad_Argument = -1,
} DLSS_Enabler_Result;
```
