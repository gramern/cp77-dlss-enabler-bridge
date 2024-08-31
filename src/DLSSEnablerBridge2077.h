#pragma once

#include <RED4ext/RED4ext.hpp>

// Function declarations
void DLSSEnabler_GetVersionAsString(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, int64_t a4);
void DLSSEnabler_GetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut, int64_t a4);
void DLSSEnabler_GetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
void DLSSEnabler_GetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
void DLSSEnabler_SetFrameGenerationMode(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
void DLSSEnabler_SetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
void DLSSEnabler_SetDynamicFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
void DLSSEnabler_ToggleFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);

// Initialization and cleanup
bool OnInitialize();
void OnUninitialize();

// Utility functions
bool IsGameReady();
bool ShouldLog(const std::string& message);

// DLSSEnabler's API
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

// External declarations
extern const RED4ext::Sdk* sdk;
extern RED4ext::PluginHandle pluginHandle;
extern HMODULE hDll;
extern GetFrameGenerationModeFunc g_GetFrameGenerationModeFunc;
extern SetFrameGenerationModeFunc g_SetFrameGenerationModeFunc;
extern bool g_isLoggingDisabled;
extern bool g_isLastMessageRepeated;
extern bool g_deBridgeDebug;
extern bool g_deBridgeDebugExt;

// Logging macros
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

// Constants
extern const wchar_t* DLSS_ENABLER_DLL_NAME;
extern const char* LOG_MSG_CALLED;
extern const char* LOG_MSG_CALLED_SHOULD_ENABLE;
extern const char* LOG_MSG_COMPLETED;
extern const char* LOG_MSG_DISABLED;
extern const char* LOG_MSG_ENABLED;
extern const char* LOG_MSG_GAME_NOT_READY;
extern const char* LOG_MSG_FALSE;
extern const char* LOG_MSG_FUNC_ADDR_FAILED;
extern const char* LOG_MSG_FUNC_GET_ADDR_FAILED;
extern const char* LOG_MSG_FUNC_SET_ADDR_FAILED;
extern const char* LOG_MSG_NULL_OUTPUT;
extern const char* LOG_MSG_TRUE;
extern const char* LOG_MSG_UNKNOWN;
