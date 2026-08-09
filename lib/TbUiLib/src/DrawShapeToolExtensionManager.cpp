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

#include "ui/DrawShapeToolExtensionManager.h"

#include "ui/DrawShapeToolExtensions.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"

#include <ranges>

namespace tb::ui
{

DrawShapeToolExtensionManager::DrawShapeToolExtensionManager(MapDocument& document)
  : m_extensions{createDrawShapeToolExtensions(document)}
{
  contract_pre(!m_extensions.empty());
}

const std::vector<DrawShapeToolExtension*> DrawShapeToolExtensionManager::extensions()
  const
{
  return m_extensions
         | std::views::transform([](const auto& extension) { return extension.get(); })
         | kdl::ranges::to<std::vector>();
}

const DrawShapeToolExtension& DrawShapeToolExtensionManager::currentExtension() const
{
  return *m_extensions[m_currentExtensionIndex];
}

bool DrawShapeToolExtensionManager::setCurrentExtensionIndex(size_t currentExtensionIndex)
{
  if (currentExtensionIndex != m_currentExtensionIndex)
  {
    m_currentExtensionIndex = currentExtensionIndex;
    currentExtensionDidChangeNotifier(m_currentExtensionIndex);
    return true;
  }

  return false;
}

std::vector<DrawShapeToolExtensionPage*> DrawShapeToolExtensionManager::createToolPages(
  QWidget* parent)
{
  auto toolPages = m_extensions | std::views::transform([&](const auto& extension) {
                     auto* toolPage = extension->createToolPage(m_parameters, parent);
                     m_notifierConnection +=
                       toolPage->applyParametersNotifier.connect(applyParametersNotifier);
                     return toolPage;
                   })
                   | kdl::ranges::to<std::vector>();

  // update all tool pages to reflect the current parameter values
  m_parameters.parametersDidChangeNotifier();

  return toolPages;
}

Result<std::vector<mdl::Brush>> DrawShapeToolExtensionManager::createBrushes(
  const vm::bbox3d& bounds) const
{
  return currentExtension().createBrushes(bounds, m_parameters);
}

} // namespace tb::ui
