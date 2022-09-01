#pragma once

namespace modulith{

    void InitializeImGui(Window& window);
    void ShutdownImGui(Window& window);

    void BeginImGuiRender(Window& window, bool imguiWindowsEnabled);
    void EndImGuiRender(Window& window);


}
