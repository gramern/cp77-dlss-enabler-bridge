#include <windows.h>
#include <RED4ext/RED4ext.hpp>

const RED4ext::Sdk* sdk;
RED4ext::PluginHandle handle;

static bool g_deBridgeDebug = false;

typedef enum DLSS_Enabler_FrameGeneration_Mode
{
    DLSS_Enabler_FrameGeneration_Disabled = 0,
    DLSS_Enabler_FrameGeneration_Enabled = 1,
    DLSS_Enabler_FrameGeneration_DFG_Disabled = 2,
    DLSS_Enabler_FrameGeneration_DFG_Enabled = 3,
} DLSS_Enabler_FrameGeneration_Mode;

typedef enum DLSS_Enabler_Result
{
    DLSS_Enabler_Result_Success = 1,
    DLSS_Enabler_Result_Fail_Unsupported = 0,
    DLSS_Enabler_Result_Fail_Bad_Argument = -1,
} DLSS_Enabler_Result;

typedef DLSS_Enabler_Result(*GetFrameGenerationModeFunc)(DLSS_Enabler_FrameGeneration_Mode& mode);
typedef DLSS_Enabler_Result(*SetFrameGenerationModeFunc)(DLSS_Enabler_FrameGeneration_Mode mode);

static HMODULE hDll;
static GetFrameGenerationModeFunc g_GetFrameGenerationMode = nullptr;
static SetFrameGenerationModeFunc g_SetFrameGenerationMode = nullptr;

bool DLSSEnabler_OnInitialize()
{
    hDll = LoadLibraryW(L"dlss-enabler.dll");
    if (!hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_OnInitialize] Failed to load dlss-enabler.dll. Error code: %lu", error);
        return false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_OnInitialize] dlss-enabler.dll loaded successfully");
    }

    g_GetFrameGenerationMode = (GetFrameGenerationModeFunc)GetProcAddress(hDll, "GetFrameGenerationMode");
    g_SetFrameGenerationMode = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!g_GetFrameGenerationMode || !g_SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_OnInitialize] Failed to get function addresses. Error code: %lu", error);
        FreeLibrary(hDll);
        hDll = nullptr;
        return false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_OnInitialize] Plugin has loaded succesfully");
    }

    return true;
}

void DLSSEnabler_OnUninitialize()
{
    if (hDll)
    {
        g_GetFrameGenerationMode = nullptr;
        g_SetFrameGenerationMode = nullptr;
        FreeLibrary(hDll);
        hDll = nullptr;
    }
}

void DLSSEnabler_GetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationMode] Called!");
    }

    if (!g_GetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_GetFrameGenerationMode] Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = DLSS_Enabler_Result_Fail_Unsupported;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (g_deBridgeDebug)
        {
            const char* modeString;
            switch (currentMode)
            {
            case DLSS_Enabler_FrameGeneration_Disabled:
                modeString = "FG Disabled; DFG Disabled";
                break;
            case DLSS_Enabler_FrameGeneration_Enabled:
                modeString = "FG Enabled; DFG Disabled";
                break;
            case DLSS_Enabler_FrameGeneration_DFG_Disabled:
                modeString = "FG Unknown; DFG Disabled";
                break;
            case DLSS_Enabler_FrameGeneration_DFG_Enabled:
                modeString = "FG Unknown; DFG Enabled";
                break;
            default:
                modeString = "Unknown";
            }

            sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationMode] Current Frame Generation Mode: %s", modeString);
        }

        if (aOut) *aOut = currentMode;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_GetFrameGenerationMode] Failed to get Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = -1;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationMode] Completed");
    }
}

void DLSSEnabler_GetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationState] Called!");
    }

    if (!g_GetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_GetFrameGenerationState] Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (g_deBridgeDebug)
        {
            const char* modeString;
            switch (currentMode)
            {
            case DLSS_Enabler_FrameGeneration_Enabled:
                modeString = "Enabled";
                break;
            case DLSS_Enabler_FrameGeneration_Disabled:
                modeString = "Disabled";
                break;
            default:
                modeString = "Unknown";
            }

            sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationState] Current Frame Generation State: %s", modeString);
        }

        if (aOut) *aOut = (currentMode == DLSS_Enabler_FrameGeneration_Enabled);
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_GetFrameGenerationState] Failed to get Frame Generation State. Result: %d", result);
        if (aOut) *aOut = false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationState] Completed");
    }
}

void DLSSEnabler_GetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetDynamicFrameGenerationState] Called!");
    }

    if (!g_GetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_GetDynamicFrameGenerationState] Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (g_deBridgeDebug)
        {
            const char* modeString;
            switch (currentMode)
            {
            case DLSS_Enabler_FrameGeneration_DFG_Enabled:
                modeString = "Enabled";
                break;
            case DLSS_Enabler_FrameGeneration_DFG_Disabled:
                modeString = "Disabled";
                break;
            case DLSS_Enabler_FrameGeneration_Enabled:
                modeString = "Disabled";
                break;
            case DLSS_Enabler_FrameGeneration_Disabled:
                modeString = "Disabled";
                break;
            default:
                modeString = "Unknown";
            }

            sdk->logger->InfoF(handle, "[DLSSEnabler_GetDynamicFrameGenerationState] Current Dynamic Frame Generation State: %s", modeString);
        }

        if (aOut) *aOut = (currentMode == DLSS_Enabler_FrameGeneration_DFG_Enabled);
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_GetDynamicFrameGenerationState] Failed to get Dynamic Frame Generation State. Result: %d", result);
        if (aOut) *aOut = false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetDynamicFrameGenerationState] Completed");
    }
}

void DLSSEnabler_SetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    int32_t newMode;
    RED4ext::GetParameter(aFrame, &newMode);
    aFrame->code++; // skip ParamEnd

    if (newMode < 0 || newMode > 3) {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetFrameGenerationMode] Invalid mode value: %d", newMode);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode modeValue = static_cast<DLSS_Enabler_FrameGeneration_Mode>(newMode);

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGenerationMode] Called with mode = %d", newMode);
    }

    if (!g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetFrameGenerationMode] Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGenerationMode] SetFrameGenerationMode function address obtained successfully");
    }

    DLSS_Enabler_Result result = g_SetFrameGenerationMode(modeValue);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (g_deBridgeDebug)
        {
            sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGenerationMode] Frame Generation mode set successfully");
        }
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetFrameGenerationMode] Failed to set Frame Generation mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGenerationMode] Completed");
    }
}

void DLSSEnabler_SetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGeneration] Called with shouldEnable = %s", shouldEnable ? "true" : "false");
    }

    if (!g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetFrameGeneration] Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGeneration] SetFrameGenerationMode function address obtained successfully");
    }

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_Enabled : DLSS_Enabler_FrameGeneration_Disabled;
    DLSS_Enabler_Result result = g_SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (g_deBridgeDebug)
        {
            sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGeneration] Frame Generation set to %s", shouldEnable ? "Enabled" : "Disabled");
        }
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetFrameGeneration] Failed to set Frame Generation. Result: %d", result);
        if (aOut) *aOut = false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGeneration] Completed");
    }
}

void DLSSEnabler_SetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetDynamicFrameGeneration] Called with shouldEnable = %s", shouldEnable ? "true" : "false");
    }

    if (!g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetDynamicFrameGeneration] Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetDynamicFrameGeneration] SetFrameGenerationMode function address obtained successfully");
    }

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_DFG_Enabled : DLSS_Enabler_FrameGeneration_DFG_Disabled;
    DLSS_Enabler_Result result = g_SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (g_deBridgeDebug)
        {
            sdk->logger->InfoF(handle, "[DLSSEnabler_SetDynamicFrameGeneration] Dynamic Frame Generation set to %s", shouldEnable ? "Enabled" : "Disabled");
        }
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_SetDynamicFrameGeneration] Failed to set Dynamic Frame Generation. Result: %d", result);
        if (aOut) *aOut = false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetDynamicFrameGeneration] Completed");
    }
}

void DLSSEnabler_ToggleFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGeneration] Called!");
    }

    if (!g_GetFrameGenerationMode || !g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler_ToggleFrameGeneration] Failed to get function addresses. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGeneration] Function addresses obtained successfully");
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result != DLSS_Enabler_Result_Success)
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_ToggleFrameGeneration] GetFrameGenerationMode failed with result: %d", result);
        if (aOut) *aOut = false;
        return;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGeneration] Current Frame Generation Mode: %d", currentMode);
    }

    if (currentMode == DLSS_Enabler_FrameGeneration_Disabled)
    {
        result = g_SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Enabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            if (g_deBridgeDebug)
            {
                sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGeneration] Frame Generation set to Enabled");
            }
            if (aOut) *aOut = true;
        }
        else
        {
            sdk->logger->ErrorF(handle, "[DLSSEnabler_ToggleFrameGeneration] Failed to enable Frame Generation. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else if (currentMode == DLSS_Enabler_FrameGeneration_Enabled)
    {
        result = g_SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Disabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            if (g_deBridgeDebug)
            {
                sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGeneration] Frame Generation set to Disabled");
            }
            if (aOut) *aOut = true;
        }
        else
        {
            sdk->logger->ErrorF(handle, "[DLSSEnabler_ToggleFrameGeneration] Failed to disable Frame Generation. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnabler_ToggleFrameGeneration] Unexpected Frame Generation Mode: %d", currentMode);
        if (aOut) *aOut = false;
    }

    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGeneration] Completed");
    }
}

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
{
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
{
    auto rtti = RED4ext::CRTTISystem::Get();

    auto getModeFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetFrameGenerationMode", "DLSSEnabler_GetFrameGenerationMode", &DLSSEnabler_GetFrameGenerationMode);
    getModeFunc->SetReturnType("Int32");
    rtti->RegisterFunction(getModeFunc);
    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationMode] Registered!");
    }

    auto getStateFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetFrameGenerationState", "DLSSEnabler_GetFrameGenerationState", &DLSSEnabler_GetFrameGenerationState);
    getStateFunc->SetReturnType("Bool");
    rtti->RegisterFunction(getStateFunc);
    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetFrameGenerationState] Registered!");
    }

    auto getDFGStateFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetDynamicFrameGenerationState", "DLSSEnabler_GetDynamicFrameGenerationState", &DLSSEnabler_GetDynamicFrameGenerationState);
    getDFGStateFunc->SetReturnType("Bool");
    rtti->RegisterFunction(getDFGStateFunc);
    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_GetDynamicFrameGenerationState] Registered!");
    }

    auto setModeFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_SetFrameGenerationMode", "DLSSEnabler_SetFrameGenerationMode", &DLSSEnabler_SetFrameGenerationMode);
    setModeFunc->AddParam("Int32", "newMode");
    setModeFunc->SetReturnType("Bool");
    rtti->RegisterFunction(setModeFunc);
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGenerationMode] Registered!");
    }

    auto setFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_SetFrameGenerationState", "DLSSEnabler_SetFrameGenerationState", &DLSSEnabler_SetFrameGenerationState);
    setFunc->AddParam("Bool", "shouldEnable");
    setFunc->SetReturnType("Bool");
    rtti->RegisterFunction(setFunc);
    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetFrameGenerationState] Registered!");
    }

    auto setDFGFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_SetDynamicFrameGenerationState", "DLSSEnabler_SetDynamicFrameGenerationState", &DLSSEnabler_SetDynamicFrameGenerationState);
    setDFGFunc->AddParam("Bool", "shouldEnable");
    setDFGFunc->SetReturnType("Bool");
    rtti->RegisterFunction(setDFGFunc);
    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_SetDynamicFrameGenerationState Registered!");
    }

    auto toggleFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_ToggleFrameGenerationState", "DLSSEnabler_ToggleFrameGenerationState", &DLSSEnabler_ToggleFrameGenerationState);
    toggleFunc->SetReturnType("Bool");
    rtti->RegisterFunction(toggleFunc);
    if (g_deBridgeDebug)
    {
        sdk->logger->InfoF(handle, "[DLSSEnabler_ToggleFrameGenerationState] Registered!");
    }
}

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason, const RED4ext::Sdk* aSdk)
{
    sdk = aSdk;
    handle = aHandle;

    switch (aReason)
    {
    case RED4ext::EMainReason::Load:
    {
        auto rtti = RED4ext::CRTTISystem::Get();

        rtti->AddRegisterCallback(RegisterTypes);
        rtti->AddPostRegisterCallback(PostRegisterTypes);

        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv != nullptr)
        {
            for (int i = 0; i < argc; ++i)
            {
                if (wcscmp(argv[i], L"--de-bridge-debug") == 0)
                {
                    g_deBridgeDebug = true;
                    break;
                }
            }
            LocalFree(argv);
        }

        if (g_deBridgeDebug)
        {
            sdk->logger->Info(handle, "Debug: Comprehensive logging Enabled.");
        }

        if (!DLSSEnabler_OnInitialize())
        {
            sdk->logger->Error(handle, "Failed to initialize. Plugin may not function correctly.");
        }
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        DLSSEnabler_OnUninitialize();
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"DLSS Enabler Bridge 2077";
    aInfo->author = L"gramern";
    aInfo->version = RED4EXT_SEMVER(0, 3, 3, 0);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}