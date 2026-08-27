/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceView.h"

#include <filesystem>
#include <functional>
#include <mutex>

namespace tb::ui
{

/**
 * A project-configured JSON store. Updates take an exclusive lock, validate the
 * whole project, increment its store revision, and commit through QSaveFile.
 */
class AcceptanceViewStore
{
public:
  using Update = std::function<AcceptanceValidationResult(AcceptanceProject&)>;

  explicit AcceptanceViewStore(std::filesystem::path projectPath);

  const std::filesystem::path& projectPath() const;
  AcceptanceProjectResult load() const;
  AcceptanceProjectResult replace(AcceptanceProject project, size_t expectedRevision);
  AcceptanceProjectResult update(size_t expectedRevision, const Update& update);

private:
  std::filesystem::path m_projectPath;
  mutable std::mutex m_mutex;
};

} // namespace tb::ui
