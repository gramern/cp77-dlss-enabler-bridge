#include <iostream>
#include <windows.h>
#include <RED4ext/RED4ext.hpp>
#include <RED4ext/Scripting/IScriptable.hpp>
#include <RED4ext/RTTITypes.hpp>

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

struct DLSSEnabler : RED4ext::IScriptable
{
    static HMODULE hDll;
    static GetFrameGenerationModeFunc GetFrameGenerationMode;
    static SetFrameGenerationModeFunc SetFrameGenerationMode;

    static bool OnInitialize();
    static void OnUninitialize();

    RED4ext::CClass* GetNativeType();

    static void GetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
    static void SetFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
    static void ToggleFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4);
};

RED4ext::TTypedClass<DLSSEnabler> cls("DLSSEnabler");

RED4ext::CClass* DLSSEnabler::GetNativeType()
{
    return &cls;
}

HMODULE DLSSEnabler::hDll = nullptr;
GetFrameGenerationModeFunc DLSSEnabler::GetFrameGenerationMode = nullptr;
SetFrameGenerationModeFunc DLSSEnabler::SetFrameGenerationMode = nullptr;

bool DLSSEnabler::OnInitialize()
{
    hDll = LoadLibraryW(L"dlss-enabler.dll");
    if (!hDll)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler] Failed to load dlss-enabler.dll. Error code: %lu", error);
        return false;
    }

    GetFrameGenerationMode = (GetFrameGenerationModeFunc)GetProcAddress(hDll, "GetFrameGenerationMode");
    SetFrameGenerationMode = (SetFrameGenerationModeFunc)GetProcAddress(hDll, "SetFrameGenerationMode");

    if (!GetFrameGenerationMode || !SetFrameGenerationMode)
    {
        DWORD error = GetLastError();
        sdk->logger->ErrorF(handle, "[DLSSEnabler] Failed to get function addresses. Error code: %lu", error);
        FreeLibrary(hDll);
        hDll = nullptr;
        return false;
    }

    return true;
}

void DLSSEnabler::OnUninitialize()
{
    if (hDll)
    {
        FreeLibrary(hDll);
        hDll = nullptr;
    }
    GetFrameGenerationMode = nullptr;
    SetFrameGenerationMode = nullptr;
}

void DLSSEnabler::GetFrameGenerationState(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    //sdk->logger->InfoF(handle, "[DLSSEnablerGetFrameGeneration] called!");

    if (!GetFrameGenerationMode)
    {
        sdk->logger->Error(handle, "[DLSSEnablerGetFrameGenerationState] GetFrameGenerationMode function not available.");
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = GetFrameGenerationMode(currentMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (aOut) *aOut = (currentMode == DLSS_Enabler_FrameGeneration_Enabled);
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerGetFrameGenerationState] Failed to get Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    aFrame->code++; // skip ParamEnd
}

void DLSSEnabler::SetFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    // Get the boolean parameter
    bool shouldEnable;
    RED4ext::GetParameter(aFrame, &shouldEnable);

    if (!SetFrameGenerationMode)
    {
        sdk->logger->Error(handle, "[DLSSEnablerSetFrameGeneration] SetFrameGenerationMode function not available.");
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode newMode = shouldEnable ? DLSS_Enabler_FrameGeneration_Enabled : DLSS_Enabler_FrameGeneration_Disabled;
    DLSS_Enabler_Result result = SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerSetFrameGeneration] Failed to set Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    aFrame->code++; // skip ParamEnd
}

void DLSSEnabler::ToggleFrameGeneration(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, bool* aOut, int64_t a4)
{
    RED4EXT_UNUSED_PARAMETER(aContext);
    RED4EXT_UNUSED_PARAMETER(aFrame);
    RED4EXT_UNUSED_PARAMETER(a4);

    if (!GetFrameGenerationMode || !SetFrameGenerationMode)
    {
        sdk->logger->Error(handle, "[DLSSEnablerToggleFrameGeneration] GetFrameGenerationMode or SetFrameGenerationMode function not available.");
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode currentMode;
    DLSS_Enabler_Result result = GetFrameGenerationMode(currentMode);

    if (result != DLSS_Enabler_Result_Success)
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] GetFrameGenerationMode failed with result: %d", result);
        if (aOut) *aOut = false;
        return;
    }

    DLSS_Enabler_FrameGeneration_Mode newMode = (currentMode == DLSS_Enabler_FrameGeneration_Disabled) ?
        DLSS_Enabler_FrameGeneration_Enabled : DLSS_Enabler_FrameGeneration_Disabled;

    result = SetFrameGenerationMode(newMode);

    if (result == DLSS_Enabler_Result_Success)
    {
        if (aOut) *aOut = true;
    }
    else
    {
        sdk->logger->ErrorF(handle, "[DLSSEnablerToggleFrameGeneration] Failed to toggle Frame Generation Mode. Result: %d", result);
        if (aOut) *aOut = false;
    }

    aFrame->code++; // skip ParamEnd
}

RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
{
    RED4ext::CNamePool::Add("DLSSEnabler");
    cls.flags = { .isNative = true };
    RED4ext::CRTTISystem::Get()->RegisterType(&cls);
}

RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
{
    auto rtti = RED4ext::CRTTISystem::Get();
    auto scriptable = rtti->GetClass("IScriptable");
    cls.parent = scriptable;

    RED4ext::CBaseFunction::Flags flags = { .isNative = true, .isStatic = true };

    auto getStateFunc = RED4ext::CClassStaticFunction::Create(&cls, "GetFrameGenerationState", "GetFrameGenerationState", &DLSSEnabler::GetFrameGenerationState);
    getStateFunc->flags = flags;
    getStateFunc->SetReturnType("Bool");
    cls.RegisterFunction(getStateFunc);

    auto setFunc = RED4ext::CClassStaticFunction::Create(&cls, "SetFrameGeneration", "SetFrameGeneration", &DLSSEnabler::SetFrameGeneration);
    setFunc->flags = flags;
    setFunc->AddParam("Bool", "shouldEnable");
    setFunc->SetReturnType("Bool");
    cls.RegisterFunction(setFunc);

    auto toggleFunc = RED4ext::CClassStaticFunction::Create(&cls, "ToggleFrameGeneration", "ToggleFrameGeneration", &DLSSEnabler::ToggleFrameGeneration);
    toggleFunc->flags = flags;
    toggleFunc->SetReturnType("Bool");
    cls.RegisterFunction(toggleFunc);
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

        if (!DLSSEnabler::OnInitialize())
        {
            sdk->logger->Error(handle, "[DLSSEnabler] Failed to initialize. Plugin may not function correctly.");
        }
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        DLSSEnabler::OnUninitialize();
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"DLSS Enabler Control Interface 2077";
    aInfo->author = L"gramern";
    aInfo->version = RED4EXT_SEMVER(0, 4, 0, 0);
    aInfo->runtime = RED4EXT_RUNTIME_LATEST;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}
