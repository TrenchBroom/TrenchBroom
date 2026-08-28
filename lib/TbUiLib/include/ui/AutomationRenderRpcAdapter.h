/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QJsonObject>
#include <QString>

namespace tb::ui
{
class AppController;
class AutomationDocumentRegistry;
class JsonRpcResponse;

/** Strict, window-independent adapter for render.capture, render.context, and
 * render.pick. */
class AutomationRenderRpcAdapter
{
private:
  AppController& m_appController;

public:
  explicit AutomationRenderRpcAdapter(AppController& appController);

  JsonRpcResponse handle(
    const QString& method,
    const QJsonObject& params,
    const AutomationDocumentRegistry& documentRegistry) const;
};

} // namespace tb::ui
