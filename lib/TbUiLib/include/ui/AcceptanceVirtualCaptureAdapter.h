/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceComparisonRunner.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace tb::ui
{
class AppController;
class AutomationDocumentRegistry;
class MapDocument;

/**
 * Production AV-to-EV bridge. Exact registered live paths are reused; every other
 * path is loaded into an adapter-owned MapDocument and never a MapWindow.
 */
class AcceptanceVirtualCaptureAdapter : public AcceptanceVirtualCapture
{
private:
  struct HiddenDocument
  {
    std::unique_ptr<MapDocument> document;
    std::string id;
  };

  AppController& m_appController;
  const AutomationDocumentRegistry& m_documentRegistry;
  std::map<std::filesystem::path, HiddenDocument> m_hiddenDocuments;

public:
  AcceptanceVirtualCaptureAdapter(
    AppController& appController, const AutomationDocumentRegistry& documentRegistry);

  Result<AcceptanceVirtualCaptureResult, AcceptanceVirtualCaptureError> capture(
    const AcceptanceVirtualCaptureRequest& request) override;

  /**
   * Returns a non-owning document pointer only for this adapter's registered live or
   * cached hidden capture identities. It never resolves an active window implicitly.
   */
  MapDocument* findDocument(std::string_view documentId) const;
};

} // namespace tb::ui
