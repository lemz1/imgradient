#pragma once

#include <imgui.h>
#include "imgradient.h"

struct ImGradientPicker
{
  ImVector<ImGradientMarker> Markers;
  int                        SelectedIdx;
};

struct ImGradientContext
{
  bool IsDraggingMarker = false;

  ImGuiID                    CurrentPicker;
  ImVector<ImGradientPicker> Pickers;
  ImGuiStorage               IdToPickerIdx;
  ImGradientPickerFlags      PickerFlags;

  ImVector<ImGradientMarker> NextMarkers;
};

namespace ImGradient
{
ImGradientPicker& GetOrCreatePicker(ImGuiID id);
ImGradientPicker& GetCurrentPicker();

void AddColor(float position);
void RemoveColor(int idx);
void SortMarkers();
} // namespace ImGradient
