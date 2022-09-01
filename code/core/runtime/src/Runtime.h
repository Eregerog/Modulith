/*
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <Engine.h>

/**
 * A simple implementation of the engine that does nothing and only inherits the base class behaviour
 */
class Runtime : public modulith::Engine {
public:
    Runtime();
    ~Runtime() override;

protected:
    void OnInitialize() override;
    void OnUpdate(float deltaTime) override;
    void OnShutdown() override;
private:
};


