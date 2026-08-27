/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationViewRegistry.h"

#include <QUuid>

namespace tb::ui
{
namespace
{

QString randomToken()
{
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString viewTypeToken(const MapViewType type)
{
  switch (type)
  {
  case MapViewType::ThreeD:
    return "3d";
  case MapViewType::XY:
    return "xy";
  case MapViewType::XZ:
    return "xz";
  case MapViewType::YZ:
    return "yz";
  }
  return "unknown";
}

} // namespace

AutomationViewRegistry::AutomationViewRegistry(QObject* parent)
  : AutomationViewRegistry{randomToken, parent}
{
}

AutomationViewRegistry::AutomationViewRegistry(
  TokenGenerator tokenGenerator, QObject* parent)
  : QObject{parent}
  , m_tokenGenerator{std::move(tokenGenerator)}
{
}

QString AutomationViewRegistry::registerView(const QString& documentId, MapViewBase& view)
{
  if (documentId.isEmpty())
  {
    return {};
  }
  if (const auto it = m_idsByView.constFind(&view); it != m_idsByView.cend())
  {
    const auto existing = m_viewsById.constFind(*it);
    return existing != m_viewsById.cend() && existing->documentId == documentId
             ? *it
             : QString{};
  }

  const auto id = mintId(view.viewType());
  m_viewsById.insert(
    id, AutomationViewDescriptor{id, documentId, &view, view.viewType()});
  m_idsByView.insert(&view, id);
  connect(&view, &QObject::destroyed, this, [this, id] { unregisterView(id); });
  return id;
}

bool AutomationViewRegistry::unregisterView(const QString& viewId)
{
  if (!m_viewsById.remove(viewId))
  {
    return false;
  }

  for (auto it = m_idsByView.begin(); it != m_idsByView.end();)
  {
    if (it.value() == viewId)
    {
      it = m_idsByView.erase(it);
    }
    else
    {
      ++it;
    }
  }
  return true;
}

QString AutomationViewRegistry::viewId(const MapViewBase& view) const
{
  return m_idsByView.value(const_cast<MapViewBase*>(&view));
}

MapViewBase* AutomationViewRegistry::findView(
  const QString& documentId, const QString& viewId) const
{
  const auto it = m_viewsById.constFind(viewId);
  if (it == m_viewsById.cend() || it->documentId != documentId)
  {
    return nullptr;
  }
  return it->view.data();
}

std::vector<AutomationViewDescriptor> AutomationViewRegistry::views(
  const QString& documentId) const
{
  auto result = std::vector<AutomationViewDescriptor>{};
  for (const auto& view : m_viewsById)
  {
    if (view.documentId == documentId && view.view)
    {
      result.push_back(view);
    }
  }
  return result;
}

QString AutomationViewRegistry::mintId(const MapViewType type)
{
  auto id = QString{};
  const auto prefix = "view-" + viewTypeToken(type) + "-";
  do
  {
    id = prefix + m_tokenGenerator();
  } while (id == prefix || m_issuedIds.contains(id));
  m_issuedIds.insert(id);
  return id;
}

} // namespace tb::ui
