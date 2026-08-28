/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceVirtualCaptureAdapter.h"

#include <QCryptographicHash>
#include <QFile>

#include "fs/DiskIO.h"
#include "gl/GlManager.h"
#include "mdl/GameInfo.h"
#include "mdl/GameManager.h"
#include "mdl/Map.h"
#include "mdl/MapHeader.h"
#include "ui/AppController.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/AutomationVirtualRenderService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/QPathUtils.h"

#include "kd/result.h"

#include "vm/util.h"

#include <iomanip>
#include <optional>
#include <sstream>
#include <system_error>
#include <utility>

namespace tb::ui
{
namespace
{

constexpr auto RendererVersion = "automation-offscreen-v1";

AcceptanceVirtualCaptureError error(const std::string& message)
{
  return {message};
}

std::filesystem::path normalizedPath(const std::filesystem::path& path)
{
  auto errorCode = std::error_code{};
  const auto canonical = std::filesystem::weakly_canonical(path, errorCode);
  if (!errorCode)
    return canonical;
  errorCode.clear();
  const auto absolute = std::filesystem::absolute(path, errorCode);
  return (errorCode ? path : absolute).lexically_normal();
}

std::optional<std::string> fileFingerprint(const std::filesystem::path& path)
{
  auto file = QFile{pathAsQString(path)};
  if (!file.open(QIODevice::ReadOnly))
    return std::nullopt;
  auto hash = QCryptographicHash{QCryptographicHash::Sha256};
  while (!file.atEnd())
  {
    const auto chunk = file.read(64 * 1024);
    if (chunk.isEmpty() && file.error() != QFile::NoError)
      return std::nullopt;
    hash.addData(chunk);
  }
  return hash.result().toHex().toStdString();
}

std::string hiddenDocumentId(
  const std::filesystem::path& path, const std::string_view fingerprint)
{
  auto hash = uint64_t{14695981039346656037ull};
  const auto input = path.generic_string() + ":" + std::string{fingerprint};
  for (const auto character : input)
  {
    hash ^= static_cast<unsigned char>(character);
    hash *= 1099511628211ull;
  }
  auto stream = std::ostringstream{};
  stream << "hidden-" << std::hex << hash;
  return stream.str();
}

std::optional<automation::AutomationRenderRequest> renderRequest(
  const AcceptanceVirtualCaptureRequest& request)
{
  if (request.renderMode != "textured")
  {
    return std::nullopt;
  }
  if (
    vm::is_zero(request.camera.direction, vm::Cd::almost_zero())
    || vm::is_zero(request.camera.up, vm::Cd::almost_zero()))
  {
    return std::nullopt;
  }

  const auto direction = vm::normalize(request.camera.direction);
  const auto projectedUp =
    request.camera.up - vm::dot(request.camera.up, direction) * direction;
  if (vm::is_zero(projectedUp, vm::Cd::almost_zero()))
  {
    return std::nullopt;
  }
  const auto projection = request.camera.projection == AcceptanceProjection::Perspective
                            ? automation::AutomationProjection::Perspective
                            : automation::AutomationProjection::Orthographic;
  const auto camera = automation::AutomationCamera{
    projection,
    request.camera.position,
    direction,
    vm::normalize(projectedUp),
    projection == automation::AutomationProjection::Perspective
      ? request.camera.verticalFov
      : std::nullopt,
    projection == automation::AutomationProjection::Orthographic ? request.camera.zoom
                                                                 : std::nullopt,
    request.camera.nearPlane,
    request.camera.farPlane,
  };
  return automation::AutomationRenderRequest{
    camera,
    {request.size.width, request.size.height},
    automation::AutomationRenderMode::Textured,
    {request.overlays.brushEdges, request.overlays.selection, request.overlays.grid},
    {request.depth},
    std::nullopt};
}

} // namespace

AcceptanceVirtualCaptureAdapter::AcceptanceVirtualCaptureAdapter(
  AppController& appController, const AutomationDocumentRegistry& documentRegistry)
  : m_appController{appController}
  , m_documentRegistry{documentRegistry}
{
}

MapDocument* AcceptanceVirtualCaptureAdapter::findDocument(
  const std::string_view documentId) const
{
  for (const auto& descriptor : m_documentRegistry.documents())
  {
    if (descriptor.window != nullptr && descriptor.id.toStdString() == documentId)
    {
      return &descriptor.window->document();
    }
  }
  for (const auto& entry : m_hiddenDocuments)
  {
    const auto& hidden = entry.second;
    if (hidden.id == documentId)
    {
      return hidden.document.get();
    }
  }
  return nullptr;
}

Result<AcceptanceVirtualCaptureDocument, AcceptanceVirtualCaptureError>
AcceptanceVirtualCaptureAdapter::documentFor(const std::filesystem::path& requestedPath)
{
  const auto path = normalizedPath(requestedPath);
  auto* document = static_cast<MapDocument*>(nullptr);
  auto documentId = std::string{};
  auto keepAlive = std::shared_ptr<MapDocument>{};
  for (const auto& descriptor : m_documentRegistry.documents())
  {
    if (
      descriptor.window != nullptr && !descriptor.window->document().map().path().empty()
      && normalizedPath(descriptor.window->document().map().path()) == path)
    {
      if (document != nullptr)
      {
        return error("Multiple registered live documents match the acceptance map path");
      }
      document = &descriptor.window->document();
      documentId = descriptor.id.toStdString();
    }
  }

  if (document == nullptr)
  {
    const auto fingerprint = fileFingerprint(path);
    if (!fingerprint)
      return error("Could not fingerprint the acceptance map");
    auto hidden = m_hiddenDocuments.find(path);
    if (hidden != m_hiddenDocuments.end() && hidden->second.fingerprint != *fingerprint)
    {
      m_hiddenDocuments.erase(hidden);
      hidden = m_hiddenDocuments.end();
    }
    if (hidden == m_hiddenDocuments.end())
    {
      const auto header = fs::Disk::withInputStream(path, mdl::readMapHeader);
      if (header.is_error())
      {
        return error("Could not read the acceptance map header");
      }
      const auto& [gameName, mapFormat] = header.value();
      if (!gameName || mapFormat == mdl::MapFormat::Unknown)
      {
        return error(
          "Acceptance map header does not declare a known game and map format");
      }
      const auto* gameInfo = m_appController.gameManager().gameInfo(*gameName);
      if (gameInfo == nullptr)
      {
        return error("Acceptance map references an unavailable game configuration");
      }
      auto loaded = MapDocument::loadDocument(
        m_appController.environmentConfig(),
        *gameInfo,
        mapFormat,
        MapDocument::DefaultWorldBounds,
        path,
        m_appController.taskManager(),
        m_appController.glManager().resourceManager());
      if (loaded.is_error())
      {
        return error("Could not load the acceptance map without opening a window");
      }
      hidden = m_hiddenDocuments
                 .emplace(
                   path,
                   HiddenDocument{
                     std::move(loaded).value(),
                     hiddenDocumentId(path, *fingerprint),
                     *fingerprint})
                 .first;
    }
    document = hidden->second.document.get();
    documentId = hidden->second.id;
    keepAlive = hidden->second.document;
  }

  return AcceptanceVirtualCaptureDocument{
    document, std::move(documentId), std::move(keepAlive)};
}

Result<AcceptanceSolidSpaceDocument, AcceptanceSolidSpaceError>
AcceptanceVirtualCaptureAdapter::queryFor(const std::filesystem::path& path)
{
  const auto resolved = documentFor(path);
  if (resolved.is_error())
  {
    return AcceptanceSolidSpaceError{
      std::get<AcceptanceVirtualCaptureError>(resolved.error()).message};
  }
  return AcceptanceSolidSpaceDocument{
    std::make_shared<AcceptanceMapSolidSpaceQuery>(
      resolved.value().document->map(), resolved.value().keepAlive),
    resolved.value().documentId,
    resolved.value().document->map().modificationCount()};
}

Result<AcceptanceVirtualCaptureResult, AcceptanceVirtualCaptureError>
AcceptanceVirtualCaptureAdapter::capture(const AcceptanceVirtualCaptureRequest& request)
{
  const auto convertedRequest = renderRequest(request);
  if (!convertedRequest)
  {
    return error("Acceptance capture request is not a valid textured virtual render");
  }

  auto resolved = documentFor(request.documentPath);
  if (resolved.is_error())
    return std::get<AcceptanceVirtualCaptureError>(resolved.error());
  const auto resolvedDocument = std::move(resolved).value();

  auto renderer = AutomationVirtualRenderService{m_appController};
  const auto captured = renderer.capture(*resolvedDocument.document, *convertedRequest);
  if (!captured)
  {
    return error(captured.message.toStdString());
  }
  if (request.depth && !captured.output.depth)
  {
    return error("Virtual render did not return requested EV6 depth output");
  }

  return AcceptanceVirtualCaptureResult{
    {request.documentPath, resolvedDocument.documentId, captured.revision},
    AcceptanceCamera{
      request.camera.projection,
      captured.request.camera.position,
      captured.request.camera.direction,
      captured.request.camera.up,
      captured.request.camera.verticalFov,
      captured.request.camera.nearPlane,
      captured.request.camera.farPlane,
      captured.request.camera.zoom},
    {captured.output.size.width, captured.output.size.height},
    captured.output.imagePath,
    captured.output.depth ? std::optional{captured.output.depth->path} : std::nullopt,
    RendererVersion};
}

} // namespace tb::ui
