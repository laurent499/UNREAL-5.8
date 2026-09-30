#include "Subsystems.h"

#include "HttpManager.h"
#include "HttpModule.h"

#define LOCTEXT_NAMESPACE "FSubsystemsModule"

void FSubsystemsModule::StartupModule()
{
}

void FSubsystemsModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSubsystemsModule, Subsystems)