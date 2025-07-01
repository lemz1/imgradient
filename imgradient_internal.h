#pragma once

#include "imgradient.h"

struct ImGradientContext;
struct ImGradientPicker;

struct ImGradientContext
{
  bool         Initialized = false;
  ImGradientIO IO;

  int PrevColorIdx;
  int NewColorIdx;
};
