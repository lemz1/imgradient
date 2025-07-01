#include "imgradient.h"

#include "imgradient_internal.h"

#include <cmath>

ImGradientContext* GImGradient = NULL;

namespace ImGradient
{
static void Initialize();
static void Shutdown();
static void Checkerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max);
static void Gradient(
    ImDrawList*           draw_list,
    ImVec2                p_min,
    ImVec2                p_max,
    int                   count,
    const float*          colors,
    const float*          positions,
    ImGradientPickerFlags flags);
static void HandlePickerIO();

ImGradientContext* CreateContext()
{
  ImGradientContext* ctx = IM_NEW(ImGradientContext)();
  if (GImGradient == NULL)
    SetCurrentContext(ctx);
  Initialize();
  return ctx;
}

void DestroyContext()
{
  Shutdown();
  IM_DELETE(GImGradient);
}

ImGradientContext* GetCurrentContext() { return GImGradient; }

void SetCurrentContext(ImGradientContext* ctx) { GImGradient = ctx; }

bool GradientPicker(
    int                   count,
    const float*          colors,
    const float*          positions,
    ImGradientPickerFlags flags)
{
  IM_ASSERT(count > 0);

  ImGradientContext& g = *GImGradient;
  ImDrawList*        d = ImGui::GetWindowDrawList();
  ImVec2             c = ImGui::GetCursorScreenPos();

  ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 20);

  ImVec2 p_min = c;
  ImVec2 p_max = ImVec2(c.x + size.x, c.y + size.y);

  if ((flags & ImGradientPickerFlags_NoAlpha) == 0)
  {
    Checkerboard(d, p_min, p_max);
  }

  Gradient(d, p_min, p_max, count, colors, positions, flags);

  return false;
}

void GetRepositionedIndex(int* prev_color_idx, int* new_color_idx)
{
  ImGradientContext& g = *GImGradient;

  *prev_color_idx = g.PrevColorIdx;
  *new_color_idx = g.NewColorIdx;
}

static void Initialize()
{
  ImGradientContext& g = *GImGradient;
  IM_ASSERT(g.Initialized == false);
  g.Initialized = true;
}

static void Shutdown()
{
  ImGradientContext& g = *GImGradient;
  IM_ASSERT(g.Initialized == true);
  g.Initialized = false;
}

void Checkerboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max)
{
  float tile_size = 10.0f;
  ImU32 col1 = IM_COL32(200, 200, 200, 255);
  ImU32 col2 = IM_COL32(255, 255, 255, 255);
  for (float y = p_min.y; y < p_max.y; y += tile_size)
  {
    for (float x = p_min.x; x < p_max.x; x += tile_size)
    {
      bool even = (int((x - p_min.x) / tile_size) + int((y - p_min.y) / tile_size)) % 2 == 0;
      draw_list->AddRectFilled(
          ImVec2(x, y),
          ImVec2(fminf(x + tile_size, p_max.x), fminf(y + tile_size, p_max.y)),
          even ? col1 : col2);
    }
  }
}

void Gradient(
    ImDrawList*           draw_list,
    ImVec2                p_min,
    ImVec2                p_max,
    int                   count,
    const float*          colors,
    const float*          positions,
    ImGradientPickerFlags flags)
{
  bool alpha = (flags & ImGradientPickerFlags_NoAlpha) == 0;
  int  components = alpha ? 4 : 3;

  float prev_position = 0.0f;
  ImU32 prev_color = IM_COL32(
      colors[0] * 255.0f, colors[1] * 255.0f, colors[2] * 255.0f, alpha ? colors[3] * 255.0f : 255);
  for (int i = 0; i < count; i++)
  {
    float position = positions[i];
    ImU32 color = IM_COL32(
        colors[i * components + 0] * 255.0f,
        colors[i * components + 1] * 255.0f,
        colors[i * components + 2] * 255.0f,
        alpha ? colors[i * components + 3] * 255.0f : 255);

    if (position != 0.0f)
    {
      draw_list->AddRectFilledMultiColor(
          ImVec2(p_min.x + (p_max.x - p_min.x) * prev_position, p_min.y),
          ImVec2(p_min.x + (p_max.x - p_min.x) * position, p_max.y),
          prev_color,
          color,
          color,
          prev_color);
    }

    if (i == count - 1 && position != 1.0f)
    {
      draw_list->AddRectFilled(
          ImVec2(p_min.x + (p_max.x - p_min.x) * position, p_min.y),
          ImVec2(p_min.x + (p_max.x - p_min.x) * 1.0f, p_max.y),
          color);
    }

    prev_position = position;
    prev_color = color;
  }
}

static void HandlePickerIO() {}
} // namespace ImGradient
