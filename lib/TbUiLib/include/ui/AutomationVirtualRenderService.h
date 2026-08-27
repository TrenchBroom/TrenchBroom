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

#include <cstddef>
#include <filesystem>

namespace tb::ui
{
class AppController;
class MapDocument;

enum class AutomationVirtualRenderError
{
  None,
  InvalidRequest,
  ContextUnavailable,
  ResourceNotReady,
  DocumentChanged,
  RenderFailed,
  OutputUnavailable,
};

/** Value result intended to be translated directly by a future render.capture RPC
 * handler. */
struct AutomationVirtualRenderResult
{
  AutomationVirtualRenderError error = AutomationVirtualRenderError::None;
  QString message;
  automation::AutomationRenderRequest request;
  automation::AutomationRenderOutput output;
  size_t revision = 0u;

  explicit operator bool() const { return error == AutomationVirtualRenderError::None; }
};

/**
 * Captures an explicitly supplied document and camera without resolving a MapWindow or
 * MapViewBase. The AppController provides the serialized GUI-thread GL context.
 */
class AutomationVirtualRenderService
{
private:
  AppController& m_appController;
  std::filesystem::path m_outputDirectory;

public:
  explicit AutomationVirtualRenderService(
    AppController& appController, std::filesystem::path outputDirectory = {});

  AutomationVirtualRenderResult capture(
    MapDocument& document, const automation::AutomationRenderRequest& request);
};

} // namespace tb::ui
