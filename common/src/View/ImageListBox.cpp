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

#include "ImageListBox.h"

#include "View/ElidedLabel.h"
#include "View/QtUtils.h"
#include "View/ViewConstants.h"

#include <QBoxLayout>
#include <QLabel>

#include <cassert>

namespace TrenchBroom {
namespace View {
ImageListBoxItemRenderer::ImageListBoxItemRenderer(
  const QString& title, const QString& subtitle, const QPixmap& image, QWidget* parent)
  : ControlListBoxItemRenderer(parent)
  , m_titleLabel(nullptr)
  , m_subtitleLabel(nullptr)
  , m_imageLabel(nullptr) {
  m_titleLabel = new ElidedLabel(title, Qt::ElideRight);
  makeEmphasized(m_titleLabel);

  m_subtitleLabel = new ElidedLabel(subtitle, Qt::ElideMiddle);
  makeInfo(m_subtitleLabel);

  auto* imageAndTextLayout = new QHBoxLayout();
  imageAndTextLayout->setContentsMargins(0, 0, 0, 0);
  imageAndTextLayout->setSpacing(LayoutConstants::MediumHMargin);
  setLayout(imageAndTextLayout);

  m_imageLabel = new QLabel(this);
  imageAndTextLayout->addWidget(m_imageLabel);
  m_imageLabel->setPixmap(image);

  auto* textLayout = new QVBoxLayout();
  textLayout->setContentsMargins(0, 0, 0, 0);
  textLayout->setSpacing(0);
  textLayout->addWidget(m_titleLabel);
  textLayout->addWidget(m_subtitleLabel);

  imageAndTextLayout->addLayout(textLayout, 1);
}

void ImageListBoxItemRenderer::updateItem() {
  QObject* element = this->parent();
  ImageListBox* listBox = nullptr;
  do {
    listBox = dynamic_cast<ImageListBox*>(element);
    element = element->parent();
  } while (listBox == nullptr && element != nullptr);
  if (listBox != nullptr) {
    m_titleLabel->setText(listBox->title(m_index));
    m_subtitleLabel->setText(listBox->subtitle(m_index));
    m_imageLabel->setPixmap(listBox->image(m_index));
  }
}

ImageListBox::ImageListBox(const QString& emptyText, bool showSeparator, QWidget* parent)
  : ControlListBox(emptyText, showSeparator, parent) {}

ControlListBoxItemRenderer* ImageListBox::createItemRenderer(QWidget* parent, const size_t index) {
  return new ImageListBoxItemRenderer(title(index), subtitle(index), image(index), parent);
}

QPixmap ImageListBox::image(const size_t /* index */) const {
  return QPixmap();
}
} // namespace View
} // namespace TrenchBroom
