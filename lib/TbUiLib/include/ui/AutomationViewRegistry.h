/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>

#include "ui/MapViewBase.h"

#include <functional>
#include <vector>

namespace tb::ui
{

struct AutomationViewDescriptor
{
  QString id;
  QString documentId;
  QPointer<MapViewBase> view;
  MapViewType type = MapViewType::ThreeD;
};

/**
 * Registry for real, widget-backed map views. An ID belongs to exactly one document
 * identity and is invalidated when its MapViewBase is destroyed or unregistered.
 */
class AutomationViewRegistry : public QObject
{
public:
  using TokenGenerator = std::function<QString()>;

private:
  TokenGenerator m_tokenGenerator;
  QHash<QString, AutomationViewDescriptor> m_viewsById;
  QHash<MapViewBase*, QString> m_idsByView;
  QSet<QString> m_issuedIds;

public:
  explicit AutomationViewRegistry(QObject* parent = nullptr);
  AutomationViewRegistry(TokenGenerator tokenGenerator, QObject* parent = nullptr);

  QString registerView(const QString& documentId, MapViewBase& view);
  bool unregisterView(const QString& viewId);

  QString viewId(const MapViewBase& view) const;
  MapViewBase* findView(const QString& documentId, const QString& viewId) const;
  std::vector<AutomationViewDescriptor> views(const QString& documentId) const;

private:
  QString mintId(MapViewType type);
};

} // namespace tb::ui
