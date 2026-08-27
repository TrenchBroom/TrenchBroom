/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationDocumentRegistry.h"

#include <QUuid>

#include "ui/MapWindow.h"

namespace tb::ui
{
namespace
{

QString randomToken()
{
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace

AutomationDocumentRegistry::AutomationDocumentRegistry(QObject* parent)
  : AutomationDocumentRegistry{randomToken, parent}
{
}

AutomationDocumentRegistry::AutomationDocumentRegistry(
  TokenGenerator tokenGenerator, QObject* parent)
  : QObject{parent}
  , m_tokenGenerator{std::move(tokenGenerator)}
{
}

QString AutomationDocumentRegistry::registerDocument(MapWindow& window)
{
  if (const auto it = m_idsByWindow.constFind(&window); it != m_idsByWindow.cend())
  {
    return *it;
  }

  const auto id = mintId();
  m_documentsById.insert(id, AutomationDocumentDescriptor{id, &window});
  m_idsByWindow.insert(&window, id);
  connect(&window, &QObject::destroyed, this, [this, id] { unregisterDocument(id); });
  return id;
}

bool AutomationDocumentRegistry::unregisterDocument(const QString& documentId)
{
  if (!m_documentsById.remove(documentId))
  {
    return false;
  }

  for (auto it = m_idsByWindow.begin(); it != m_idsByWindow.end();)
  {
    if (it.value() == documentId)
    {
      it = m_idsByWindow.erase(it);
    }
    else
    {
      ++it;
    }
  }
  return true;
}

QString AutomationDocumentRegistry::documentId(const MapWindow& window) const
{
  return m_idsByWindow.value(const_cast<MapWindow*>(&window));
}

MapWindow* AutomationDocumentRegistry::findWindow(const QString& documentId) const
{
  const auto it = m_documentsById.constFind(documentId);
  return it != m_documentsById.cend() ? it->window.data() : nullptr;
}

std::vector<AutomationDocumentDescriptor> AutomationDocumentRegistry::documents() const
{
  auto result = std::vector<AutomationDocumentDescriptor>{};
  result.reserve(static_cast<size_t>(m_documentsById.size()));
  for (const auto& document : m_documentsById)
  {
    if (document.window)
    {
      result.push_back(document);
    }
  }
  return result;
}

QString AutomationDocumentRegistry::mintId()
{
  auto id = QString{};
  do
  {
    id = "document-" + m_tokenGenerator();
  } while (id == "document-" || m_issuedIds.contains(id));
  m_issuedIds.insert(id);
  return id;
}

} // namespace tb::ui
