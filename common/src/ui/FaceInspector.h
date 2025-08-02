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

#include "NotifierConnection.h"
#include "ui/TabBook.h"

#include <memory>

class QSplitter;
class QWidget;

namespace tb::mdl
{
class Map;
class Material;
} // namespace tb::mdl

namespace tb::ui
{
class CollapsibleTitledPanel;
class FaceAttribsEditor;
class GLContextManager;
class MapDocument;
class MaterialBrowser;

class FaceInspector : public TabBookPage
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  QSplitter* m_splitter = nullptr;
  FaceAttribsEditor* m_faceAttribsEditor = nullptr;
  MaterialBrowser* m_materialBrowser = nullptr;
  QWidget* m_materialBrowserInfo = nullptr;

  NotifierConnection m_notifierConnection;

public:
  FaceInspector(
    std::weak_ptr<MapDocument> document,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);
  ~FaceInspector() override;

  bool cancelMouseDrag();
  void revealMaterial(const mdl::Material* material);

private:
  void createGui(GLContextManager& contextManager);
  QWidget* createFaceAttribsEditor(GLContextManager& contextManager);
  QWidget* createMaterialBrowser(GLContextManager& contextManager);
  QWidget* createMaterialBrowserInfo();

  void materialSelected(const mdl::Material* material);

  void connectObservers();
  void mapWasCreated(mdl::Map& map);
  void mapWasLoaded(mdl::Map& map);
};

} // namespace tb::ui
