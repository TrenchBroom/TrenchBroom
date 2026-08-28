/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceComparisonContextResolver.h"

#include <algorithm>
#include <ranges>
#include <system_error>
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

Result<std::filesystem::path, AcceptanceError> canonicalize(
  const std::filesystem::path& path)
{
  auto ec = std::error_code{};
  const auto absolutePath = std::filesystem::absolute(path, ec);
  if (ec)
  {
    return error(
      AcceptanceErrorCode::FileAccess,
      "Could not make the acceptance document path absolute");
  }
  const auto canonicalPath = std::filesystem::weakly_canonical(absolutePath, ec);
  if (ec)
  {
    return error(
      AcceptanceErrorCode::FileAccess,
      "Could not canonicalize the acceptance document path");
  }
  return canonicalPath;
}

} // namespace

ResolvedAcceptanceComparisonContextResult resolveAcceptanceComparisonContext(
  const std::filesystem::path& projectPath,
  const AcceptanceProject& project,
  const std::string& contextId)
{
  const auto projectValidation = validateAcceptanceProject(project);
  if (projectValidation.is_error())
    return resultError(projectValidation);

  const auto context =
    std::ranges::find(project.contexts, contextId, &AcceptanceComparisonContext::id);
  if (context == project.contexts.end())
  {
    return error(
      AcceptanceErrorCode::BrokenReference, "Acceptance context was not found");
  }

  const auto referencePath = resolveAcceptancePath(projectPath, context->referencePath);
  const auto candidatePath = resolveAcceptancePath(projectPath, context->candidatePath);
  if (referencePath.is_error())
    return resultError(referencePath);
  if (candidatePath.is_error())
    return resultError(candidatePath);

  const auto canonicalReferencePath = canonicalize(referencePath.value());
  const auto canonicalCandidatePath = canonicalize(candidatePath.value());
  if (canonicalReferencePath.is_error())
    return resultError(canonicalReferencePath);
  if (canonicalCandidatePath.is_error())
    return resultError(canonicalCandidatePath);
  if (canonicalReferencePath.value() == canonicalCandidatePath.value())
  {
    return error(
      AcceptanceErrorCode::InvalidValue,
      "Acceptance context reference and candidate resolve to the same document");
  }

  return ResolvedAcceptanceComparisonContext{
    context->id,
    canonicalReferencePath.value(),
    canonicalCandidatePath.value(),
    context->alignment,
  };
}

} // namespace tb::ui
