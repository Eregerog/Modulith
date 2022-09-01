#pragma once
#include "Engine.h"

extern modulith::Engine* modulith::CreateEngineInstance();

int main() {
    auto* application = modulith::CreateEngineInstance();
    application->Run();
    delete application;
}