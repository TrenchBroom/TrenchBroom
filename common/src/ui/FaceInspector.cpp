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

#include "FaceInspector.h"

#include <QLabel>
#include <QVBoxLayout>

#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Material.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "ui/BorderLine.h"
#include "ui/FaceAttribsEditor.h"
#include "ui/MapDocument.h"
#include "ui/MaterialBrowser.h"
#include "ui/MaterialCollectionEditor.h"
#include "ui/QtUtils.h"
#include "ui/Splitter.h"
#include "ui/SwitchableTitledPanel.h"

#include <algorithm>
#include <vector>

namespace tb::ui
{
namespace
{

void resetMaterialBrowserInfo(mdl::Map& map, QWidget* materialBrowserInfo)
{
  const auto& gameConfig = map.game()->config();
  materialBrowserInfo->setVisible(gameConfig.materialConfig.property != std::nullopt);
}

} // namespace

FaceInspector::FaceInspector(
  MapDocument& document, GLContextManager& contextManager, QWidget* parent)
  : TabBookPage{parent}
  , m_document{document}
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

void FaceInspector::revealMaterial(const mdl::Material* material)
{
  m_materialBrowser->revealMaterial(material);
  m_materialBrowser->setSelectedMaterial(material);
}

void FaceInspector::createGui(GLContextManager& contextManager)
{
  m_splitter = new Splitter{Qt::Vertical};
  m_splitter->setObjectName("FaceInspector_Splitter");

  m_splitter->addWidget(createFaceAttribsEditor(contextManager));
  m_splitter->addWidget(createMaterialBrowser(contextManager));

  // when the window resizes, the browser should get extra space
  m_splitter->setStretchFactor(0, 0);
  m_splitter->setStretchFactor(1, 1);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_splitter, 1);
  setLayout(layout);

  connect(
    m_materialBrowser,
    &MaterialBrowser::materialSelected,
    this,
    &FaceInspector::materialSelected);

  restoreWindowState(m_splitter);
}

QWidget* FaceInspector::createFaceAttribsEditor(GLContextManager& contextManager)
{
  m_faceAttribsEditor = new FaceAttribsEditor{m_document, contextManager};
  return m_faceAttribsEditor;
}

QWidget* FaceInspector::createMaterialBrowser(GLContextManager& contextManager)
{
  auto* panel =
    new SwitchableTitledPanel{tr("Material Browser"), {{tr("Browser"), tr("Settings")}}};

  m_materialBrowser = new MaterialBrowser{m_document, contextManager};

  auto* materialBrowserLayout = new QVBoxLayout{};
  materialBrowserLayout->setContentsMargins(0, 0, 0, 0);
  materialBrowserLayout->addWidget(m_materialBrowser, 1);
  panel->getPanel(0)->setLayout(materialBrowserLayout);

  auto* materialCollectionEditor = new MaterialCollectionEditor{m_document};
  m_materialBrowserInfo = createMaterialBrowserInfo();

  auto* materialCollectionEditorLayout = new QVBoxLayout{};
  materialCollectionEditorLayout->setContentsMargins(0, 0, 0, 0);
  materialCollectionEditorLayout->setSpacing(0);
  materialCollectionEditorLayout->addWidget(materialCollectionEditor, 1);
  materialCollectionEditorLayout->addWidget(m_materialBrowserInfo, 0);

  panel->getPanel(1)->setLayout(materialCollectionEditorLayout);

  return panel;
}

QWidget* FaceInspector::createMaterialBrowserInfo()
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

void FaceInspector::materialSelected(const mdl::Material* material)
{
  auto& map = m_document.map();
  const auto faces = map.selection().allBrushFaces();

  if (material)
  {
    if (!faces.empty())
    {
      const auto allFacesHaveMaterial = std::ranges::all_of(
        faces,
        [&](const auto& faceHandle) { return faceHandle.face().material() == material; });

      const auto materialNameToSet = !allFacesHaveMaterial
                                       ? material->name()
                                       : mdl::BrushFaceAttributes::NoMaterialName;

      map.setCurrentMaterialName(materialNameToSet);
      setBrushFaceAttributes(map, {.materialName = materialNameToSet});
    }
    else
    {
      map.setCurrentMaterialName(
        map.currentMaterialName() != material->name()
          ? material->name()
          : mdl::BrushFaceAttributes::NoMaterialName);
    }
  }
}

void FaceInspector::connectObservers()
{
  m_notifierConnection += m_document.documentWasCreatedNotifier.connect(
    this, &FaceInspector::documentWasCreated);
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect(this, &FaceInspector::documentWasLoaded);
}

void FaceInspector::documentWasCreated()
{
  resetMaterialBrowserInfo(m_document.map(), m_materialBrowserInfo);
}

void FaceInspector::documentWasLoaded()
{
  resetMaterialBrowserInfo(m_document.map(), m_materialBrowserInfo);
}

} // namespace tb::ui
