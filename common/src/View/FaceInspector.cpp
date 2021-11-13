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

#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceHandle.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "View/BorderLine.h"
#include "View/CollapsibleTitledPanel.h"
#include "View/FaceAttribsEditor.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"
#include "View/TitledPanel.h"

#include <kdl/memory_utils.h>

#include <QVBoxLayout>

#include <vector>

namespace TrenchBroom {
namespace View {
FaceInspector::FaceInspector(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : TabBookPage(parent)
  , m_document(document)
  , m_splitter(nullptr)
  , m_faceAttribsEditor(nullptr)
  , m_textureBrowser(nullptr)
  , m_textureCollectionsEditor(nullptr) {
  createGui(document, contextManager);
}

FaceInspector::~FaceInspector() {
  saveWindowState(m_splitter);
  saveWindowState(m_textureCollectionsEditor);
}

bool FaceInspector::cancelMouseDrag() {
  return m_faceAttribsEditor->cancelMouseDrag();
}

void FaceInspector::revealTexture(const Assets::Texture* texture) {
  m_textureBrowser->revealTexture(texture);
  m_textureBrowser->setSelectedTexture(texture);
}

void FaceInspector::createGui(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager) {
  m_splitter = new Splitter(Qt::Vertical);
  m_splitter->setObjectName("FaceInspector_Splitter");

  m_splitter->addWidget(createFaceAttribsEditor(m_splitter, document, contextManager));
  m_splitter->addWidget(createTextureBrowser(m_splitter, document, contextManager));

  // when the window resizes, the browser should get extra space
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);

  m_textureCollectionsEditor = createTextureCollectionEditor(this, document);

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_splitter, 1);
  layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
  layout->addWidget(m_textureCollectionsEditor);
  setLayout(layout);

  connect(
    m_textureBrowser, &TextureBrowser::textureSelected, this, &FaceInspector::textureSelected);

  restoreWindowState(m_splitter);
}

QWidget* FaceInspector::createFaceAttribsEditor(
  QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager) {
  m_faceAttribsEditor = new FaceAttribsEditor(std::move(document), contextManager, parent);
  return m_faceAttribsEditor;
}

QWidget* FaceInspector::createTextureBrowser(
  QWidget* parent, std::weak_ptr<MapDocument> document, GLContextManager& contextManager) {
  TitledPanel* panel = new TitledPanel(tr("Texture Browser"), parent);
  m_textureBrowser = new TextureBrowser(std::move(document), contextManager);

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_textureBrowser, 1);
  panel->getPanel()->setLayout(layout);

  return panel;
}

CollapsibleTitledPanel* FaceInspector::createTextureCollectionEditor(
  QWidget* parent, std::weak_ptr<MapDocument> document) {
  auto* panel = new CollapsibleTitledPanel(tr("Texture Collections"), true, parent);
  panel->setObjectName("FaceInspector_TextureCollections");

  auto* collectionEditor = new TextureCollectionEditor(std::move(document));

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(collectionEditor, 1);
  panel->getPanel()->setLayout(layout);

  restoreWindowState(panel);

  return panel;
}

static bool allFacesHaveTexture(
  const std::vector<Model::BrushFaceHandle>& faceHandles, const Assets::Texture* texture) {
  for (const Model::BrushFaceHandle& faceHandle : faceHandles) {
    if (faceHandle.face().texture() != texture) {
      return false;
    }
  }
  return true;
}

void FaceInspector::textureSelected(const Assets::Texture* texture) {
  auto document = kdl::mem_lock(m_document);
  const std::vector<Model::BrushFaceHandle> faces = document->allSelectedBrushFaces();

  if (texture != nullptr) {
    if (faces.empty()) {
      if (document->currentTextureName() == texture->name()) {
        document->setCurrentTextureName(Model::BrushFaceAttributes::NoTextureName);
      } else {
        document->setCurrentTextureName(texture->name());
      }
    } else {
      if (allFacesHaveTexture(faces, texture)) {
        texture = nullptr;
        document->setCurrentTextureName(Model::BrushFaceAttributes::NoTextureName);
      } else {
        document->setCurrentTextureName(texture->name());
      }
    }
  }

  if (!faces.empty()) {
    Model::ChangeBrushFaceAttributesRequest request;
    if (texture == nullptr) {
      request.setTextureName(Model::BrushFaceAttributes::NoTextureName);
    } else {
      request.setTextureName(texture->name());
    }
    document->setFaceAttributes(request);
  }
}
} // namespace View
} // namespace TrenchBroom
