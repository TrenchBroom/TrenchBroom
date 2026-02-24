/*
 Copyright (C) 2025 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/MapDocumentFixture.h"

#include "gl/Resource.h"
#include "gl/ResourceManager.h"
#include "gl/TestUtils.h"
#include "mdl/Map.h"
#include "mdl/TestUtils.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"

namespace tb::ui
{

Result<std::unique_ptr<MapDocument>> createFixtureDocument(
  mdl::MapFixtureConfig& config,
  kdl::task_manager& taskManager,
  gl::ResourceManager& resourceManager)
{
  const auto mapFormat = config.mapFormat.value_or(mdl::MapFormat::Standard);

  return MapDocument::createDocument(
           config.environmentConfig,
           config.gameInfo,
           mapFormat,
           vm::bbox3d{8192.0},
           taskManager,
           resourceManager)
         | kdl::transform([&](auto document) {
             document->map().setIsCommandCollationEnabled(false);
             return document;
           });
}

Result<std::unique_ptr<MapDocument>> loadFixtureDocument(
  const std::filesystem::path& path,
  mdl::MapFixtureConfig& config,
  kdl::task_manager& taskManager,
  gl::ResourceManager& resourceManager)
{
  const auto mapFormat = config.mapFormat.value_or(mdl::MapFormat::Standard);

  return MapDocument::loadDocument(
           config.environmentConfig,
           config.gameInfo,
           mapFormat,
           vm::bbox3d{8192.0},
           path,
           taskManager,
           resourceManager)
         | kdl::transform([&](auto document) {
             document->map().setIsCommandCollationEnabled(false);
             gl::processResourcesSync(
               resourceManager, gl::ProcessContext{false, [](auto, auto) {}});
             return document;
           });
}

MapDocumentFixture::MapDocumentFixture()
  : m_taskManager{createTestTaskManager()}
  , m_resourceManager{std::make_unique<gl::ResourceManager>()}
{
}

MapDocumentFixture::~MapDocumentFixture() = default;

MapDocument& MapDocumentFixture::create(mdl::MapFixtureConfig config)
{
  m_config = std::move(config);

  contract_assert(
    createFixtureDocument(*m_config, *m_taskManager, *m_resourceManager)
    | kdl::transform([&](auto document) { m_document = std::move(document); })
    | kdl::is_success());

  return *m_document;
}

MapDocument& MapDocumentFixture::load(
  const std::filesystem::path& path, mdl::MapFixtureConfig config)
{
  m_config = std::move(config);

  const auto absPath = path.is_absolute() ? path : std::filesystem::current_path() / path;

  contract_assert(
    loadFixtureDocument(absPath, *m_config, *m_taskManager, *m_resourceManager)
    | kdl::transform([&](auto document) { m_document = std::move(document); })
    | kdl::is_success());

  return *m_document;
}

} // namespace tb::ui
