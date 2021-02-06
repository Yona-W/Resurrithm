#pragma once

#include <string>
#define SU_IF_KEY "Key"
#define SU_IF_SEVERITY "Severity"

enum class ScriptLogSeverity {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

class ExecutionManager;
void InterfacesRegisterEnum(ExecutionManager *exm);
void InterfacesRegisterGlobalFunction(ExecutionManager *exm);
void InterfacesRegisterSceneFunction(ExecutionManager *exm);
void InterfacesExitApplication();
void InterfacesWriteLog(ScriptLogSeverity severity, const std::string &message);
