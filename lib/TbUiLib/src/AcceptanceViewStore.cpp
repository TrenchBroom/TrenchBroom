/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceViewStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLockFile>
#include <QSaveFile>

#include "ui/QPathUtils.h"

#include <utility>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceError error(const AcceptanceErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

template <typename Value>
AcceptanceError resultError(const Result<Value, AcceptanceError>& result)
{
  return std::get<AcceptanceError>(result.error());
}

AcceptanceProjectResult readProject(const std::filesystem::path& path)
{
  const auto filePath = pathAsQString(path);
  auto file = QFile{filePath};
  if (!file.exists())
  {
    return AcceptanceProject{};
  }
  if (!file.open(QIODevice::ReadOnly))
  {
    return error(AcceptanceErrorCode::FileAccess, "Could not read acceptance project");
  }
  const auto contents = file.readAll();
  auto parseError = QJsonParseError{};
  const auto json = QJsonDocument::fromJson(contents, &parseError);
  if (parseError.error != QJsonParseError::NoError || !json.isObject())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, "Acceptance project is not a JSON object");
  }
  return acceptanceProjectFromJson(json.object());
}

AcceptanceValidationResult writeProject(
  const std::filesystem::path& path, const AcceptanceProject& project)
{
  const auto validation = validateAcceptanceProject(project);
  if (validation.is_error())
    return resultError(validation);
  const auto filePath = pathAsQString(path);
  if (!QDir{}.mkpath(QFileInfo{filePath}.dir().path()))
  {
    return error(
      AcceptanceErrorCode::FileAccess, "Could not create acceptance project directory");
  }
  const auto json =
    QJsonDocument{acceptanceProjectToJson(project)}.toJson(QJsonDocument::Indented);
  auto file = QSaveFile{filePath};
  if (
    !file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit())
  {
    return error(
      AcceptanceErrorCode::FileAccess, "Could not atomically write acceptance project");
  }
  return {};
}

template <typename Function>
AcceptanceProjectResult withProjectLock(
  const std::filesystem::path& path, Function function)
{
  const auto filePath = pathAsQString(path);
  if (!QDir{}.mkpath(QFileInfo{filePath}.dir().path()))
  {
    return error(
      AcceptanceErrorCode::FileAccess, "Could not create acceptance project directory");
  }
  auto lock = QLockFile{filePath + ".lock"};
  lock.setStaleLockTime(0);
  if (!lock.tryLock(1000))
  {
    return error(AcceptanceErrorCode::LockFailed, "Could not lock acceptance project");
  }
  const auto result = function();
  lock.unlock();
  return result;
}

} // namespace

AcceptanceViewStore::AcceptanceViewStore(std::filesystem::path projectPath)
  : m_projectPath{std::move(projectPath)}
{
}

const std::filesystem::path& AcceptanceViewStore::projectPath() const
{
  return m_projectPath;
}

AcceptanceProjectResult AcceptanceViewStore::load() const
{
  std::scoped_lock lock{m_mutex};
  return withProjectLock(m_projectPath, [&] { return readProject(m_projectPath); });
}

AcceptanceProjectResult AcceptanceViewStore::replace(
  AcceptanceProject newProject, const size_t expectedRevision)
{
  return update(
    expectedRevision,
    [project = std::move(newProject)](AcceptanceProject& current) mutable {
      current = std::move(project);
      return AcceptanceValidationResult{};
    });
}

AcceptanceProjectResult AcceptanceViewStore::update(
  const size_t expectedRevision, const Update& update)
{
  if (!update)
  {
    return error(
      AcceptanceErrorCode::InvalidValue, "Acceptance store update is required");
  }
  std::scoped_lock lock{m_mutex};
  return withProjectLock(m_projectPath, [&]() -> AcceptanceProjectResult {
    auto currentResult = readProject(m_projectPath);
    if (currentResult.is_error())
      return resultError(currentResult);
    auto current = currentResult.value();
    if (current.revision != expectedRevision)
    {
      return error(
        AcceptanceErrorCode::RevisionConflict, "Acceptance store revision conflict");
    }
    const auto updateResult = update(current);
    if (updateResult.is_error())
      return resultError(updateResult);
    current.revision = expectedRevision + 1u;
    const auto writeResult = writeProject(m_projectPath, current);
    if (writeResult.is_error())
      return resultError(writeResult);
    return current;
  });
}

} // namespace tb::ui
