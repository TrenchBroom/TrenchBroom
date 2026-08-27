/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUuid>

#include "ui/AutomationService.h"
#include "ui/QPathUtils.h"
#include "ui/SystemPaths.h"

namespace tb::ui
{

bool AutomationService::start()
{
  m_serverName = QString{"trenchbroom-%1-%2"}
                   .arg(QCoreApplication::applicationPid())
                   .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  if (!m_server.listen(m_serverName))
  {
    return false;
  }

  const auto discoveryDirectory = SystemPaths::userDataDirectory() / "automation";
  if (!QDir{}.mkpath(pathAsQString(discoveryDirectory)))
  {
    m_server.close();
    return false;
  }

  m_discoveryPath = discoveryDirectory
                   / (std::to_string(QCoreApplication::applicationPid()) + "-"
                      + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString()
                      + ".json");
  auto file = QSaveFile{pathAsQString(m_discoveryPath)};
  if (!file.open(QIODevice::WriteOnly))
  {
    m_server.close();
    m_discoveryPath.clear();
    return false;
  }

  const auto discovery = QJsonObject{
    {"apiVersion", 1},
    {"pid", QCoreApplication::applicationPid()},
    {"socket", m_serverName},
  };
  file.write(QJsonDocument{discovery}.toJson(QJsonDocument::Indented));
  if (!file.commit())
  {
    m_server.close();
    m_discoveryPath.clear();
    return false;
  }
  QFile::setPermissions(
    pathAsQString(m_discoveryPath), QFileDevice::ReadUser | QFileDevice::WriteUser);
  return true;
}

void AutomationService::removeDiscoveryFile()
{
  if (!m_discoveryPath.empty())
  {
    QFile::remove(pathAsQString(m_discoveryPath));
  }
}

} // namespace tb::ui
