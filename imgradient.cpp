#include "imgradient.h"

#include <imgui_internal.h>

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
static bool Marker(
    ImDrawList* draw_list,
    ImRect      rect,
    ImVec2      marker_size,
    float       position,
    bool        is_selected);

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

const ImVector<ImGradientMarker>& GradientPicker(const char* label, ImGradientPickerFlags flags)
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();
  ImDrawList*        d = ImGui::GetWindowDrawList();

  g.PickerFlags = flags;

  ImGradientPicker& picker = GetOrCreatePicker(ImGui::GetID(label));

  bool no_alpha = (flags & ImGradientPickerFlags_NoAlpha) != 0;
  bool no_adding = (flags & ImGradientPickerFlags_NoAdding) != 0;
  bool no_removing = picker.Markers.size() == 1 || (flags & ImGradientPickerFlags_NoRemoving) != 0;
  bool no_moving = (flags & ImGradientPickerFlags_NoMoving) != 0;
  bool no_coloring = (flags & ImGradientPickerFlags_NoColoring) != 0;

  float button_size = ImGui::GetFrameHeight();
  if (no_removing)
  {
    ImGui::BeginDisabled();
  }
  if (ImGui::Button("-", ImVec2(button_size, button_size)))
  {
    RemoveColor(picker.SelectedIdx);
  }
  if (no_removing)
  {
    ImGui::EndDisabled();
  }
  ImGui::SameLine();
  if (no_adding)
  {
    ImGui::BeginDisabled();
  }
  if (ImGui::Button("+", ImVec2(button_size, button_size)))
  {
    if (picker.Markers.size() == 1)
    {
      if (picker.Markers[picker.SelectedIdx].Position != 1.0f)
      {
        AddColor(1.0f);
      }
      else
      {
        AddColor(0.0f);
      }
    }
    else
    {
      float position1 = picker.Markers[picker.SelectedIdx].Position;
      float position2 = picker.SelectedIdx == 0 ? picker.Markers[picker.SelectedIdx + 1].Position
                                                : picker.Markers[picker.SelectedIdx - 1].Position;
      AddColor((position1 + position2) * 0.5f);
    }
  }
  if (no_adding)
  {
    ImGui::EndDisabled();
  }
  ImGui::SameLine();
  ImGui::Text(label);

  ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
  cursor_pos.y += 2.0f;
  ImVec2 gradient_size = ImVec2(ImGui::GetContentRegionAvail().x, 20.0f);

  ImVec2 marker_size = ImVec2(gradient_size.y * 0.6f, gradient_size.y * 0.6f);

  ImRect gradient_rect = ImRect(
      ImVec2(cursor_pos.x + marker_size.x * 0.5f, cursor_pos.y),
      ImVec2(
          cursor_pos.x + gradient_size.x - marker_size.x * 0.5f, cursor_pos.y + gradient_size.y));

  Checkerboard(d, gradient_rect);

  Gradient(d, gradient_rect);

  bool dragging_marker = false;
  int  new_selected_idx = -1;
  for (int i = 0; i < picker.Markers.size(); i++)
  {
    if (i == picker.SelectedIdx)
    {
      continue;
    }

    ImGui::PushID(i);
    if (Marker(d, gradient_rect, marker_size, picker.Markers[i].Position, false))
    {
      dragging_marker = true;
      new_selected_idx = i;
    }
    ImGui::PopID();
  }

  ImGui::PushID(picker.SelectedIdx);
  if (Marker(d, gradient_rect, marker_size, picker.Markers[picker.SelectedIdx].Position, true))
  {
    dragging_marker = true;
    new_selected_idx = picker.SelectedIdx;
  }
  ImGui::PopID();

  if (!g.IsDraggingMarker && new_selected_idx != -1)
  {
    if (!no_removing && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::GetIO().KeyCtrl)
    {
      RemoveColor(new_selected_idx);
    }
    else if (!ImGui::GetIO().KeyCtrl)
    {
      picker.SelectedIdx = new_selected_idx;
      g.IsDraggingMarker = dragging_marker && !no_moving;
    }
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

  ImGui::ItemSize(ImVec2(gradient_size.x, gradient_size.y + marker_size.y * 0.5f));
  ImGui::ItemAdd(gradient_rect, g.CurrentPicker);

  bool hovered_gradient;
  ImGui::ButtonBehavior(gradient_rect, g.CurrentPicker, &hovered_gradient, NULL);

  if (!no_adding && hovered_gradient && !g.IsDraggingMarker &&
      ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::GetIO().KeyShift)
  {
    float position = (ImGui::GetIO().MousePos.x - gradient_rect.Min.x) /
                     (gradient_rect.Max.x - gradient_rect.Min.x);
    AddColor(position);
  }

  if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
  {
    g.IsDraggingMarker = false;
  }

  ImGui::SetNextItemWidth(75.0f);
  if (ImGui::InputInt("##Marker Index", &picker.SelectedIdx, 1, 1))
  {
    picker.SelectedIdx =
        (picker.SelectedIdx < 0
             ? 0
             : (picker.SelectedIdx >= picker.Markers.size() ? picker.Markers.size() - 1
                                                            : picker.SelectedIdx));
  }

  ImGui::SameLine();

  if (no_moving)
  {
    ImGui::BeginDisabled();
  }
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  if (ImGui::DragFloat(
          "##Marker Position", &picker.Markers[picker.SelectedIdx].Position, 0.001f, 0.0f, 1.0f))
  {
    if ((picker.SelectedIdx - 1 >= 0 && picker.Markers[picker.SelectedIdx - 1].Position >
                                            picker.Markers[picker.SelectedIdx].Position) ||
        (picker.SelectedIdx + 1 < picker.Markers.size() &&
         picker.Markers[picker.SelectedIdx + 1].Position <
             picker.Markers[picker.SelectedIdx].Position))
    {
      SortMarkers();
    }
  }
  if (no_moving)
  {
    ImGui::EndDisabled();
  }

  if (no_coloring)
  {
    ImGui::BeginDisabled();
  }
  if (ImGui::ColorButton(
          "##Marker Color",
          picker.Markers[picker.SelectedIdx].Color,
          0,
          ImVec2(ImGui::GetContentRegionAvail().x, 20)))
  {
    ImGui::OpenPopup("Marker Popup");
    ImVec2 bl = ImGui::GetCurrentContext()->LastItemData.Rect.GetBL();
    ImGui::SetNextWindowPos(ImVec2(bl.x, bl.y + ImGui::GetStyle().ItemSpacing.y));
  }
  if (no_coloring)
  {
    ImGui::EndDisabled();
  }

  if (ImGui::BeginPopup("Marker Popup"))
  {
    ImGui::SetNextItemWidth(200.0f);
    ImGui::ColorPicker4(
        "##Color Picker",
        (float*)&picker.Markers[picker.SelectedIdx].Color,
        no_alpha ? ImGuiColorEditFlags_NoAlpha : ImGuiColorEditFlags_AlphaBar);
    ImGui::EndPopup();
  }

  g.NextMarkers.resize(0);

  return picker.Markers;
}

void AddInitialMarker(ImVec4 color, float position)
{
  ImGradientContext& g = *ImGradient::GetCurrentContext();

  g.NextMarkers.push_back(ImGradientMarker{color, position});
}

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
      for (int i = 0; i < g.NextMarkers.size(); i++)
      {
        if ((g.PickerFlags & ImGradientPickerFlags_NoAlpha) != 0)
        {
          IM_ASSERT_USER_ERROR(
              g.NextMarkers[i].Color.w == 1.0f, "Alpha needs to be 1.0f when disabling alpha");
        }

        if (i > 0)
        {
          IM_ASSERT_USER_ERROR(
              g.NextMarkers[i - 1].Position < g.NextMarkers[i].Position,
              "The initial markers have to be in ascending order");
        }
      }

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

void AddColor(float position)
{
  ImGradientPicker& picker = GetCurrentPicker();

  if (picker.Markers.size() == 1)
  {
    picker.Markers.push_back(ImGradientMarker{picker.Markers[0].Color, position});
    picker.SelectedIdx = picker.Markers.size() - 1;
    SortMarkers();
    return;
  }

  ImVec4 color1;
  float  position1;
  ImVec4 color2;
  float  position2;
  int    insert_idx = -1;
  for (int i = 0; i < picker.Markers.size(); i++)
  {
    if (i == 0 && picker.Markers[i].Position > position)
    {
      picker.Markers.insert(
          picker.Markers.begin(), ImGradientMarker{picker.Markers[i].Color, position});
      picker.SelectedIdx = 0;
      return;
    }
    else if (i == picker.Markers.size() - 1 && picker.Markers[i].Position < position)
    {
      picker.Markers.push_back(ImGradientMarker{picker.Markers[i].Color, position});
      picker.SelectedIdx = picker.Markers.size() - 1;
      return;
    }

    if (picker.Markers[i].Position > position)
    {
      color1 = picker.Markers[i - 1].Color;
      position1 = picker.Markers[i - 1].Position;
      color2 = picker.Markers[i].Color;
      position2 = picker.Markers[i].Position;
      insert_idx = i;
      break;
    }
  }

  IM_ASSERT(insert_idx != -1);

  float  color_t = (position - position1) / (position2 - position1);
  ImVec4 color = ImVec4(
      color1.x + (color2.x - color1.x) * color_t,
      color1.y + (color2.y - color1.y) * color_t,
      color1.z + (color2.z - color1.z) * color_t,
      color1.w + (color2.w - color1.w) * color_t);

  picker.Markers.insert(picker.Markers.begin() + insert_idx, ImGradientMarker{color, position});
  picker.SelectedIdx = insert_idx;
}

void RemoveColor(int idx)
{
  ImGradientPicker& picker = GetCurrentPicker();

  IM_ASSERT_USER_ERROR(picker.Markers.size() > 1, "Atleast one marker needs to exist");

  picker.Markers.erase(picker.Markers.begin() + idx);

  if (picker.SelectedIdx == picker.Markers.size())
  {
    picker.SelectedIdx -= 1;
  }
}

void SortMarkers()
{
  ImGradientPicker& picker = GetCurrentPicker();

  ImGradientMarker moved = picker.Markers[picker.SelectedIdx];
  picker.Markers.erase(picker.Markers.begin() + picker.SelectedIdx);

  auto it = std::lower_bound(
      picker.Markers.begin(),
      picker.Markers.end(),
      moved.Position,
      [](const ImGradientMarker& a, float value) { return a.Position < value; });

  int new_idx = (int)(it - picker.Markers.begin());

  picker.Markers.insert(it, moved);
  picker.SelectedIdx = new_idx;
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

static bool Marker(
    ImDrawList* draw_list,
    ImRect      rect,
    ImVec2      marker_size,
    float       position,
    bool        is_selected)
{
  ImGradientPicker& picker = GetCurrentPicker();

  float gradient_height = rect.Max.y - rect.Min.y;
  float offset_y = rect.Min.y + gradient_height * 0.75f;

  float thickness = gradient_height * 0.1f;

  float tri_height = gradient_height * 0.6f;
  float tri_width = gradient_height * 0.6f;

  float center_x = rect.Min.x + (rect.Max.x - rect.Min.x) * position;

  ImVec2 tri_p1(center_x, offset_y);
  ImVec2 tri_p2(center_x - tri_width * 0.5f, offset_y + gradient_height * 0.35f);
  ImVec2 tri_p3(center_x + tri_width * 0.5f, offset_y + gradient_height * 0.35f);

  if (is_selected)
  {
    const ImVec4& color = picker.Markers[picker.SelectedIdx].Color;
    float         brightness = (0.299f * color.x + 0.587f * color.y + 0.114f * color.z);

    ImU32 line_color = brightness > 0.5f ? IM_COL32_BLACK : IM_COL32_WHITE;
    float line_offset = gradient_height * 0.15f;

    draw_list->AddLine(
        ImVec2(tri_p1.x, tri_p1.y),
        ImVec2(center_x, tri_p1.y - line_offset),
        line_color,
        thickness);

    draw_list->AddLine(
        ImVec2(tri_p1.x, tri_p1.y - line_offset * 2.0f),
        ImVec2(center_x, tri_p1.y - line_offset * 3.0f),
        line_color,
        thickness);

    draw_list->AddLine(
        ImVec2(tri_p1.x, tri_p1.y - line_offset * 4.0f),
        ImVec2(center_x, tri_p1.y - line_offset * 5.0f),
        line_color,
        thickness);
  }

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
