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

#include <QLabel>
#include <QVBoxLayout>

#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushFaceHandle.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/FaceAttribsEditor.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/Splitter.h"
#include "View/SwitchableTitledPanel.h"
#include "View/TextureBrowser.h"
#include "View/TextureCollectionEditor.h"

#include "kdl/memory_utils.h"

#include <vector>

namespace TrenchBroom::View
{
FaceInspector::FaceInspector(
  std::weak_ptr<MapDocument> document, GLContextManager& contextManager, QWidget* parent)
  : TabBookPage{parent}
  , m_document{std::move(document)}
{
  createGui(contextManager);
  connectObservers();
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

void FaceInspector::createGui(GLContextManager& contextManager)
{
  m_splitter = new Splitter{Qt::Vertical};
  m_splitter->setObjectName("FaceInspector_Splitter");

  m_splitter->addWidget(createFaceAttribsEditor(contextManager));
  m_splitter->addWidget(createTextureBrowser(contextManager));

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

QWidget* FaceInspector::createFaceAttribsEditor(GLContextManager& contextManager)
{
  m_faceAttribsEditor = new FaceAttribsEditor{m_document, contextManager};
  return m_faceAttribsEditor;
}

QWidget* FaceInspector::createTextureBrowser(GLContextManager& contextManager)
{
  auto* panel =
    new SwitchableTitledPanel{tr("Texture Browser"), {{tr("Browser"), tr("Settings")}}};

  m_textureBrowser = new TextureBrowser{m_document, contextManager};

  auto* textureBrowserLayout = new QVBoxLayout{};
  textureBrowserLayout->setContentsMargins(0, 0, 0, 0);
  textureBrowserLayout->addWidget(m_textureBrowser, 1);
  panel->getPanel(0)->setLayout(textureBrowserLayout);

  auto* textureCollectionEditor = new TextureCollectionEditor{m_document};
  m_textureBrowserInfo = createTextureBrowserInfo();

  auto* textureCollectionEditorLayout = new QVBoxLayout{};
  textureCollectionEditorLayout->setContentsMargins(0, 0, 0, 0);
  textureCollectionEditorLayout->setSpacing(0);
  textureCollectionEditorLayout->addWidget(textureCollectionEditor, 1);
  textureCollectionEditorLayout->addWidget(m_textureBrowserInfo, 0);

  panel->getPanel(1)->setLayout(textureCollectionEditorLayout);

  return panel;
}

QWidget* FaceInspector::createTextureBrowserInfo()
{
  auto* label = new QLabel{tr(
    R"(To manage wad files, select the "wad" property of the worldspawn entity to reveal a wad file manager below the entity property table.)")};

  label->setWordWrap(true);
  makeInfo(label);

  auto* labelLayout = new QVBoxLayout{};
  labelLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  labelLayout->addWidget(label);

  auto* panelLayout = new QVBoxLayout{};
  panelLayout->setContentsMargins(0, 0, 0, 0);
  panelLayout->setSpacing(0);
  panelLayout->addWidget(new BorderLine{}, 0);
  panelLayout->addLayout(labelLayout);

  auto* panel = new QWidget{};
  panel->setLayout(panelLayout);
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

void FaceInspector::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &FaceInspector::documentWasNewedOrOpened);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &FaceInspector::documentWasNewedOrOpened);
}

void FaceInspector::documentWasNewedOrOpened(MapDocument* document)
{
  const auto& game = *document->game();
  const auto& gameConfig = Model::GameFactory::instance().gameConfig(game.gameName());
  m_textureBrowserInfo->setVisible(gameConfig.textureConfig.property != std::nullopt);
}
} // namespace TrenchBroom::View
