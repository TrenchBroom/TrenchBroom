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

#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBoxItemRenderer::ImageListBoxItemRenderer(size_t index, const QString& title, const QString& subtitle, const QPixmap& image, QWidget* parent)  :
        ControlListBoxItemRenderer(parent),
        m_index(index),
        m_titleLabel(nullptr),
        m_subtitleLabel(nullptr),
        m_imageLabel(nullptr) {
            createGui(title, subtitle, image);
        }

        void ImageListBoxItemRenderer::setSelected(const bool selected) {
            if (selected) {
                makeSelected(m_titleLabel);
                makeSelected(m_subtitleLabel);
            } else {
                makeEmphasized(m_titleLabel);
                makeInfo(m_subtitleLabel);
            }
        }

        void ImageListBoxItemRenderer::createGui(const QString& title, const QString& subtitle, const QPixmap& image) {
            m_titleLabel = new QLabel(title);
            makeEmphasized(m_titleLabel);

            m_subtitleLabel = new QLabel(subtitle);
            makeInfo(m_subtitleLabel);

            auto* imageAndTextLayout = new QHBoxLayout();
            imageAndTextLayout->setContentsMargins(QMargins(2, 2, 2, 2));
            imageAndTextLayout->setSpacing(2);
            setLayout(imageAndTextLayout);

            if (!image.isNull()) {
                m_imageLabel = new QLabel(this);
                m_imageLabel->setPixmap(image);
                imageAndTextLayout->addWidget(m_imageLabel);
            }

            auto* textLayout = new QVBoxLayout();
            textLayout->setContentsMargins(QMargins());
            textLayout->setSpacing(0);
            imageAndTextLayout->addLayout(textLayout, 1);

            textLayout->addWidget(m_titleLabel);
            textLayout->addWidget(m_subtitleLabel);
        }

        void ImageListBoxItemRenderer::mouseDoubleClickEvent(QMouseEvent* event) {
            if (event->button() == Qt::LeftButton) {
                emit doubleClick(m_index);
            }
        }

        ImageListBox::ImageListBox(const QString& emptyText, QWidget* parent) :
        ControlListBox(emptyText, parent) {}

        ControlListBoxItemRenderer* ImageListBox::createItemRenderer(QWidget* parent, const size_t index) {
            auto* result = new ImageListBoxItemRenderer(index, title(index), subtitle(index), image(index), parent);
            connect(result, &ImageListBoxItemRenderer::doubleClick, this, &ImageListBox::onItemDoubleClick);
            return result;
        }

        QPixmap ImageListBox::image(const size_t index) const {
            return QPixmap();
        }

        void ImageListBox::onItemDoubleClick(size_t index) {}
    }
}
