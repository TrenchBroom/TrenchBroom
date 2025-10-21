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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QWidget>

#include <string>
#include <vector>

namespace tb::mdl
{
class EntityNodeBase;
class Map;
} // namespace tb::mdl

namespace tb::ui
{

class SmartPropertyEditor : public QWidget
{
  Q_OBJECT
private:
  mdl::Map& m_map;

  std::string m_propertyKey;
  std::vector<mdl::EntityNodeBase*> m_nodes;
  bool m_active = false;

public:
  explicit SmartPropertyEditor(mdl::Map& map, QWidget* parent = nullptr);
  ~SmartPropertyEditor() override;

  bool usesPropertyKey(const std::string& propertyKey) const;

  void activate(const std::string& propertyKey);
  void update(const std::vector<mdl::EntityNodeBase*>& nodes);
  void deactivate();

protected:
  mdl::Map& map();
  const std::string& propertyKey() const;
  const std::vector<mdl::EntityNodeBase*> nodes() const;
  void addOrUpdateProperty(const std::string& value);

private:
  virtual void doUpdateVisual(const std::vector<mdl::EntityNodeBase*>& nodes) = 0;
};

} // namespace tb::ui
