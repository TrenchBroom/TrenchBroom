/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QString>

#include "ui/AutomationRenderRequest.h"
#include "ui/MapViewContext.h"

#include <cstddef>

namespace tb::ui
{
class MapDocument;

enum class AutomationVirtualPickError
{
  None,
  InvalidRequest,
  InvalidPixel,
  DocumentChanged,
  PickFailed,
};

/** A model-space pick result for one explicit document revision. */
struct AutomationVirtualPickResult
{
  AutomationVirtualPickError error = AutomationVirtualPickError::None;
  QString message;
  automation::AutomationRenderRequest request;
  MapViewPickResult pick;
  size_t revision = 0u;

  explicit operator bool() const { return error == AutomationVirtualPickError::None; }
};

/**
 * Performs a focus-neutral model pick using the exact camera request accepted by
 * virtual rendering. It has no QWidget, MapWindow, or GL-context dependency.
 */
class AutomationVirtualPickService
{
public:
  AutomationVirtualPickResult pick(
    MapDocument& document,
    const automation::AutomationRenderRequest& request,
    double x,
    double y) const;
};

} // namespace tb::ui
