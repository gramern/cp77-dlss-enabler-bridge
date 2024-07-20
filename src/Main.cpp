#include <windows.h>
#include <RED4ext/RED4ext.hpp>

const RED4ext::Sdk* sdk;
RED4ext::PluginHandle handle;

typedef enum DLSS_Enabler_FrameGeneration_Mode
{
    DLSS_Enabler_FrameGeneration_Disabled = 0,
    DLSS_Enabler_FrameGeneration_Enabled = 1,
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

bool DLSSEnablerOnInitialize()
{
    hDll = LoadLibraryW(L"dlss-enabler.dll");
    if (!hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnablerControlInterface] Failed to load dlss-enabler.dll. Error code: %lu", error);
        return false;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerOnInitialize] dlss-enabler.dll loaded successfully");

    g_GetFrameGenerationMode = (GetFrameGenerationModeFunc)GetProcAddress(hDll, "GetFrameGenerationMode");
    g_SetFrameGenerationMode = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!g_GetFrameGenerationMode || !g_SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnablerControlInterface] Failed to get function addresses. Error code: %lu", error);
        FreeLibrary(hDll);
        return false;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerOnInitialize] Plugin has loaded succesfully");

    return true;
}

void DLSSEnablerOnUninitialize()
{
    if (hDll)
    {
        g_GetFrameGenerationMode = nullptr;
        g_SetFrameGenerationMode = nullptr;
        FreeLibrary(hDll);
    }
}

void DLSSEnablerGetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    //sdk->logger->InfoF(handle, "[DLSSEnablerGetFrameGeneration] called!");

    if (!g_GetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnablerGetFrameGenerationState] Failed to get GetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (aOut) *aOut = (currentMode == DLSS_Enabler_FrameGeneration_Enabled);
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerGetFrameGenerationState] Failed to get Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerGetFrameGeneration] Completed");

    aFrame->code++; // skip ParamEnd
}

void DLSSEnablerSetFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    // Get the boolean parameter
    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);
    aFrame->code++; // skip ParamEnd

    //sdk->logger->InfoF(handle, "[DLSSEnablerSetFrameGeneration] Called with shouldEnable = %s", shouldEnable ? "true" : "false");

    if (!g_SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnablerSetFrameGeneration] Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerSetFrameGeneration] SetFrameGenerationMode function address obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_Enabled : DLSS_Enabler_FrameGeneration_Disabled;
    DLSS_Enabler_Result result = g_SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        //sdk->logger->InfoF(handle, "[DLSSEnablerSetFrameGeneration] Frame Generation Mode set to %s", shouldEnable ? "Enabled" : "Disabled");
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerSetFrameGeneration] Failed to set Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerSetFrameGeneration] Completed");
}

void DLSSEnablerToggleFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] called!");

    if (!g_GetFrameGenerationMode || !g_SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] Failed to get function addresses. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] Function addresses obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = g_GetFrameGenerationMode(currentMode);

    if (result != DLSS_Enabler_Result_Success)
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] GetFrameGenerationMode failed with result: %d", result);
        if (aOut) *aOut = false;
        return;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] Current Frame Generation Mode: %d", currentMode);

    if (currentMode == DLSS_Enabler_FrameGeneration_Disabled)
    {
        result = g_SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Enabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] Frame Generation Mode set to Enabled");
            if (aOut) *aOut = true;
        }
        else
        {
            sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] Failed to enable Frame Generation Mode. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else if (currentMode == DLSS_Enabler_FrameGeneration_Enabled)
    {
        result = g_SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Disabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] Frame Generation Mode set to Disabled");
            if (aOut) *aOut = true;
        }
        else
        {
            sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] Failed to disable Frame Generation Mode. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] Unexpected Frame Generation Mode: %d", currentMode);
        if (aOut) *aOut = false;
    }

    //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] Completed");

    aFrame->code++; // skip ParamEnd
}

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
{
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
{
    auto rtti = RED4ext::CRTTISystem::Get();

    auto getStateFunc = RED4ext::CGlobalFunction::Create("DLSSEnablerGetFrameGenerationState", "DLSSEnablerGetFrameGenerationState", &DLSSEnablerGetFrameGenerationState);
    getStateFunc->SetReturnType("Bool");
    rtti->RegisterFunction(getStateFunc);
    //sdk->logger->InfoF(handle, "[DLSSEnablerGetFrameGenerationState] Registered!");

    auto func = RED4ext::CGlobalFunction::Create("DLSSEnablerSetFrameGeneration", "DLSSEnablerSetFrameGeneration", &DLSSEnablerSetFrameGeneration);
    func->AddParam("Bool", "shouldEnable");
    func->SetReturnType("Bool");
    rtti->RegisterFunction(func);
    //sdk->logger->InfoF(handle, "[DLSSEnablerSetFrameGeneration] Registered!");

    auto toggleFunc = RED4ext::CGlobalFunction::Create("DLSSEnablerToggleFrameGeneration", "DLSSEnablerToggleFrameGeneration", &DLSSEnablerToggleFrameGeneration);
    toggleFunc->SetReturnType("Bool");
    rtti->RegisterFunction(toggleFunc);
    //sdk->logger->InfoF(handle, "[DLSSEnablerToggleFrameGeneration] Registered!");
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

        if (!DLSSEnablerOnInitialize())
        {
            sdk->logger->Error(handle, "[DLSSEnabler] Failed to initialize. Plugin may not function correctly.");
        }
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        DLSSEnablerOnUninitialize();
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"DLSS Enabler Control Interface 2077";
    aInfo->author = L"gramern";
    aInfo->version = RED4EXT_SEMVER(0, 3, 1, 1);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}