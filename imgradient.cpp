#include "imgradient.h"

#include "imgui_internal.h"

#include <cmath>
#include <glfw/glfw3.h>
#include "imgradient_internal.h"

#include <algorithm>
#include <iostream>

static ImGradientContext* GImGradient = NULL;

namespace ImGradient
{
static void Checkerboard(ImDrawList* draw_list, ImRect rect);
static void Gradient(ImDrawList* draw_list, ImRect rect);
static bool Marker(ImDrawList* draw_list, ImRect rect, ImVec2 marker_size, float position);

const ImVector<ImGradientMarker>& GradientPicker(const char* label, ImGradientPickerFlags flags)
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();
  ImDrawList*        d = ImGui::GetWindowDrawList();

  g.PickerFlags = flags;

  ImGradientPicker& picker = GetOrCreatePicker(ImGui::GetID(label));

  ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
  ImVec2 gradient_size = ImVec2(ImGui::GetContentRegionAvail().x, 20.0f);

  ImVec2 marker_size = ImVec2(gradient_size.y * 0.6f, gradient_size.y * 0.6f);

  ImRect gradient_rect = ImRect(
      ImVec2(cursor_pos.x + marker_size.x * 0.5f, cursor_pos.y),
      ImVec2(
          cursor_pos.x + gradient_size.x - marker_size.x * 0.5f, cursor_pos.y + gradient_size.y));

  Checkerboard(d, gradient_rect);

  Gradient(d, gradient_rect);

  bool dragging_marker = false;
  int  new_selected_idx = picker.SelectedIdx;
  for (int i = 0; i < picker.Markers.size(); i++)
  {
    if (i == picker.SelectedIdx)
    {
      continue;
    }

    ImGui::PushID(i);
    if (Marker(d, gradient_rect, marker_size, picker.Markers[i].Position))
    {
      dragging_marker = true;
      new_selected_idx = i;
    }
    ImGui::PopID();
  }

  ImGui::PushID(picker.SelectedIdx);
  if (Marker(d, gradient_rect, marker_size, picker.Markers[picker.SelectedIdx].Position))
  {
    dragging_marker = true;
    new_selected_idx = picker.SelectedIdx;
  }
  ImGui::PopID();

  if (!g.IsDraggingMarker)
  {
    picker.SelectedIdx = new_selected_idx;
    g.IsDraggingMarker = dragging_marker;
  }

  if (g.IsDraggingMarker)
  {
    float prev_mouse_x = ImGui::GetIO().MousePosPrev.x;
    float mouse_x = ImGui::GetIO().MousePos.x;

    if (prev_mouse_x != -FLT_MAX && mouse_x != -FLT_MAX)
    {
      float delta_x = mouse_x - prev_mouse_x;
      float cur_pos = picker.Markers[picker.SelectedIdx].Position;
      float marker_x =
          (gradient_rect.Min.x + (gradient_rect.Max.x - gradient_rect.Min.x) * cur_pos) + delta_x;
      float clamped_x =
          (marker_x < gradient_rect.Min.x
               ? gradient_rect.Min.x
               : (marker_x > gradient_rect.Max.x ? gradient_rect.Max.x : marker_x));
      float new_pos =
          (clamped_x - gradient_rect.Min.x) / (gradient_rect.Max.x - gradient_rect.Min.x);
      picker.Markers[picker.SelectedIdx].Position = new_pos;

      if ((picker.SelectedIdx - 1 >= 0 &&
           picker.Markers[picker.SelectedIdx - 1].Position > new_pos) ||
          (picker.SelectedIdx + 1 < picker.Markers.size() &&
           picker.Markers[picker.SelectedIdx + 1].Position < new_pos))
      {
        SortMarkers();
      }
    }
  }

  ImGui::ButtonBehavior(gradient_rect, g.CurrentPicker, NULL, NULL);

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
  {
    g.IsDraggingMarker = false;
  }

  g.NextMarkers.resize(0);

  return picker.Markers;
}

void AddNextGradientPickerMarker(ImVec4 color, float position)
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();

  g.NextMarkers.push_back(ImGradientMarker{color, position});
}

ImGradientContext* CreateContext()
{
  IM_ASSERT(!GImGradient);
  GImGradient = IM_NEW(ImGradientContext)();
  return GImGradient;
}

void DestroyContext()
{
  IM_ASSERT(GImGradient);
  IM_DELETE(GImGradient);
  GImGradient = NULL;
}

ImGradientContext* GetCurrentContext() { return GImGradient; }

ImGradientPicker& GetOrCreatePicker(ImGuiID id)
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();
  g.CurrentPicker = id;

  int idx = g.IdToPickerIdx.GetInt(id, -1);
  if (idx != -1)
  {
    return g.Pickers[idx];
  }
  else
  {
    g.Pickers.push_back(ImGradientPicker());
    ImGradientPicker& picker = g.Pickers[g.Pickers.size() - 1];
    if (g.NextMarkers.size() > 0)
    {
      picker.Markers = g.NextMarkers;
    }
    else
    {
      picker.Markers.push_back(ImGradientMarker{ImVec4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f});
      picker.Markers.push_back(ImGradientMarker{ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f});
    }
    picker.SelectedIdx = 0;
    g.IdToPickerIdx.SetInt(id, g.Pickers.size() - 1);
    return picker;
  }
}

ImGradientPicker& GetCurrentPicker()
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();

  int picker_idx = g.IdToPickerIdx.GetInt(g.CurrentPicker, -1);

  IM_ASSERT(picker_idx != -1);

  return g.Pickers[picker_idx];
}

void AddColor() {}

void RemoveColor() {}

void SortMarkers()
{
  ImGradientPicker& picker = GetCurrentPicker();

  float position = picker.Markers[picker.SelectedIdx].Position;

  std::sort(
      picker.Markers.begin(),
      picker.Markers.end(),
      [](const ImGradientMarker& a, const ImGradientMarker& b) { return a.Position < b.Position; });

  for (int i = 0; i < picker.Markers.size(); i++)
  {
    if (picker.Markers[i].Position == position)
    {
      picker.SelectedIdx = i;
      break;
    }
  }
}

static void Checkerboard(ImDrawList* draw_list, ImRect rect)
{
  float tile_size = 10.0f;
  ImU32 col1 = IM_COL32(200, 200, 200, 255);
  ImU32 col2 = IM_COL32(255, 255, 255, 255);
  for (float y = rect.Min.y; y < rect.Max.y; y += tile_size)
  {
    for (float x = rect.Min.x; x < rect.Max.x; x += tile_size)
    {
      bool even = (int((x - rect.Min.x) / tile_size) + int((y - rect.Min.y) / tile_size)) % 2 == 0;
      draw_list->AddRectFilled(
          ImVec2(x, y),
          ImVec2(fminf(x + tile_size, rect.Max.x), fminf(y + tile_size, rect.Max.y)),
          even ? col1 : col2);
    }
  }
}

static void Gradient(ImDrawList* draw_list, ImRect rect)
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();

  ImGradientPicker& picker = GetCurrentPicker();

  bool alpha = (g.PickerFlags & ImGradientPickerFlags_NoAlpha) == 0;

  float prev_position = 0.0f;
  ImU32 prev_color = IM_COL32(
      picker.Markers[0].Color.x * 255.0f,
      picker.Markers[0].Color.y * 255.0f,
      picker.Markers[0].Color.z * 255.0f,
      picker.Markers[0].Color.w * 255.0f);

  for (int i = 0; i < picker.Markers.size(); i++)
  {
    ImGradientMarker& marker = picker.Markers[i];

    float position = marker.Position;
    ImU32 color = IM_COL32(
        marker.Color.x * 255.0f,
        marker.Color.y * 255.0f,
        marker.Color.z * 255.0f,
        marker.Color.w * 255.0f);

    if (position != 0.0f)
    {
      draw_list->AddRectFilledMultiColor(
          ImVec2(rect.Min.x + (rect.Max.x - rect.Min.x) * prev_position, rect.Min.y),
          ImVec2(rect.Min.x + (rect.Max.x - rect.Min.x) * position, rect.Max.y),
          prev_color,
          color,
          color,
          prev_color);
    }

    if (i == picker.Markers.size() - 1 && position != 1.0f)
    {
      draw_list->AddRectFilled(
          ImVec2(rect.Min.x + (rect.Max.x - rect.Min.x) * position, rect.Min.y),
          ImVec2(rect.Min.x + (rect.Max.x - rect.Min.x) * 1.0f, rect.Max.y),
          color);
    }

    prev_position = position;
    prev_color = color;
  }
}

static bool Marker(ImDrawList* draw_list, ImRect rect, ImVec2 marker_size, float position)
{
  float gradient_height = rect.Max.y - rect.Min.y;
  float offset_y = rect.Min.y + gradient_height * 0.75f;

  float thickness = gradient_height * 0.1f;

  float tri_height = gradient_height * 0.6f;
  float tri_width = gradient_height * 0.6f;

  float center_x = rect.Min.x + (rect.Max.x - rect.Min.x) * position;

  ImVec2 tri_p1(center_x, offset_y);
  ImVec2 tri_p2(center_x - tri_width * 0.5f, offset_y + gradient_height * 0.35f);
  ImVec2 tri_p3(center_x + tri_width * 0.5f, offset_y + gradient_height * 0.35f);

  draw_list->AddTriangleFilled(tri_p1, tri_p2, tri_p3, IM_COL32(255, 255, 255, 255));
  draw_list->AddTriangle(tri_p1, tri_p2, tri_p3, IM_COL32(100, 100, 100, 255), thickness);

  ImVec2 mouse_pos = ImGui::GetIO().MousePos;
  ImRect tri_rect = ImRect(
      ImVec2(tri_p2.x - thickness, tri_p1.y - thickness),
      ImVec2(tri_p3.x + thickness, tri_p2.y + thickness));

  ImGui::ButtonBehavior(tri_rect, ImGui::GetID(0), NULL, NULL);

  return mouse_pos.x >= tri_rect.Min.x && mouse_pos.x <= tri_rect.Max.x &&
         mouse_pos.y >= tri_rect.Min.y && mouse_pos.y <= tri_rect.Max.y &&
         ImGui::IsMouseDown(ImGuiMouseButton_Left);
}
} // namespace ImGradient
