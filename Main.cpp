#include <iostream>
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

void DLSSEnablerSetFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(a4);

    // Get the boolean parameter
    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);

    sdk->logger->InfoF(handle, "DLSSEnablerSetFrameGeneration called with shouldEnable = %s", shouldEnable ? "true" : "false");

    HMODULE hDll = LoadLibraryW(L"dlss-enabler.dll");
    if (!hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "Failed to load dlss-enabler.dll. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    sdk->logger->InfoF(handle, "dlss-enabler.dll loaded successfully");

    SetFrameGenerationModeFunc SetFrameGenerationMode = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "Failed to get SetFrameGenerationMode function address. Error code: %lu", error);
        FreeLibrary(hDll);
        if (aOut) *aOut = false;
        return;
    }

    sdk->logger->InfoF(handle, "SetFrameGenerationMode function address obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_Enabled : DLSS_Enabler_FrameGeneration_Disabled;
    DLSS_Enabler_Result result = SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        sdk->logger->InfoF(handle, "Frame Generation Mode set to %s", shouldEnable ? "Enabled" : "Disabled");
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "Failed to set Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    FreeLibrary(hDll);
    sdk->logger->InfoF(handle, "DLSSEnablerSetFrameGeneration completed");

    aFrame->code++; // skip ParamEnd
}

void DLSSEnablerToggleFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    sdk->logger->InfoF(handle, "DLSSEnablerToggleFrameGeneration called!");

    HMODULE hDll = LoadLibraryW(L"dlss-enabler.dll");
    if (!hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "Failed to load dlss-enabler.dll. Error code: %lu", error);
        if (aOut) *aOut = false;
        return;
    }

    sdk->logger->InfoF(handle, "dlss-enabler.dll loaded successfully");

    GetFrameGenerationModeFunc GetFrameGenerationMode = (GetFrameGenerationModeFunc)GetProcAddress(hDll, "GetFrameGenerationMode");
    SetFrameGenerationModeFunc SetFrameGenerationMode = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!GetFrameGenerationMode || !SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "Failed to get function addresses. Error code: %lu", error);
        FreeLibrary(hDll);
        if (aOut) *aOut = false;
        return;
    }

    sdk->logger->InfoF(handle, "Function addresses obtained successfully");

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = GetFrameGenerationMode(currentMode);

    if (result != DLSS_Enabler_Result_Success)
    {
        sdk->logger->ErrorF(handle, "GetFrameGenerationMode failed with result: %d", result);
        FreeLibrary(hDll);
        if (aOut) *aOut = false;
        return;
    }

    sdk->logger->InfoF(handle, "Current Frame Generation Mode: %d", currentMode);

    if (currentMode == DLSS_Enabler_FrameGeneration_Disabled)
    {
        result = SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Enabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            sdk->logger->InfoF(handle, "Frame Generation Mode set to Enabled");
            if (aOut) *aOut = true;
        }
        else
        {
            sdk->logger->ErrorF(handle, "Failed to enable Frame Generation Mode. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else if (currentMode == DLSS_Enabler_FrameGeneration_Enabled)
    {
        result = SetFrameGenerationMode(DLSS_Enabler_FrameGeneration_Disabled);
        if (result == DLSS_Enabler_Result_Success)
        {
            sdk->logger->InfoF(handle, "Frame Generation Mode set to Disabled");
            if (aOut) *aOut = true;
        }
        else
        {
            sdk->logger->ErrorF(handle, "Failed to disable Frame Generation Mode. Result: %d", result);
            if (aOut) *aOut = false;
        }
    }
    else
    {
        sdk->logger->ErrorF(handle, "Unexpected Frame Generation Mode: %d", currentMode);
        if (aOut) *aOut = false;
    }

    FreeLibrary(hDll);
    sdk->logger->InfoF(handle, "DLSSEnablerToggleFrameGeneration completed");
}

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
{
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
{
    auto rtti = RED4ext::CRTTISystem::Get();

    auto func = RED4ext::CGlobalFunction::Create("DLSSEnablerSetFrameGeneration", "DLSSEnablerSetFrameGeneration", &DLSSEnablerSetFrameGeneration);
    func->AddParam("Bool", "shouldEnable");
    func->SetReturnType("Bool");
    rtti->RegisterFunction(func);
    sdk->logger->InfoF(handle, "DLSSEnablerSetFrameGeneration registered!");

    auto toggleFunc = RED4ext::CGlobalFunction::Create("DLSSEnablerToggleFrameGeneration", "DLSSEnablerToggleFrameGeneration", &DLSSEnablerToggleFrameGeneration);
    toggleFunc->SetReturnType("Bool");
    rtti->RegisterFunction(toggleFunc);
    sdk->logger->InfoF(handle, "DLSSEnablerToggleFrameGeneration registered!");
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
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"DLSS Enabler Control Interface";
    aInfo->author = L"gramern";
    aInfo->version = RED4EXT_SEMVER(0, 2, 0, 0);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}