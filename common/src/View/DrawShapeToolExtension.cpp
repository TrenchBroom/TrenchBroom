/*
 Copyright (C) 2023 Kristian Duske

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

#include "DrawShapeToolExtension.h"

#include "Ensure.h"

#include "kdl/vector_utils.h"

namespace TrenchBroom::View
{

DrawShapeToolExtension::~DrawShapeToolExtension() = default;

DrawShapeToolExtensionManager::DrawShapeToolExtensionManager(
  std::vector<std::unique_ptr<DrawShapeToolExtension>> extensions)
  : m_extensions{std::move(extensions)}
{
  ensure(!m_extensions.empty(), "extensions must not be empty");
}

const std::vector<DrawShapeToolExtension*> DrawShapeToolExtensionManager::extensions()
  const
{
  return kdl::vec_transform(
    m_extensions, [](const auto& extension) { return extension.get(); });
}

DrawShapeToolExtension& DrawShapeToolExtensionManager::currentExtension()
{
  return *m_extensions[m_currentExtensionIndex];
}

void DrawShapeToolExtensionManager::setCurrentExtensionIndex(size_t currentExtensionIndex)
{
  if (currentExtensionIndex != m_currentExtensionIndex)
  {
    m_currentExtensionIndex = currentExtensionIndex;
    currentExtensionDidChangeNotifier(m_currentExtensionIndex);
  }
}

} // namespace TrenchBroom::View
