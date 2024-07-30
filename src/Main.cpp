#include <windows.h>
#include <string_view>
#include <RED4ext/RED4ext.hpp>

const RED4ext::Sdk* sdk;
RED4ext::PluginHandle handle;

static bool g_deBridgeDebug = false;
static bool g_deBridgeDebugExt = false;
const wchar_t* DLSS_ENABLER_DLL_NAME = L"dlss-enabler.dll";

#define FUNCTION_NAME __FUNCTION__

#define LOG_DEBUG(format, ...) \
    if (g_deBridgeDebug || g_deBridgeDebugExt) \
    { \
        sdk->logger->InfoF(handle, "[%s] " format, FUNCTION_NAME, ##__VA_ARGS__); \
    }

#define LOG_DEBUG_EXT(format, ...) \
    if (g_deBridgeDebugExt) \
    { \
        sdk->logger->InfoF(handle, "[%s] " format, FUNCTION_NAME, ##__VA_ARGS__); \
    }

#define LOG_ERROR(format, ...) \
    sdk->logger->ErrorF(handle, "[%s] " format, FUNCTION_NAME, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    if (g_deBridgeDebug || g_deBridgeDebugExt) \
    { \
        sdk->logger->WarnF(handle, "[%s] " format, FUNCTION_NAME, ##__VA_ARGS__); \
    }

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

/////////////////////
// Initialize / Uninitialize
/////////////////////

bool DLSSEnabler_OnInitialize()
{
    hDll = LoadLibraryW(DLSS_ENABLER_DLL_NAME);
    if (!hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to load dlss-enabler.dll. Error code : % lu", error);
        return false;
    }

    LOG_DEBUG("dlss-enabler.dll loaded successfully");

    g_GetFrameGenerationMode = (GetFrameGenerationModeFunc)GetProcAddress(hDll, "GetFrameGenerationMode");
    g_SetFrameGenerationMode = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!g_GetFrameGenerationMode || !g_SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get function addresses. Error code: %lu", error);
        FreeLibrary(hDll);
        hDll = nullptr;
        return false;
    }

    LOG_DEBUG("Plugin has loaded succesfully");

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
    
    LOG_DEBUG("Plugin unloading...");
}

/////////////////////
// Getters
/////////////////////

void DLSSEnabler_GetVersionAsString(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT("Called!");

    std::string versionStr = "Unknown";

    DWORD verSize = GetFileVersionInfoSizeW(DLSS_ENABLER_DLL_NAME, NULL);
    if (verSize != 0)
    {
        std::vector<char> verData(verSize);
        if (GetFileVersionInfoW(DLSS_ENABLER_DLL_NAME, 0, verSize, verData.data()))
        {
            UINT size = 0;
            VS_FIXEDFILEINFO* verInfo = nullptr;
            if (VerQueryValueW(verData.data(), L"\\", (VOID FAR * FAR*) & verInfo, &size))
            {
                if (size >= sizeof(VS_FIXEDFILEINFO) && verInfo->dwSignature == 0xfeef04bd)
                {
                    char szVersion[32];
                    sprintf_s(szVersion, "%d.%d.%d.%d",
                        HIWORD(verInfo->dwFileVersionMS),
                        LOWORD(verInfo->dwFileVersionMS),
                        HIWORD(verInfo->dwFileVersionLS),
                        LOWORD(verInfo->dwFileVersionLS));
                    versionStr = szVersion;
                }
            }
        }
        LOG_DEBUG("DLL version: %s", versionStr.c_str());
    }
    else
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get DLL version info. Error code: %lu", error);
    }

    // Add null check before dereferencing aOut
    if (aOut)
    {
        *aOut = RED4ext::CString(std::string_view(versionStr));
    }
    else
    {
        LOG_WARN("Output parameter is null");
    }

    LOG_DEBUG_EXT("Completed");
}

void DLSSEnabler_GetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT("Called!");

    if (!g_GetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
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

            LOG_DEBUG("Current Frame Generation Mode: %d", currentMode);
            LOG_DEBUG("Current Frame Generation Status: %s", modeString);
        }

        if (aOut)
        {
            *aOut = currentMode;
        }
        else
        {
            LOG_WARN("Output parameter is null");
        }
    }
    else
    {
        LOG_ERROR("Failed to get Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = -1;
    }

    LOG_DEBUG_EXT("Completed");
}

void DLSSEnabler_GetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT("Called!");

    if (!g_GetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
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

            LOG_DEBUG("Current Frame Generation State: %s", modeString);
        }

        if (aOut)
        {
            *aOut = (currentMode == DLSS_Enabler_FrameGeneration_Enabled);
        }
        else
        {
            LOG_WARN("Output parameter is null");
        }
    }
    else
    {
        LOG_ERROR("Failed to get Frame Generation State. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT("Completed");
}

void DLSSEnabler_GetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT("Called!");

    if (!g_GetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
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

            LOG_DEBUG("Current Dynamic Frame Generation State: %s", modeString);
        }

        if (aOut)
        {
            *aOut = (currentMode == DLSS_Enabler_FrameGeneration_DFG_Enabled);
        }
        else
        {
            LOG_WARN("Output parameter is null");
        }
    }
    else
    {
        LOG_ERROR("Failed to get Dynamic Frame Generation State. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT("Completed");
}

/////////////////////
// Setters
/////////////////////

void DLSSEnabler_SetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    int32_t newMode;
    RED4ext::GetParameter(aFrame, &newMode);
    aFrame->code++; // skip ParamEnd

    if (newMode < 0 || newMode > 3) {
        LOG_ERROR("Invalid mode value: %d", newMode);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode modeValue = static_cast<DLSS_Enabler_FrameGeneration_Mode>(newMode);
    
    LOG_DEBUG_EXT("Called with mode = %d", newMode);

    if (!g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("SetFrameGenerationMode function address obtained successfully");

    DLSS_Enabler_Result result = g_SetFrameGenerationMode(modeValue);

    if (result == DLSS_Enabler_Result_Success)
    {
        LOG_DEBUG("Frame Generation mode set successfully: %d", newMode);
        if (aOut) *aOut = true;
    }
    else
    {
        LOG_ERROR("Failed to set Frame Generation mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT("Completed");
}

void DLSSEnabler_SetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    LOG_DEBUG_EXT("Called with shouldEnable = %s", shouldEnable ? "true" : "false");

    if (!g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("SetFrameGenerationMode function address obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_Enabled : DLSS_Enabler_FrameGeneration_Disabled;
    DLSS_Enabler_Result result = g_SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        LOG_DEBUG("Frame Generation set to %s", shouldEnable ? "Enabled" : "Disabled");
        if (aOut) *aOut = true;
    }
    else
    {
        LOG_ERROR("Failed to set Frame Generation. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT("Completed");
}

void DLSSEnabler_SetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    LOG_DEBUG_EXT("Called with shouldEnable = %s", shouldEnable ? "true" : "false");

    if (!g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("SetFrameGenerationMode function address obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_DFG_Enabled : DLSS_Enabler_FrameGeneration_DFG_Disabled;
    DLSS_Enabler_Result result = g_SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        LOG_DEBUG("Dynamic Frame Generation set to %s", shouldEnable ? "Enabled" : "Disabled");
        if (aOut) *aOut = true;
    }
    else
    {
        LOG_ERROR("Failed to set Dynamic Frame Generation. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT("Completed");
}

/////////////////////
// Togglers
/////////////////////

void DLSSEnabler_ToggleFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT("Called!");

    if (!g_GetFrameGenerationMode || !g_SetFrameGenerationMode || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to get function addresses. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("Function addresses obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result != DLSS_Enabler_Result_Success)
    {
        LOG_ERROR("GetFrameGenerationMode failed with result: %d", result);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG("Current Frame Generation Mode: %d", currentMode);

    if (currentMode == DLSS_Enabler_FrameGeneration_Disabled)
    {
        result = g_SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Enabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            LOG_DEBUG("Frame Generation set to Enabled");
            if (aOut) *aOut = true;
        }
        else
        {
            LOG_ERROR("Failed to enable Frame Generation. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else if (currentMode == DLSS_Enabler_FrameGeneration_Enabled)
    {
        result = g_SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Disabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            LOG_DEBUG("Frame Generation set to Disabled");
            if (aOut) *aOut = true;
        }
        else
        {
            LOG_ERROR("Failed to disable Frame Generation. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else
    {
        LOG_ERROR("Unexpected Frame Generation Mode: %d", currentMode);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT("Completed");
}

/////////////////////
// Registers
/////////////////////

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
{
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
{
    auto rtti = RED4ext::CRTTISystem::Get();

    auto getDLLVersionStringFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetVersionAsString", "DLSSEnabler_GetVersionAsString", &DLSSEnabler_GetVersionAsString);
    getDLLVersionStringFunc->SetReturnType("String");
    rtti->RegisterFunction(getDLLVersionStringFunc);
    LOG_DEBUG("DLSSEnabler_GetVersionAsString Registered!");

    auto getModeFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetFrameGenerationMode", "DLSSEnabler_GetFrameGenerationMode", &DLSSEnabler_GetFrameGenerationMode);
    getModeFunc->SetReturnType("Int32");
    rtti->RegisterFunction(getModeFunc);
    LOG_DEBUG("DLSSEnabler_GetFrameGenerationMode Registered!");

    auto getStateFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetFrameGenerationState", "DLSSEnabler_GetFrameGenerationState", &DLSSEnabler_GetFrameGenerationState);
    getStateFunc->SetReturnType("Bool");
    rtti->RegisterFunction(getStateFunc);
    LOG_DEBUG("DLSSEnabler_GetFrameGenerationState Registered!");

    auto getDFGStateFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_GetDynamicFrameGenerationState", "DLSSEnabler_GetDynamicFrameGenerationState", &DLSSEnabler_GetDynamicFrameGenerationState);
    getDFGStateFunc->SetReturnType("Bool");
    rtti->RegisterFunction(getDFGStateFunc);
    LOG_DEBUG("DLSSEnabler_GetDynamicFrameGenerationState Registered!");

    auto setModeFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_SetFrameGenerationMode", "DLSSEnabler_SetFrameGenerationMode", &DLSSEnabler_SetFrameGenerationMode);
    setModeFunc->AddParam("Int32", "newMode");
    setModeFunc->SetReturnType("Bool");
    rtti->RegisterFunction(setModeFunc);
    LOG_DEBUG("DLSSEnabler_SetFrameGenerationMode Registered!");

    auto setFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_SetFrameGenerationState", "DLSSEnabler_SetFrameGenerationState", &DLSSEnabler_SetFrameGenerationState);
    setFunc->AddParam("Bool", "shouldEnable");
    setFunc->SetReturnType("Bool");
    rtti->RegisterFunction(setFunc);
    LOG_DEBUG("DLSSEnabler_SetFrameGenerationState Registered!");

    auto setDFGFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_SetDynamicFrameGenerationState", "DLSSEnabler_SetDynamicFrameGenerationState", &DLSSEnabler_SetDynamicFrameGenerationState);
    setDFGFunc->AddParam("Bool", "shouldEnable");
    setDFGFunc->SetReturnType("Bool");
    rtti->RegisterFunction(setDFGFunc);
    LOG_DEBUG("DLSSEnabler_SetDynamicFrameGenerationState Registered!");

    auto toggleFunc = RED4ext::CGlobalFunction::Create("DLSSEnabler_ToggleFrameGenerationState", "DLSSEnabler_ToggleFrameGenerationState", &DLSSEnabler_ToggleFrameGenerationState);
    toggleFunc->SetReturnType("Bool");
    rtti->RegisterFunction(toggleFunc);
    LOG_DEBUG("DLSSEnabler_ToggleFrameGenerationState Registered!");
}

/////////////////////
// Main
/////////////////////

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
                }
                if (wcscmp(argv[i], L"--de-bridge-debug-ext") == 0)
                {
                    g_deBridgeDebugExt = true;
                }
            }
            LocalFree(argv);
        }

        LOG_DEBUG("Debug mode: %s, Extended debug mode: %s",
            g_deBridgeDebug ? "Enabled" : "Disabled",
            g_deBridgeDebugExt ? "Enabled" : "Disabled");

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
    aInfo->version = RED4EXT_SEMVER(0, 3, 4);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}