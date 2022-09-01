/*
 * \brief
 * \author Daniel Götz
 */

#include "MarioModule.h"
#include "MarioSystem.h"

// TODO: Automate most of this, expose a function to the user

using namespace modulith;

extern "C++" {

void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {

    module.Register<SystemResource<MarioSystem>>();
}


void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module){
}


}
