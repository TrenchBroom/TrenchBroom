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

#include "Ensure.h"
#include "Map.h"
#include "mdl/EmptyBrushEntityValidator.h"
#include "mdl/EmptyGroupValidator.h"
#include "mdl/EmptyPropertyKeyValidator.h"
#include "mdl/EmptyPropertyValueValidator.h"
#include "mdl/Game.h"
#include "mdl/InvalidUVScaleValidator.h"
#include "mdl/Issue.h"
#include "mdl/LinkSourceValidator.h"
#include "mdl/LongPropertyKeyValidator.h"
#include "mdl/LongPropertyValueValidator.h"
#include "mdl/MissingClassnameValidator.h"
#include "mdl/MissingDefinitionValidator.h"
#include "mdl/MissingModValidator.h"
#include "mdl/MixedBrushContentsValidator.h"
#include "mdl/NonIntegerVerticesValidator.h"
#include "mdl/PointEntityWithBrushesValidator.h"
#include "mdl/PropertyKeyWithDoubleQuotationMarksValidator.h"
#include "mdl/PropertyValueWithDoubleQuotationMarksValidator.h"
#include "mdl/SoftMapBoundsValidator.h"
#include "mdl/WorldBoundsValidator.h"
#include "mdl/WorldNode.h"

#include <memory>

namespace tb::mdl
{

void Map::registerValidators()
{
  ensure(m_world, "world is null");
  ensure(m_game != nullptr, "game is null");

  m_world->registerValidator(std::make_unique<MissingClassnameValidator>());
  m_world->registerValidator(std::make_unique<MissingDefinitionValidator>());
  m_world->registerValidator(std::make_unique<MissingModValidator>(*m_game));
  m_world->registerValidator(std::make_unique<EmptyGroupValidator>());
  m_world->registerValidator(std::make_unique<EmptyBrushEntityValidator>());
  m_world->registerValidator(std::make_unique<PointEntityWithBrushesValidator>());
  m_world->registerValidator(std::make_unique<LinkSourceValidator>());
  m_world->registerValidator(std::make_unique<LinkSourceValidator>());
  m_world->registerValidator(std::make_unique<NonIntegerVerticesValidator>());
  m_world->registerValidator(std::make_unique<MixedBrushContentsValidator>());
  m_world->registerValidator(std::make_unique<WorldBoundsValidator>(worldBounds()));
  m_world->registerValidator(std::make_unique<SoftMapBoundsValidator>(*m_game, *m_world));
  m_world->registerValidator(std::make_unique<EmptyPropertyKeyValidator>());
  m_world->registerValidator(std::make_unique<EmptyPropertyValueValidator>());
  m_world->registerValidator(
    std::make_unique<LongPropertyKeyValidator>(m_game->config().maxPropertyLength));
  m_world->registerValidator(
    std::make_unique<LongPropertyValueValidator>(m_game->config().maxPropertyLength));
  m_world->registerValidator(
    std::make_unique<PropertyKeyWithDoubleQuotationMarksValidator>());
  m_world->registerValidator(
    std::make_unique<PropertyValueWithDoubleQuotationMarksValidator>());
  m_world->registerValidator(std::make_unique<InvalidUVScaleValidator>());
}

void Map::setIssueHidden(const Issue& issue, const bool hidden)
{
  if (issue.hidden() != hidden)
  {
    issue.node().setIssueHidden(issue.type(), hidden);
  }
}

} // namespace tb::mdl
