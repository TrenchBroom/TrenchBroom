/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

namespace TrenchBroom
{
namespace Model
{
class EntityNodeBase;
}

namespace View
{
class MapDocument;

class SmartPropertyEditor : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  std::string m_propertyKey;
  std::vector<Model::EntityNodeBase*> m_nodes;
  bool m_active;

public:
  explicit SmartPropertyEditor(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
  ~SmartPropertyEditor() override;

  bool usesPropertyKey(const std::string& propertyKey) const;

  void activate(const std::string& propertyKey);
  void update(const std::vector<Model::EntityNodeBase*>& nodes);
  void deactivate();

protected:
  std::shared_ptr<MapDocument> document() const;
  const std::string& propertyKey() const;
  const std::vector<Model::EntityNodeBase*> nodes() const;
  void addOrUpdateProperty(const std::string& value);

private:
  virtual void doUpdateVisual(const std::vector<Model::EntityNodeBase*>& nodes) = 0;
};
} // namespace View
} // namespace TrenchBroom
