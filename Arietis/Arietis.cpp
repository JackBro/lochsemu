// Arietis.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Arietis.h"
#include "winapi.h"

PluginHandle    g_handle;
Config          g_config;

ARIETIS_API bool LochsEmu_Plugin_Initialize(const LochsEmuInterface *lochsemu, PluginInfo *info)
{
    g_handle = lochsemu->Handle;
    strcpy_s(info->Name, sizeof(info->Name), "Arietis");

    std::string cfgFile = LxGetModuleDirectory(g_module) + "lochsdbg.ini";
    g_config.Initialize(cfgFile.c_str());

    if (!g_config.GetInt("General", "Enabled", 1)) {
        LxInfo("Arietis is disabled\n");
        return false;
    }

    return true;
}

ARIETIS_API void LochsEmu_Winapi_PreCall( Processor *cpu, uint apiIndex )
{
    auto dllName = LxGetWinAPIModuleName(apiIndex);
    auto apiName = LxGetWinAPIName(apiIndex);
    LxWarning("ARIETIS PRE  %s:%s\n", dllName, apiName);
}

ARIETIS_API void LochsEmu_Winapi_PostCall( Processor *cpu, uint apiIndex )
{
    auto dllName = LxGetWinAPIModuleName(apiIndex);
    auto apiName = LxGetWinAPIName(apiIndex);
    LxWarning("ARIETIS POST %s:%s\n", dllName, apiName);
}
