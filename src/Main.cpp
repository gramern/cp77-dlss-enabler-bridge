#include <windows.h>
#include <string>
#include <string_view>
#include <RED4ext/RED4ext.hpp>

const RED4ext::Sdk* sdk;
RED4ext::PluginHandle pluginHandle;

static bool g_deBridgeDebug = false;
static bool g_deBridgeDebugExt = false;
const wchar_t* DLSS_ENABLER_DLL_NAME = L"dlss-enabler.dll";

////////////////////////
// Logging Variables
////////////////////////

std::string g_lastLoggedMessage;
bool g_isLoggingDisabled = false;
bool g_isLastMessageRepeated = false;

#define FUNCTION_NAME __FUNCTION__

#define LOG_DEBUG(format, ...) \
    do { \
        if (!g_isLoggingDisabled && (g_deBridgeDebug || g_deBridgeDebugExt)) { \
            char buffer[256]; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            std::string message = std::string("[") + FUNCTION_NAME + "] " + buffer; \
            if (ShouldLog(message)) { \
                sdk->logger->InfoF(pluginHandle, "%s", message.c_str()); \
            } \
        } \
    } while(0)

#define LOG_DEBUG_EXT(format, ...) \
    do { \
        if (!g_isLoggingDisabled && g_deBridgeDebugExt) { \
            char buffer[256]; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            std::string message = std::string("[") + FUNCTION_NAME + "] " + buffer; \
            if (ShouldLog(message)) { \
                sdk->logger->InfoF(pluginHandle, "%s", message.c_str()); \
            } \
        } \
    } while(0)

#define LOG_ERROR(format, ...) \
    do { \
        if (!g_isLoggingDisabled) { \
            char buffer[256]; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            std::string message = std::string("[") + FUNCTION_NAME + "] " + buffer; \
            if (ShouldLog(message)) { \
                sdk->logger->ErrorF(pluginHandle, "%s", message.c_str()); \
            } \
        } \
    } while(0)

#define LOG_WARN(format, ...) \
    do { \
        if (!g_isLoggingDisabled && (g_deBridgeDebug || g_deBridgeDebugExt)) { \
            char buffer[256]; \
            snprintf(buffer, sizeof(buffer), format, ##__VA_ARGS__); \
            std::string message = std::string("[") + FUNCTION_NAME + "] " + buffer; \
            if (ShouldLog(message)) { \
                sdk->logger->WarnF(pluginHandle, "%s", message.c_str()); \
            } \
        } \
    } while(0)

// Log message constants
const char* LOG_MSG_CALLED = "Called!";
const char* LOG_MSG_CALLED_SHOULD_ENABLE = "Called with shouldEnable = %s";
const char* LOG_MSG_COMPLETED = "Completed";
const char* LOG_MSG_DISABLED = "Disabled";
const char* LOG_MSG_ENABLED = "Enabled";
const char* LOG_MSG_GAME_NOT_READY = "The game is paused, or in the main menu. Communication with DLSS Enabler is halted.";
const char* LOG_MSG_FALSE = "false";
const char* LOG_MSG_FUNC_ADDR_FAILED = "Failed to get function addresses. Error code: %lu";
const char* LOG_MSG_FUNC_GET_ADDR_FAILED = "Failed to get GetFrameGenerationMode function address. Error code: %lu";
const char* LOG_MSG_FUNC_SET_ADDR_FAILED = "Failed to get SetFrameGenerationMode function address. Error code: %lu";
const char* LOG_MSG_NULL_OUTPUT = "Output parameter is null";
const char* LOG_MSG_TRUE = "true";
const char* LOG_MSG_UNKNOWN = "Unknown";

////////////////////////
// DLSSEnabler's API 
////////////////////////

typedef enum DLSS_ENABLER_FRAMEGENERATION_MODE
{
    DLSS_ENABLER_FRAMEGENERATION_DISABLED = 0,
    DLSS_ENABLER_FRAMEGENERATION_ENABLED = 1,
    DLSS_ENABLER_FRAMEGENERATION_DFG_DISABLED = 2,
    DLSS_ENABLER_FRAMEGENERATION_DFG_ENABLED = 3,
} DLSS_ENABLER_FRAMEGENERATION_MODE;

typedef enum DLSS_ENABLER_RESULT
{
    DLSS_ENABLER_RESULT_SUCCESS = 1,
    DLSS_ENABLER_RESULT_FAIL_UNSUPPORTED = 0,
    DLSS_ENABLER_RESULT_FAIL_BAD_ARGUMENT = -1,
} DLSS_ENABLER_RESULT;

typedef DLSS_ENABLER_RESULT(*GetFrameGenerationModeFunc)(DLSS_ENABLER_FRAMEGENERATION_MODE& mode);
typedef DLSS_ENABLER_RESULT(*SetFrameGenerationModeFunc)(DLSS_ENABLER_FRAMEGENERATION_MODE mode);

static HMODULE hDll;
static GetFrameGenerationModeFunc g_GetFrameGenerationModeFunc = nullptr;
static SetFrameGenerationModeFunc g_SetFrameGenerationModeFunc = nullptr;

////////////////////////
// Restrict Logging: in case the modded Frame Generation goes *^(!$^% or methods are called excessively when FG is turned off in the game settings
////////////////////////

bool ShouldLog(const std::string& message) 
{
    if (g_isLoggingDisabled) {
        return false;
    }

    if (message == g_lastLoggedMessage) {
        if (!g_isLastMessageRepeated) {
            g_isLastMessageRepeated = true;
            return true;
        }
        return false;
    }
    else {
        g_lastLoggedMessage = message;
        g_isLastMessageRepeated = false;
        return true;
    }
}

/////////////////////
// Initialize / Uninitialize
/////////////////////

bool OnInitialize()
{
    g_lastLoggedMessage = "";
    g_isLoggingDisabled = false;
    g_isLastMessageRepeated = false;

    hDll = LoadLibraryW(DLSS_ENABLER_DLL_NAME);
    if (!hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR("Failed to load dlss-enabler.dll. Error code : % lu", error);
        return false;
    }

    LOG_DEBUG("dlss-enabler.dll loaded successfully");

    g_GetFrameGenerationModeFunc = (GetFrameGenerationModeFunc)GetProcAddress(hDll, "GetFrameGenerationMode");
    g_SetFrameGenerationModeFunc = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!g_GetFrameGenerationModeFunc || !g_SetFrameGenerationModeFunc)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_ADDR_FAILED, error);
        FreeLibrary(hDll);
        hDll = nullptr;
        return false;
    }

    LOG_DEBUG("Plugin has loaded successfully");

    return true;
}

void OnUninitialize()
{
    g_lastLoggedMessage = "";
    g_isLoggingDisabled = false;
    g_isLastMessageRepeated = false;

    if (hDll)
    {
        g_GetFrameGenerationModeFunc = nullptr;
        g_SetFrameGenerationModeFunc = nullptr;
        FreeLibrary(hDll);
        hDll = nullptr;
    }
    
    LOG_DEBUG("Plugin unloading...");
}

////////////////////////
// Check The Game: is the game ready to use the API?
////////////////////////

bool IsGameReady()
{
    LOG_DEBUG_EXT(LOG_MSG_CALLED);

    auto gameInstance = RED4ext::CGameEngine::Get()->framework->gameInstance;

    if (!gameInstance)
    {
        LOG_WARN(LOG_MSG_GAME_NOT_READY);
        return false;
    }

    auto rtti = RED4ext::CRTTISystem::Get();
    auto inkMenuScenCls = rtti->GetClass("inkMenuScenario");

    RED4ext::WeakHandle<RED4ext::IScriptable> weakHandle;
    RED4ext::ExecuteFunction(gameInstance, inkMenuScenCls->GetFunction("GetSystemRequestsHandler"), &weakHandle);

    auto instance = weakHandle.Lock();

    if (!instance)
    {
        LOG_WARN(LOG_MSG_GAME_NOT_READY);
        return false;
    }

    auto inkSysReqHandCls = rtti->GetClass("inkISystemRequestsHandler");
    bool isPreGame;
    bool isGamePaused;

    RED4ext::ExecuteFunction(instance, inkSysReqHandCls->GetFunction("IsPreGame"), &isPreGame);
    RED4ext::ExecuteFunction(instance, inkSysReqHandCls->GetFunction("IsGamePaused"), &isGamePaused);

    bool isReady = !isPreGame && !isGamePaused;
    if (isReady)
    {
        LOG_DEBUG_EXT("The game should be ready for API communication, if running in the normal mode.");
    }
    else
    {
        LOG_WARN(LOG_MSG_GAME_NOT_READY);
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
    return isReady;
}

/////////////////////
// Getters
/////////////////////

void DLSSEnabler_GetVersionAsString(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT(LOG_MSG_CALLED);

    std::string versionStr = LOG_MSG_UNKNOWN;

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

    if (aOut)
    {
        *aOut = RED4ext::CString(std::string_view(versionStr));
    }
    else
    {
        LOG_WARN(LOG_MSG_NULL_OUTPUT);
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

void DLSSEnabler_GetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT(LOG_MSG_CALLED);

    if (!IsGameReady())
    {
        if (aOut) *aOut = DLSS_ENABLER_RESULT_FAIL_UNSUPPORTED;
        return;
    }

    if (!g_GetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_GET_ADDR_FAILED, error);
        if (aOut) *aOut = DLSS_ENABLER_RESULT_FAIL_UNSUPPORTED;
        return;
    }

    DLSS_ENABLER_FRAMEGENERATION_MODE currentMode;
    DLSS_ENABLER_RESULT result = g_GetFrameGenerationModeFunc(currentMode);

    if (result == DLSS_ENABLER_RESULT_SUCCESS)
    {
        if (g_deBridgeDebug)
        {
            const char* modeString;
            switch (currentMode)
            {
            case DLSS_ENABLER_FRAMEGENERATION_DISABLED:
                modeString = "FG Disabled; DFG Disabled";
                break;
            case DLSS_ENABLER_FRAMEGENERATION_ENABLED:
                modeString = "FG Enabled; DFG Disabled";
                break;
            case DLSS_ENABLER_FRAMEGENERATION_DFG_DISABLED:
                modeString = "FG Unknown; DFG Disabled";
                break;
            case DLSS_ENABLER_FRAMEGENERATION_DFG_ENABLED:
                modeString = "FG Unknown; DFG Enabled";
                break;
            default:
                modeString = LOG_MSG_UNKNOWN;
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
            LOG_WARN(LOG_MSG_NULL_OUTPUT);
        }
    }
    else
    {
        LOG_ERROR("Failed to get Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = -1;
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

void DLSSEnabler_GetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT(LOG_MSG_CALLED);

    if (!IsGameReady())
    {
        if (aOut) *aOut = false;
        return;
    }

    if (!g_GetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_GET_ADDR_FAILED, error);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_ENABLER_FRAMEGENERATION_MODE currentMode;
    DLSS_ENABLER_RESULT result = g_GetFrameGenerationModeFunc(currentMode);

    if (result == DLSS_ENABLER_RESULT_SUCCESS)
    {
        if (g_deBridgeDebug)
        {
            const char* modeString;
            switch (currentMode)
            {
            case DLSS_ENABLER_FRAMEGENERATION_ENABLED:
                modeString = LOG_MSG_ENABLED;
                break;
            case DLSS_ENABLER_FRAMEGENERATION_DISABLED:
                modeString = LOG_MSG_DISABLED;
                break;
            default:
                modeString = LOG_MSG_UNKNOWN;
            }

            LOG_DEBUG("Current Frame Generation State: %s", modeString);
        }

        if (aOut)
        {
            *aOut = (currentMode == DLSS_ENABLER_FRAMEGENERATION_ENABLED);
        }
        else
        {
            LOG_WARN(LOG_MSG_NULL_OUTPUT);
        }
    }
    else
    {
        LOG_ERROR("Failed to get Frame Generation State. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

void DLSSEnabler_GetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT(LOG_MSG_CALLED);

    if (!IsGameReady())
    {
        if (aOut) *aOut = false;
        return;
    }

    if (!g_GetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_GET_ADDR_FAILED, error);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_ENABLER_FRAMEGENERATION_MODE currentMode;
    DLSS_ENABLER_RESULT result = g_GetFrameGenerationModeFunc(currentMode);

    if (result == DLSS_ENABLER_RESULT_SUCCESS)
    {
        if (g_deBridgeDebug)
        {
            const char* modeString;
            switch (currentMode)
            {
            case DLSS_ENABLER_FRAMEGENERATION_DFG_ENABLED:
                modeString = LOG_MSG_ENABLED;
                break;
            case DLSS_ENABLER_FRAMEGENERATION_DFG_DISABLED:
                modeString = LOG_MSG_DISABLED;
                break;
            case DLSS_ENABLER_FRAMEGENERATION_ENABLED:
                modeString = LOG_MSG_DISABLED;
                break;
            case DLSS_ENABLER_FRAMEGENERATION_DISABLED:
                modeString = LOG_MSG_DISABLED;
                break;
            default:
                modeString = LOG_MSG_UNKNOWN;
            }

            LOG_DEBUG("Current Dynamic Frame Generation State: %s", modeString);
        }

        if (aOut)
        {
            *aOut = (currentMode == DLSS_ENABLER_FRAMEGENERATION_DFG_ENABLED);
        }
        else
        {
            LOG_WARN(LOG_MSG_NULL_OUTPUT);
        }
    }
    else
    {
        LOG_ERROR("Failed to get Dynamic Frame Generation State. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

/////////////////////
// Setters
/////////////////////

void DLSSEnabler_SetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (!IsGameReady())
    {
        if (aOut) *aOut = false;
        return;
    }

    int32_t newMode;
    RED4ext::GetParameter(aFrame, &newMode);
    aFrame->code++; // skip ParamEnd

    if (newMode < 0 || newMode > 3) {
        LOG_ERROR("Invalid mode value: %d", newMode);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_ENABLER_FRAMEGENERATION_MODE modeValue = static_cast<DLSS_ENABLER_FRAMEGENERATION_MODE>(newMode);
    
    LOG_DEBUG_EXT("Called with mode = %d", newMode);

    if (!g_SetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_SET_ADDR_FAILED, error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("SetFrameGenerationMode function address obtained successfully");

    DLSS_ENABLER_RESULT result = g_SetFrameGenerationModeFunc(modeValue);

    if (result == DLSS_ENABLER_RESULT_SUCCESS)
    {
        LOG_DEBUG("Frame Generation mode set successfully: %d", newMode);
        if (aOut) *aOut = true;
    }
    else
    {
        LOG_ERROR("Failed to set Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

void DLSSEnabler_SetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (!IsGameReady())
    {
        if (aOut) *aOut = false;
        return;
    }

    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    LOG_DEBUG_EXT(LOG_MSG_CALLED_SHOULD_ENABLE, shouldEnable ? LOG_MSG_TRUE : LOG_MSG_FALSE);

    if (!g_SetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_SET_ADDR_FAILED, error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("SetFrameGenerationMode function address obtained successfully");

    DLSS_ENABLER_FRAMEGENERATION_MODE newMode = shouldEnable ? DLSS_ENABLER_FRAMEGENERATION_ENABLED : DLSS_ENABLER_FRAMEGENERATION_DISABLED;
    DLSS_ENABLER_RESULT result = g_SetFrameGenerationModeFunc(newMode);

    if (result == DLSS_ENABLER_RESULT_SUCCESS)
    {
        LOG_DEBUG("Frame Generation set to %s", shouldEnable ? LOG_MSG_ENABLED : LOG_MSG_DISABLED);
        if (aOut) *aOut = true;
    }
    else
    {
        LOG_ERROR("Failed to set Frame Generation. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

void DLSSEnabler_SetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (!IsGameReady())
    {
        if (aOut) *aOut = false;
        return;
    }

    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    LOG_DEBUG_EXT(LOG_MSG_CALLED_SHOULD_ENABLE, shouldEnable ? LOG_MSG_TRUE : LOG_MSG_FALSE);

    if (!g_SetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_SET_ADDR_FAILED, error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("SetFrameGenerationMode function address obtained successfully");

    DLSS_ENABLER_FRAMEGENERATION_MODE newMode = shouldEnable ? DLSS_ENABLER_FRAMEGENERATION_DFG_ENABLED : DLSS_ENABLER_FRAMEGENERATION_DFG_DISABLED;
    DLSS_ENABLER_RESULT result = g_SetFrameGenerationModeFunc(newMode);

    if (result == DLSS_ENABLER_RESULT_SUCCESS)
    {
        LOG_DEBUG("Dynamic Frame Generation set to %s", shouldEnable ? LOG_MSG_ENABLED : LOG_MSG_DISABLED);
        if (aOut) *aOut = true;
    }
    else
    {
        LOG_ERROR("Failed to set Dynamic Frame Generation. Result: %d", result);
        if (aOut) *aOut = false;
    }

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
}

/////////////////////
// Togglers
/////////////////////

void DLSSEnabler_ToggleFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    LOG_DEBUG_EXT(LOG_MSG_CALLED);

    if (!IsGameReady())
    {
        if (aOut) *aOut = false;
        return;
    }

    if (!g_GetFrameGenerationModeFunc || !g_SetFrameGenerationModeFunc || !hDll)
    {
        DWORD error = GetLastError();
        LOG_ERROR(LOG_MSG_FUNC_ADDR_FAILED, error);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG_EXT("Function addresses obtained successfully");

    DLSS_ENABLER_FRAMEGENERATION_MODE currentMode;
    DLSS_ENABLER_RESULT result = g_GetFrameGenerationModeFunc(currentMode);

    if (result != DLSS_ENABLER_RESULT_SUCCESS)
    {
        LOG_ERROR("GetFrameGenerationMode failed with result: %d", result);
        if (aOut) *aOut = false;
        return;
    }

    LOG_DEBUG("Current Frame Generation Mode: %d", currentMode);

    if (currentMode == DLSS_ENABLER_FRAMEGENERATION_DISABLED)
    {
        result = g_SetFrameGenerationModeFunc(DLSS_ENABLER_FRAMEGENERATION_ENABLED);
        if (result == DLSS_ENABLER_RESULT_SUCCESS)
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
    else if (currentMode == DLSS_ENABLER_FRAMEGENERATION_ENABLED)
    {
        result = g_SetFrameGenerationModeFunc(DLSS_ENABLER_FRAMEGENERATION_DISABLED);
        if (result == DLSS_ENABLER_RESULT_SUCCESS)
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

    LOG_DEBUG_EXT(LOG_MSG_COMPLETED);
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
    pluginHandle = aHandle;

    g_deBridgeDebug = false;
    g_deBridgeDebugExt = false;

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
            g_deBridgeDebug ? LOG_MSG_ENABLED : LOG_MSG_DISABLED,
            g_deBridgeDebugExt ? LOG_MSG_ENABLED : LOG_MSG_DISABLED);

        if (!OnInitialize())
        {
            sdk->logger->Error(pluginHandle, "Failed to initialize. Plugin may not function correctly.");
        }
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        OnUninitialize();
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"DLSS Enabler Bridge 2077";
    aInfo->author = L"gramern";
    aInfo->version = RED4EXT_SEMVER(0, 4, 0);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}