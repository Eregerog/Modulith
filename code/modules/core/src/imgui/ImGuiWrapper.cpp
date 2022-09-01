#include <window/Window.h>
#include "ImGuiWrapper.h"

#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#include "GLFW/glfw3.h"

namespace modulith{

    void InitializeImGui(Window& window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // Multiple viewports not currently supported, don't enable it!

        ImGui::StyleColorsDark();

        auto background = [](float a) { return ImVec4(0.169f, 0.169f, 0.169f, a); };
        auto text = [](float a) { return ImVec4(0.793f, 0.823f, 0.834f, a); };

        auto itemInactiveWithText = []() { return ImVec4(0.859f, 0.498f, 0.239f, 0.60f); };
        auto itemHoveredWithText = []() { return ImVec4(0.886f, 0.729f, 0.353f, 0.550f); };
        auto itemActiveWithText = [](){ return ImVec4(0.886f, 0.729f, 0.353f, 0.650f); };

        auto itemInactiveNoText = [](){ return ImVec4(0.859f, 0.498f, 0.239f, 1.000f); };
        auto itemActiveNoText = [](){ return ImVec4(0.886f, 0.729f, 0.353f, 1.000f); };

        auto* style = &ImGui::GetStyle();
        style->FrameBorderSize = 1.0f;
        style->FrameRounding = 4.0f;

        style->Colors[ImGuiCol_Text] = text(1.0f);
        style->Colors[ImGuiCol_TextDisabled] = text(0.66f);

        style->Colors[ImGuiCol_WindowBg] = background(0.9f);
        style->Colors[ImGuiCol_ChildBg] = background(0.9f);
        style->Colors[ImGuiCol_PopupBg] = background(0.9f);

        style->Colors[ImGuiCol_Border] = text(0.4f);
        // No custom border shadow

        style->Colors[ImGuiCol_FrameBg] = ImVec4(0.365f, 0.326f, 0.363f, 0.900f);
        style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.559f, 0.432f, 0.597f, 0.650f);
        style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.559f, 0.432f, 0.597f, 0.650f);

        style->Colors[ImGuiCol_TitleBg] = background(0.9f);
        style->Colors[ImGuiCol_TitleBgActive] = background(0.9f);
        style->Colors[ImGuiCol_TitleBgCollapsed] = background(0.9f);
        style->Colors[ImGuiCol_MenuBarBg] = background(0.9f);

        // No custom scrollbar

        style->Colors[ImGuiCol_CheckMark] = itemInactiveNoText();

        style->Colors[ImGuiCol_SliderGrab] = itemInactiveNoText();
        style->Colors[ImGuiCol_SliderGrabActive] = itemActiveNoText();

        style->Colors[ImGuiCol_Button] = itemInactiveWithText();
        style->Colors[ImGuiCol_ButtonHovered] = itemHoveredWithText();
        style->Colors[ImGuiCol_ButtonActive] = itemActiveWithText();

        style->Colors[ImGuiCol_Header] = itemInactiveWithText();
        style->Colors[ImGuiCol_HeaderHovered] = itemHoveredWithText();
        style->Colors[ImGuiCol_HeaderActive] = itemActiveWithText();

        style->Colors[ImGuiCol_Separator] = text(0.4f);
        style->Colors[ImGuiCol_SeparatorHovered] = text(0.65f);
        style->Colors[ImGuiCol_SeparatorActive] = text(0.8f);

        // No custom resize grip for now, no idea where this is used

        style->Colors[ImGuiCol_Tab] = ImVec4(0.624f, 0.534f, 0.469f, 0.414f);
        style->Colors[ImGuiCol_TabHovered] = ImVec4(0.886f, 0.729f, 0.353f, 0.550f);
        style->Colors[ImGuiCol_TabActive] = ImVec4(0.851f, 0.497f, 0.263f, 0.650f);
        style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.444f, 0.455f, 0.470f, 0.420f);
        style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.705f, 0.723f, 0.746f, 0.500f);


        style->Colors[ImGuiCol_DockingPreview] = ImVec4(0.851f, 0.497f, 0.263f, 0.650f);

        window.InitImGui();
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ShutdownImGui(Window& window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }


    void BeginImGuiRender(Window& window, bool imguiWindowsEnabled) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(imguiWindowsEnabled){
            // Render the dockspace in the form of an "invisible window"
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            window_flags |= ImGuiWindowFlags_NoBackground;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            ImGui::Begin("ImGui Docksapce", 0, window_flags);
            ImGui::PopStyleVar(3);

            ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

            ImGui::End();
        }
    }

    void EndImGuiRender(Window& window) {

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float) window.GetWidth(), (float) window.GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

}
