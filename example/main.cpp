#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgradient.h>
#include <glfw/glfw3.h>

#include <iostream>

int main()
{
  if (!glfwInit())
  {
    return 1;
  }

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGradient Example", nullptr, nullptr);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  ImGui::CreateContext();
  ImGradient::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    glfwSwapBuffers(window);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    ImGui::Begin("Gradient Picker");
    ImGradient::AddInitialMarker(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f);
    ImGradient::AddInitialMarker(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f);
    ImGradient::AddInitialMarker(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f);
    ImGradient::GradientPicker("Picker");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();

  ImGradient::DestroyContext();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}
