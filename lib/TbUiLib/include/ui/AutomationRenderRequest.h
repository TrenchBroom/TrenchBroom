/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "render/SceneLighting.h"

#include "vm/vec.h"

#include <filesystem>
#include <optional>
#include <vector>

class QJsonObject;

namespace tb::ui::automation
{

/** Values accepted by the first virtual-rendering API. */
enum class AutomationProjection
{
  Perspective,
  Orthographic,
};

enum class AutomationRenderMode
{
  Textured,
};

enum class AutomationCaptureMode
{
  Offscreen,
};

struct AutomationImageSize
{
  int width = 0;
  int height = 0;
};

struct AutomationOverlayOptions
{
  bool brushEdges = false;
  bool selection = false;
  bool grid = false;
};

/** Auxiliary structural buffers requested in addition to the textured color image. */
struct AutomationOutputOptions
{
  bool depth = false;
};

struct AutomationScenePreviewOptions
{
  render::PlayerVision vision = render::PlayerVision::Human;
  double timeOfDay = 12.0;
  bool entityLights = true;
};

/**
 * A normalized, widget-independent camera. Perspective cameras use verticalFov;
 * orthographic cameras use zoom. The unused optional is always empty.
 */
struct AutomationCamera
{
  AutomationProjection projection = AutomationProjection::Perspective;
  vm::vec3d position;
  vm::vec3d direction;
  vm::vec3d up;
  std::optional<double> verticalFov;
  std::optional<double> zoom;
  double nearPlane = 0.0;
  double farPlane = 0.0;
};

struct AutomationRenderRequest
{
  AutomationCamera camera;
  AutomationImageSize size;
  AutomationRenderMode renderMode = AutomationRenderMode::Textured;
  AutomationOverlayOptions overlays;
  AutomationOutputOptions outputs = {};
  std::optional<AutomationScenePreviewOptions> scenePreview;
};

/**
 * A top-left-origin depth image matching AutomationImageSize. Values are finite
 * camera-forward distances in map units; +infinity denotes an uncovered pixel.
 */
struct AutomationDepthImage
{
  AutomationImageSize size;
  std::vector<float> values;
};

/** Metadata for a linearized float32 depth buffer. */
struct AutomationDepthOutput
{
  std::filesystem::path path;
  AutomationImageSize size;
};

/** Metadata returned by an image-producing automation operation. */
struct AutomationRenderOutput
{
  std::filesystem::path imagePath;
  AutomationImageSize size;
  AutomationCaptureMode captureMode = AutomationCaptureMode::Offscreen;
  std::optional<AutomationDepthOutput> depth;
};

constexpr int AutomationMinImageDimension = 1;
constexpr int AutomationMaxImageDimension = 8192;

std::optional<AutomationRenderRequest> renderRequestFromJson(const QJsonObject& json);
QJsonObject renderRequestToJson(const AutomationRenderRequest& request);
QJsonObject renderOutputToJson(const AutomationRenderOutput& output);

bool isValidImageSize(const AutomationImageSize& size);

/**
 * The depth file is a grayscale PFM (Pf) file with IEEE-754 binary32 samples and
 * a -1.0 scale (little endian). PFM scanlines are bottom-to-top by format convention;
 * after decoding, values are indexed top-to-bottom like the color image. A finite
 * sample is linear camera-forward distance in map units, while +infinity means no
 * geometry was written to that pixel.
 */
constexpr const char* AutomationDepthFormat = "pfm-f32-linear-camera-space";

} // namespace tb::ui::automation
