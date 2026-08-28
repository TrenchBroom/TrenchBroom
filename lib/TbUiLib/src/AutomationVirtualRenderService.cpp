/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationVirtualRenderService.h"

#include <QDataStream>
#include <QDir>
#include <QImage>
#include <QSaveFile>
#include <QUuid>

#include "mdl/Map.h"
#include "ui/AppController.h"
#include "ui/AutomationOffscreenRenderer.h"
#include "ui/MapDocument.h"
#include "ui/QPathUtils.h"
#include "ui/SystemPaths.h"

#include <utility>

namespace tb::ui
{
namespace
{

const auto OutputDirectoryName = std::filesystem::path{"TrenchBroomAutomation"};

AutomationVirtualRenderResult failure(
  const AutomationVirtualRenderError error,
  const QString& message,
  const automation::AutomationRenderRequest& request,
  const size_t revision)
{
  return {error, message, request, {}, revision};
}

AutomationVirtualRenderError contextError(const AutomationOffscreenContextError error)
{
  switch (error)
  {
  case AutomationOffscreenContextError::ResourceNotReady:
    return AutomationVirtualRenderError::ResourceNotReady;
  case AutomationOffscreenContextError::None:
  case AutomationOffscreenContextError::WrongThread:
  case AutomationOffscreenContextError::Busy:
  case AutomationOffscreenContextError::ContextUnavailable:
  case AutomationOffscreenContextError::InitializationFailed:
  case AutomationOffscreenContextError::CallbackFailed:
    return AutomationVirtualRenderError::ContextUnavailable;
  }
  return AutomationVirtualRenderError::ContextUnavailable;
}

AutomationVirtualRenderError renderError(const AutomationOffscreenRenderError error)
{
  switch (error)
  {
  case AutomationOffscreenRenderError::None:
    return AutomationVirtualRenderError::None;
  case AutomationOffscreenRenderError::InvalidRequest:
    return AutomationVirtualRenderError::InvalidRequest;
  case AutomationOffscreenRenderError::ResourceNotReady:
    return AutomationVirtualRenderError::ResourceNotReady;
  case AutomationOffscreenRenderError::DocumentChanged:
    return AutomationVirtualRenderError::DocumentChanged;
  case AutomationOffscreenRenderError::ContextUnavailable:
  case AutomationOffscreenRenderError::FramebufferUnavailable:
  case AutomationOffscreenRenderError::RenderFailed:
    return AutomationVirtualRenderError::RenderFailed;
  }
  return AutomationVirtualRenderError::RenderFailed;
}

bool writePngAtomically(
  const QImage& image, const std::filesystem::path& path, QString* error)
{
  auto output = QSaveFile{pathAsQString(path)};
  if (!output.open(QIODevice::WriteOnly))
  {
    if (error != nullptr)
    {
      *error = "Could not create automation capture output";
    }
    return false;
  }
  if (!image.save(&output, "PNG"))
  {
    output.cancelWriting();
    if (error != nullptr)
    {
      *error = "Could not encode automation capture output";
    }
    return false;
  }
  if (!output.commit())
  {
    if (error != nullptr)
    {
      *error = "Could not publish automation capture output";
    }
    return false;
  }
  return true;
}

bool writeDepthAtomically(
  const automation::AutomationDepthImage& image,
  const std::filesystem::path& path,
  QString* error)
{
  auto output = QSaveFile{pathAsQString(path)};
  if (!output.open(QIODevice::WriteOnly))
  {
    if (error != nullptr)
    {
      *error = "Could not create automation depth output";
    }
    return false;
  }
  const auto header = QByteArray{"Pf\n"} + QByteArray::number(image.size.width) + ' '
                      + QByteArray::number(image.size.height) + "\n-1.0\n";
  if (output.write(header) != header.size())
  {
    output.cancelWriting();
    if (error != nullptr)
    {
      *error = "Could not write the automation depth header";
    }
    return false;
  }

  auto stream = QDataStream{&output};
  stream.setByteOrder(QDataStream::LittleEndian);
  stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
  const auto width = image.size.width;
  for (auto y = image.size.height - 1; y >= 0; --y)
  {
    for (auto x = 0; x < width; ++x)
    {
      stream << image.values[static_cast<size_t>(y * width + x)];
    }
  }
  if (stream.status() != QDataStream::Ok || !output.commit())
  {
    if (error != nullptr)
    {
      *error = "Could not publish automation depth output";
    }
    return false;
  }
  return true;
}

} // namespace

AutomationVirtualRenderService::AutomationVirtualRenderService(
  AppController& appController, std::filesystem::path outputDirectory)
  : m_appController{appController}
  , m_outputDirectory{
      outputDirectory.empty() ? SystemPaths::tempDirectory() / OutputDirectoryName
                              : std::move(outputDirectory)}
{
}

AutomationVirtualRenderResult AutomationVirtualRenderService::capture(
  MapDocument& document, const automation::AutomationRenderRequest& request)
{
  const auto revision = document.map().modificationCount();
  auto rendered = AutomationOffscreenRenderResult{};
  const auto contextResult = m_appController.withAutomationOffscreenContext(
    [&](QOpenGLContext& context, QOffscreenSurface& surface, gl::GlManager& glManager) {
      auto renderer = AutomationOffscreenRenderer{context, surface, glManager};
      rendered = renderer.capture(document, request);
    });
  if (!contextResult)
  {
    return failure(
      contextError(contextResult.error), contextResult.message, request, revision);
  }
  if (!rendered)
  {
    return failure(
      renderError(rendered.error), rendered.message, request, rendered.revision);
  }

  if (!QDir{}.mkpath(pathAsQString(m_outputDirectory)))
  {
    return failure(
      AutomationVirtualRenderError::OutputUnavailable,
      "Could not create automation capture directory",
      request,
      rendered.revision);
  }
  const auto outputPath =
    m_outputDirectory
    / ("render-" + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString() + ".png");
  auto writeError = QString{};
  if (!writePngAtomically(rendered.image, outputPath, &writeError))
  {
    return failure(
      AutomationVirtualRenderError::OutputUnavailable,
      writeError,
      request,
      rendered.revision);
  }

  auto depthOutput = std::optional<automation::AutomationDepthOutput>{};
  if (rendered.depth)
  {
    const auto depthPath =
      outputPath.parent_path() / (outputPath.stem().string() + ".pfm");
    if (!writeDepthAtomically(*rendered.depth, depthPath, &writeError))
    {
      return failure(
        AutomationVirtualRenderError::OutputUnavailable,
        writeError,
        request,
        rendered.revision);
    }
    depthOutput = automation::AutomationDepthOutput{depthPath, rendered.depth->size};
  }

  return {
    AutomationVirtualRenderError::None,
    {},
    request,
    {outputPath, request.size, automation::AutomationCaptureMode::Offscreen, depthOutput},
    rendered.revision};
}

} // namespace tb::ui
