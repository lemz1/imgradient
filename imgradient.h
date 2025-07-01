#pragma once

#include <imgui.h>

struct ImGradientContext;
struct ImGradientIO;
struct ImGradientRGB;
struct ImGradientRGBA;

// clang-format off
enum ImGradientPickerFlags
{
  ImGradientPickerFlags_None          = 0,
  ImGradientPickerFlags_NoAlpha       = 1 << 0, // Disable alpha channel
  ImGradientPickerFlags_NoAdd         = 1 << 1, // Disable adding new colors
  ImGradientPickerFlags_NoRemove      = 1 << 2, // Disable removing colors
  ImGradientPickerFlags_NoReposition  = 1 << 3, // Disable repositioning colors
};
// clang-format on

struct ImGradientIO
{
};

namespace ImGradient
{
ImGradientContext* CreateContext();
void               DestroyContext();
ImGradientContext* GetCurrentContext();
void               SetCurrentContext(ImGradientContext* ctx);

// returns true if the gradient picker is interacted with
// ImGradientPickerFlags_NoAlpha: 3 floats per color instead of 4
bool GradientPicker(
    int                   count,
    const float*          colors,
    const float*          positions,
    ImGradientPickerFlags flags = ImGradientPickerFlags_None);

void GetRepositionedIndex(int* prev_color_idx, int* new_color_idx);
} // namespace ImGradient
