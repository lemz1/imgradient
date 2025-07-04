#pragma once

#include <imgui.h>

struct ImGradientContext;
struct ImGradientMarker;

typedef int ImGradientPickerFlags; // -> enum ImGradientPickerFlags_

// clang-format off
enum ImGradientPickerFlags_
{
  ImGradientPickerFlags_None       = 0,
  ImGradientPickerFlags_NoAlpha    = 1 << 0, // Disable alpha channel
  ImGradientPickerFlags_NoAddding  = 1 << 1, // Disable adding new markers
  ImGradientPickerFlags_NoRemoving = 1 << 2, // Disable removing markers
  ImGradientPickerFlags_NoMoving   = 1 << 3, // Disable moving markers
};
// clang-format on

namespace ImGradient
{
ImGradientContext* CreateContext();
void               DestroyContext();
ImGradientContext* GetCurrentContext();

// returns a vector containing the markers in ascending order
const ImVector<ImGradientMarker>& GradientPicker(
    const char*           label,
    ImGradientPickerFlags flags = ImGradientPickerFlags_None);

// Use this to add initial markers to a gradient picker
void AddNextGradientPickerMarker(ImVec4 color, float position);
} // namespace ImGradient
