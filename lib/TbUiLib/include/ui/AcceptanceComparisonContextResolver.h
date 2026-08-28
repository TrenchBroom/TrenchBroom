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
#include <string>

namespace tb::ui
{

/**
 * The durable context resolved to concrete filesystem paths. This deliberately
 * contains no editor, view, or process-local document identity.
 */
struct ResolvedAcceptanceComparisonContext
{
  std::string id;
  std::filesystem::path referencePath;
  std::filesystem::path candidatePath;
  AcceptanceAlignment alignment;
};

using ResolvedAcceptanceComparisonContextResult =
  Result<ResolvedAcceptanceComparisonContext, AcceptanceError>;

/**
 * Resolves a named context relative to an explicitly supplied acceptance project
 * path. The returned paths are weakly canonical absolute filesystem paths.
 */
ResolvedAcceptanceComparisonContextResult resolveAcceptanceComparisonContext(
  const std::filesystem::path& projectPath,
  const AcceptanceProject& project,
  const std::string& contextId);

} // namespace tb::ui
