/*
 * \brief
 * \author Daniel Götz
 */

#include <iostream>
#include "Runtime.h"
#include "EntryPoint.h"

modulith::Engine* modulith::CreateEngineInstance() { return new Runtime(); }

Runtime::Runtime() : modulith::Engine("Runtime") {}

Runtime::~Runtime() = default;

void Runtime::OnInitialize() {
}

void Runtime::OnUpdate(float deltaTime) {

}

void Runtime::OnShutdown() {
}

