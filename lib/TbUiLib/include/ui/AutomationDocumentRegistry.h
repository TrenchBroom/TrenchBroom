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

#include <functional>
#include <vector>

namespace tb::ui
{
class MapWindow;

/** A process-lifetime, non-reusable identity for a live map window. */
struct AutomationDocumentDescriptor
{
  QString id;
  QPointer<MapWindow> window;
};

/**
 * Owns automation document identities. The registry deliberately does not infer an
 * identity from a pointer or from a path: a destroyed document ID is permanently stale.
 * MapWindowManager is the intended lifecycle owner and calls register/unregister.
 */
class AutomationDocumentRegistry : public QObject
{
public:
  using TokenGenerator = std::function<QString()>;

private:
  TokenGenerator m_tokenGenerator;
  QHash<QString, AutomationDocumentDescriptor> m_documentsById;
  QHash<MapWindow*, QString> m_idsByWindow;
  QSet<QString> m_issuedIds;

public:
  explicit AutomationDocumentRegistry(QObject* parent = nullptr);
  AutomationDocumentRegistry(TokenGenerator tokenGenerator, QObject* parent = nullptr);

  QString registerDocument(MapWindow& window);
  bool unregisterDocument(const QString& documentId);

  QString documentId(const MapWindow& window) const;
  MapWindow* findWindow(const QString& documentId) const;
  std::vector<AutomationDocumentDescriptor> documents() const;

private:
  QString mintId();
};

} // namespace tb::ui
