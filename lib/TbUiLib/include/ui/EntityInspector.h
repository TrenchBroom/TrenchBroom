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

#pragma once

#include "ui/TabBook.h"

class QSplitter;

namespace tb
{
namespace gl
{
class ContextManager;
}

namespace ui
{
class EntityBrowser;
class EntityPropertyEditor;
class MapDocument;

class EntityInspector : public TabBookPage
{
  Q_OBJECT
private:
  QSplitter* m_splitter = nullptr;
  EntityPropertyEditor* m_attributeEditor = nullptr;
  EntityBrowser* m_entityBrowser = nullptr;

public:
  EntityInspector(
    MapDocument& document, gl::ContextManager& contextManager, QWidget* parent = nullptr);
  ~EntityInspector() override;

private:
  void createGui(MapDocument& document, gl::ContextManager& contextManager);
  QWidget* createAttributeEditor(MapDocument& document, QWidget* parent);
  QWidget* createEntityBrowser(
    MapDocument& document, gl::ContextManager& contextManager, QWidget* parent);
};

} // namespace ui
} // namespace tb
