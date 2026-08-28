/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceSuiteRunner.h"

#include <filesystem>
#include <string>

namespace tb::ui
{

struct AcceptanceEvidenceError
{
  std::string message;
};

class AcceptanceDocumentSnapshotProvider
{
public:
  virtual ~AcceptanceDocumentSnapshotProvider() = default;

  /**
   * Serializes exactly the identified in-memory document revision. Implementations
   * must fail rather than snapshot a different revision.
   */
  virtual Result<void, AcceptanceEvidenceError> snapshot(
    const AcceptanceCaptureDocumentIdentity& document,
    const std::filesystem::path& outputPath) = 0;
};

struct AcceptanceEvidenceBundle
{
  std::filesystem::path path;
  std::filesystem::path manifestPath;
  std::string manifestSha256;
};

using AcceptanceEvidenceResult =
  Result<AcceptanceEvidenceBundle, AcceptanceEvidenceError>;

/**
 * Publishes an immutable, self-contained evidence directory. The output directory
 * must not already exist. All files are first written to a sibling staging directory
 * and then renamed into place.
 */
AcceptanceEvidenceResult writeAcceptanceEvidenceBundle(
  const std::filesystem::path& projectPath,
  const AcceptanceSuiteRunReport& report,
  const std::filesystem::path& outputDirectory,
  AcceptanceDocumentSnapshotProvider& snapshots);

} // namespace tb::ui
