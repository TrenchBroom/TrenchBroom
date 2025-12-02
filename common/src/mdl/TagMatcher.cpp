/*
 Copyright (C) 2010 Kristian Duske

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

#include "TagMatcher.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "mdl/Selection.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/string_compare.h"
#include "kd/struct_io.h"

#include <algorithm>
#include <ostream>
#include <ranges>
#include <vector>

namespace tb::mdl
{

namespace
{
class MatchVisitor : public ConstTagVisitor
{
private:
  bool m_matches = false;

public:
  bool matches() const { return m_matches; }

protected:
  void setMatches() { m_matches = true; }
};

class BrushFaceMatchVisitor : public MatchVisitor
{
private:
  std::function<bool(const BrushFace&)> m_matcher;

public:
  explicit BrushFaceMatchVisitor(std::function<bool(const BrushFace&)> matcher)
    : m_matcher(std::move(matcher))
  {
  }

  void visit(const BrushFace& face) override
  {
    if (m_matcher(face))
    {
      setMatches();
    }
  }
};

class BrushMatchVisitor : public MatchVisitor
{
private:
  std::function<bool(const BrushNode&)> m_matcher;

public:
  explicit BrushMatchVisitor(std::function<bool(const BrushNode&)> matcher)
    : m_matcher(std::move(matcher))
  {
  }

  void visit(const BrushNode& brush) override
  {
    if (m_matcher(brush))
    {
      setMatches();
    }
  }
};

} // namespace

void MaterialTagMatcher::enable(TagMatcherCallback& callback, Map& map) const
{
  const auto& materialManager = map.materialManager();
  const auto& allMaterials = materialManager.materials();
  auto matchingMaterials = std::vector<const Material*>{};

  std::ranges::copy_if(
    allMaterials, std::back_inserter(matchingMaterials), [this](auto* material) {
      return matchesMaterial(material);
    });

  std::ranges::sort(matchingMaterials, [](const auto* lhs, const auto* rhs) {
    return kdl::ci::str_compare(lhs->name(), rhs->name()) < 0;
  });

  const Material* material = nullptr;
  if (matchingMaterials.empty())
  {
    return;
  }
  else if (matchingMaterials.size() == 1)
  {
    material = matchingMaterials.front();
  }
  else
  {
    const auto options =
      matchingMaterials
      | std::views::transform([](const auto* current) { return current->name(); })
      | kdl::ranges::to<std::vector>();
    const auto index = callback.selectOption(options);
    if (index >= matchingMaterials.size())
    {
      return;
    }
    material = matchingMaterials[index];
  }

  contract_assert(material != nullptr);

  setBrushFaceAttributes(map, {.materialName = material->name()});
}

bool MaterialTagMatcher::canEnable() const
{
  return true;
}

void MaterialTagMatcher::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "MaterialTagMatcher";
}

MaterialNameTagMatcher::MaterialNameTagMatcher(std::string pattern)
  : m_pattern{std::move(pattern)}
{
}

std::unique_ptr<TagMatcher> MaterialNameTagMatcher::clone() const
{
  return std::make_unique<MaterialNameTagMatcher>(m_pattern);
}

bool MaterialNameTagMatcher::matches(const Taggable& taggable) const
{
  auto visitor = BrushFaceMatchVisitor{[&](const auto& face) {
    return matchesMaterialName(face.attributes().materialName());
  }};

  taggable.accept(visitor);
  return visitor.matches();
}

void MaterialNameTagMatcher::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "MaterialNameTagMatcher"
                          << "m_pattern" << m_pattern;
}

bool MaterialNameTagMatcher::matchesMaterial(const Material* material) const
{
  return material && matchesMaterialName(material->name());
}

bool MaterialNameTagMatcher::matchesMaterialName(std::string_view materialName) const
{
  // If the match pattern doesn't contain a slash, match against
  // only the last component of the material name.
  if (m_pattern.find('/') == std::string::npos)
  {
    const auto pos = materialName.find_last_of('/');
    if (pos != std::string::npos)
    {
      materialName = materialName.substr(pos + 1);
    }
  }

  return kdl::ci::str_matches_glob(materialName, m_pattern);
}

SurfaceParmTagMatcher::SurfaceParmTagMatcher(std::string parameter)
  : m_parameters{{std::move(parameter)}}
{
}

SurfaceParmTagMatcher::SurfaceParmTagMatcher(kdl::vector_set<std::string> parameters)
  : m_parameters{std::move(parameters)}
{
}

std::unique_ptr<TagMatcher> SurfaceParmTagMatcher::clone() const
{
  return std::make_unique<SurfaceParmTagMatcher>(m_parameters);
}

bool SurfaceParmTagMatcher::matches(const Taggable& taggable) const
{
  auto visitor = BrushFaceMatchVisitor{
    [&](const auto& face) { return matchesMaterial(face.material()); }};

  taggable.accept(visitor);
  return visitor.matches();
}

void SurfaceParmTagMatcher::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "SurfaceParmTagMatcher"
                          << "m_parameters" << m_parameters;
}

bool SurfaceParmTagMatcher::matchesMaterial(const Material* material) const
{
  if (material)
  {
    const auto& parameters = material->surfaceParms();
    auto iMaterialParams = parameters.begin();
    auto iTagParams = m_parameters.begin();
    while (iMaterialParams != parameters.end() && iTagParams != m_parameters.end())
    {
      if (*iMaterialParams < *iTagParams)
      {
        ++iMaterialParams;
      }
      else if (*iTagParams < *iMaterialParams)
      {
        ++iTagParams;
      }
      else
      {
        return true;
      }
    }
  }

  return false;
}

FlagsTagMatcher::FlagsTagMatcher(
  const int flags,
  GetFlags getFlags,
  SetFlags setFlags,
  SetFlags unsetFlags,
  GetFlagNames getFlagNames)
  : m_flags{flags}
  , m_getFlags{std::move(getFlags)}
  , m_setFlags{std::move(setFlags)}
  , m_unsetFlags{std::move(unsetFlags)}
  , m_getFlagNames{std::move(getFlagNames)}
{
}

bool FlagsTagMatcher::matches(const Taggable& taggable) const
{
  auto visitor = BrushFaceMatchVisitor{
    [&](const BrushFace& face) { return (m_getFlags(face) & m_flags) != 0; }};

  taggable.accept(visitor);
  return visitor.matches();
}

void FlagsTagMatcher::enable(TagMatcherCallback& callback, Map& map) const
{
  constexpr auto bits = sizeof(decltype(m_flags)) * 8;

  auto flagIndices = std::vector<size_t>{};
  for (size_t i = 0; i < bits; ++i)
  {
    if ((m_flags & (1 << i)) != 0)
    {
      flagIndices.push_back(i);
    }
  }

  auto flagToSet = 0;
  if (flagIndices.empty())
  {
    return;
  }
  else if (flagIndices.size() == 1)
  {
    flagToSet = m_flags;
  }
  else
  {
    const auto options = m_getFlagNames(*map.game(), m_flags);
    const auto selectedOptionIndex = callback.selectOption(options);
    if (selectedOptionIndex == options.size())
    {
      return;
    }

    // convert the option index into the index of the flag to set
    size_t currentIndex = 0;
    for (size_t i = 0; i < bits; ++i)
    {
      if ((m_flags & (1 << i)) != 0)
      {
        // only consider flags which are set to 1
        if (currentIndex == selectedOptionIndex)
        {
          // we found the flag that corresponds to the selected option
          flagToSet = (1 << i);
          break;
        }
        ++currentIndex;
      }
    }
  }

  setBrushFaceAttributes(map, m_setFlags(flagToSet));
}

void FlagsTagMatcher::disable(TagMatcherCallback&, Map& map) const
{
  setBrushFaceAttributes(map, m_unsetFlags(m_flags));
}

bool FlagsTagMatcher::canEnable() const
{
  return true;
}

bool FlagsTagMatcher::canDisable() const
{
  return true;
}

void FlagsTagMatcher::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "FlagsTagMatcher"
                          << "m_flags" << m_flags;
}

ContentFlagsTagMatcher::ContentFlagsTagMatcher(const int i_flags)
  : FlagsTagMatcher{
      i_flags,
      [](const auto& face) { return face.resolvedSurfaceContents(); },
      [](const auto flags) {
        return UpdateBrushFaceAttributes{.surfaceContents = SetFlagBits{flags}};
      },
      [](const auto flags) {
        return UpdateBrushFaceAttributes{.surfaceContents = ClearFlagBits{flags}};
      },
      [](const auto& game, const auto flags) {
        return game.config().faceAttribsConfig.contentFlags.flagNames(flags);
      }}
{
}

std::unique_ptr<TagMatcher> ContentFlagsTagMatcher::clone() const
{
  return std::make_unique<ContentFlagsTagMatcher>(m_flags);
}

SurfaceFlagsTagMatcher::SurfaceFlagsTagMatcher(const int i_flags)
  : FlagsTagMatcher{
      i_flags,
      [](const auto& face) { return face.resolvedSurfaceFlags(); },
      [](const auto flags) {
        return UpdateBrushFaceAttributes{.surfaceFlags = SetFlagBits{flags}};
      },
      [](const auto flags) {
        return UpdateBrushFaceAttributes{.surfaceFlags = ClearFlagBits{flags}};
      },
      [](const auto& game, const auto flags) {
        return game.config().faceAttribsConfig.surfaceFlags.flagNames(flags);
      }}
{
}

std::unique_ptr<TagMatcher> SurfaceFlagsTagMatcher::clone() const
{
  return std::make_unique<SurfaceFlagsTagMatcher>(m_flags);
}

EntityClassNameTagMatcher::EntityClassNameTagMatcher(
  std::string pattern, std::string material)
  : m_pattern{std::move(pattern)}
  , m_material{std::move(material)}
{
}

std::unique_ptr<TagMatcher> EntityClassNameTagMatcher::clone() const
{
  return std::make_unique<EntityClassNameTagMatcher>(m_pattern, m_material);
}

bool EntityClassNameTagMatcher::matches(const Taggable& taggable) const
{
  BrushMatchVisitor visitor([this](const BrushNode& brush) {
    if (const auto* entityNode = brush.entity())
    {
      return matchesClassname(entityNode->entity().classname());
    }
    else
    {
      return false;
    }
  });

  taggable.accept(visitor);
  return visitor.matches();
}

void EntityClassNameTagMatcher::enable(TagMatcherCallback& callback, Map& map) const
{
  if (!map.selection().hasOnlyBrushes())
  {
    return;
  }

  auto matchingDefinitions = map.entityDefinitionManager().definitions()
                             | std::views::filter([&](const auto& definition) {
                                 return getType(definition) == EntityDefinitionType::Brush
                                        && matchesClassname(definition.name);
                               })
                             | std::views::transform([](const auto& d) { return &d; })
                             | kdl::ranges::to<std::vector>();

  std::ranges::sort(matchingDefinitions, [](const auto* lhs, const auto* rhs) {
    return kdl::ci::str_compare(lhs->name, rhs->name) < 0;
  });

  const EntityDefinition* definition = nullptr;
  if (matchingDefinitions.empty())
  {
    return;
  }
  else if (matchingDefinitions.size() == 1)
  {
    definition = matchingDefinitions.front();
  }
  else
  {
    const auto options = matchingDefinitions
                         | std::views::transform([](const auto& d) { return d->name; })
                         | kdl::ranges::to<std::vector>();
    const auto index = callback.selectOption(options);
    if (index >= matchingDefinitions.size())
    {
      return;
    }
    definition = matchingDefinitions[index];
  }

  contract_assert(definition != nullptr);
  createBrushEntity(map, *definition);

  if (!m_material.empty())
  {
    setBrushFaceAttributes(map, {.materialName = m_material});
  }
}

void EntityClassNameTagMatcher::disable(TagMatcherCallback&, Map& map) const
{
  // entities will be removed automatically when they become empty

  const auto selectedBrushes = map.selection().nodes;
  auto detailBrushes = std::vector<Node*>{};
  for (auto* brush : selectedBrushes)
  {
    if (matches(*brush))
    {
      detailBrushes.push_back(brush);
    }
  }

  if (detailBrushes.empty())
  {
    return;
  }
  deselectAll(map);
  reparentNodes(map, {{parentForNodes(map, selectedBrushes), detailBrushes}});
  selectNodes(
    map, std::vector<Node*>(std::begin(detailBrushes), std::end(detailBrushes)));
}

bool EntityClassNameTagMatcher::canEnable() const
{
  return true;
}

bool EntityClassNameTagMatcher::canDisable() const
{
  return true;
}

void EntityClassNameTagMatcher::appendToStream(std::ostream& str) const
{
  kdl::struct_stream{str} << "EntityClassNameMatcher"
                          << "m_pattern" << m_pattern << "m_material" << m_material;
}

bool EntityClassNameTagMatcher::matchesClassname(const std::string& classname) const
{
  return kdl::ci::str_matches_glob(classname, m_pattern);
}

} // namespace tb::mdl
