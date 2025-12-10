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

#include "MapDocumentFixture.h"

#include "TestUtils.h"
#include "mdl/Map.h"
#include "mdl/Resource.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"

namespace tb::ui
{

MapDocumentFixture::MapDocumentFixture()
  : m_taskManager{createTestTaskManager()}
{
}

MapDocumentFixture::~MapDocumentFixture() = default;

MapDocument& MapDocumentFixture::create(mdl::MapFixtureConfig config)
{
  m_config = std::move(config);

  const auto mapFormat = m_config->mapFormat.value_or(mdl::MapFormat::Standard);

  contract_assert(
    MapDocument::createDocument(
      mapFormat, m_config->gameInfo, vm::bbox3d{8192.0}, *m_taskManager)
    | kdl::transform([&](auto document) {
        m_document = std::move(document);
        m_document->map().setIsCommandCollationEnabled(false);
      })
    | kdl::is_success());

  return *m_document;
}

MapDocument& MapDocumentFixture::load(
  const std::filesystem::path& path, mdl::MapFixtureConfig config)
{
  m_config = std::move(config);

  const auto absPath = path.is_absolute() ? path : std::filesystem::current_path() / path;

  const auto mapFormat = m_config->mapFormat.value_or(mdl::MapFormat::Unknown);

  contract_assert(
    MapDocument::loadDocument(
      absPath, mapFormat, m_config->gameInfo, vm::bbox3d{8192.0}, *m_taskManager)
    | kdl::transform([&](auto document) {
        m_document = std::move(document);
        m_document->map().setIsCommandCollationEnabled(false);
        m_document->map().processResourcesSync(
          mdl::ProcessContext{false, [](auto, auto) {}});
      })
    | kdl::is_success());

  return *m_document;
}

} // namespace tb::ui
