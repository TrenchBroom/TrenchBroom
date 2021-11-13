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

#include "TextureCollectionEditor.h"

#include "Model/Game.h"
#include "View/DirectoryTextureCollectionEditor.h"
#include "View/FileTextureCollectionEditor.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QVBoxLayout>

namespace TrenchBroom {
namespace View {
TextureCollectionEditor::TextureCollectionEditor(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget(parent)
  , m_document(std::move(document)) {
  auto doc = kdl::mem_lock(m_document);
  m_notifierConnection +=
    doc->documentWasNewedNotifier.connect(this, &TextureCollectionEditor::documentWasNewedOrLoaded);
  m_notifierConnection += doc->documentWasLoadedNotifier.connect(
    this, &TextureCollectionEditor::documentWasNewedOrLoaded);
}

void TextureCollectionEditor::documentWasNewedOrLoaded(MapDocument*) {
  createGui();
}

void TextureCollectionEditor::createGui() {
  deleteChildWidgetsLaterAndDeleteLayout(this);

  QWidget* collectionEditor = nullptr;

  auto document = kdl::mem_lock(m_document);
  const auto type = document->game()->texturePackageType();
  switch (type) {
    case Model::Game::TexturePackageType::File:
      collectionEditor = new FileTextureCollectionEditor(m_document);
      break;
    case Model::Game::TexturePackageType::Directory:
      collectionEditor = new DirectoryTextureCollectionEditor(m_document);
      break;
  }

  auto* layout = new QVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(collectionEditor, 1);
  setLayout(layout);
}
} // namespace View
} // namespace TrenchBroom
