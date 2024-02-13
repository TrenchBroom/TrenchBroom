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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FaceInspector.h"

#include <QVBoxLayout>

#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceHandle.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "View/BorderLine.h"
#include "View/FaceAttribsEditor.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"
#include "View/SwitchableTitledPanel.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"

#include <kdl/memory_utils.h>

#include <vector>

namespace TrenchBroom::View
{
FaceInspector::FaceInspector(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : TabBookPage{parent}
  , m_document{std::move(document)}
{
  createGui(m_document, contextManager);
}

FaceInspector::~FaceInspector()
{
  saveWindowState(m_splitter);
}

bool FaceInspector::cancelMouseDrag()
{
  return m_faceAttribsEditor->cancelMouseDrag();
}

void FaceInspector::revealTexture(const Assets::Texture* texture)
{
  m_textureBrowser->revealTexture(texture);
  m_textureBrowser->setSelectedTexture(texture);
}

void FaceInspector::createGui(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager)
{
  m_splitter = new Splitter{Qt::Vertical};
  m_splitter->setObjectName("FaceInspector_Splitter");

  m_splitter->addWidget(createFaceAttribsEditor(m_splitter, document, contextManager));
  m_splitter->addWidget(createTextureBrowser(m_splitter, document, contextManager));

  // when the window resizes, the browser should get extra space
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_splitter, 1);
  setLayout(layout);

  connect(
    m_textureBrowser,
    &TextureBrowser::textureSelected,
    this,
    &FaceInspector::textureSelected);

  restoreWindowState(m_splitter);
}

QWidget* FaceInspector::createFaceAttribsEditor(
  QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager)
{
  m_faceAttribsEditor =
    new FaceAttribsEditor{std::move(document), contextManager, parent};
  return m_faceAttribsEditor;
}

QWidget* FaceInspector::createTextureBrowser(
  QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager)
{
  auto* panel = new SwitchableTitledPanel{
    tr("Texture Browser"), {{tr("Browser"), tr("Settings")}}, parent};

  m_textureBrowser = new TextureBrowser{document, contextManager};

  auto* textureBrowserLayout = new QVBoxLayout{};
  textureBrowserLayout->setContentsMargins(0, 0, 0, 0);
  textureBrowserLayout->addWidget(m_textureBrowser, 1);
  panel->getPanel(0)->setLayout(textureBrowserLayout);

  auto* textureCollectionEditor = new TextureCollectionEditor{document};

  auto* textureCollectionEditorLayout = new QVBoxLayout{};
  textureCollectionEditorLayout->setContentsMargins(0, 0, 0, 0);
  textureCollectionEditorLayout->addWidget(textureCollectionEditor, 1);
  panel->getPanel(1)->setLayout(textureCollectionEditorLayout);

  return panel;
}

static bool allFacesHaveTexture(
  const std::vector<Model::BrushFaceHandle>& faceHandles, const Assets::Texture* texture)
{
  return std::all_of(faceHandles.begin(), faceHandles.end(), [&](const auto& faceHandle) {
    return faceHandle.face().texture() == texture;
  });
}

void FaceInspector::textureSelected(const Assets::Texture* texture)
{
  auto document = kdl::mem_lock(m_document);
  const auto faces = document->allSelectedBrushFaces();

  if (texture)
  {
    if (!faces.empty())
    {
      const auto textureNameToSet = !allFacesHaveTexture(faces, texture)
                                      ? texture->name()
                                      : Model::BrushFaceAttributes::NoTextureName;

      document->setCurrentTextureName(textureNameToSet);
      auto request = Model::ChangeBrushFaceAttributesRequest{};
      request.setTextureName(textureNameToSet);
      document->setFaceAttributes(request);
    }
    else
    {
      document->setCurrentTextureName(
        document->currentTextureName() != texture->name()
          ? texture->name()
          : Model::BrushFaceAttributes::NoTextureName);
    }
  }
}
} // namespace TrenchBroom::View
