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

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBox::ImageListBox(const QString& emptyText, QWidget* parent) :
        ControlListBox(emptyText, parent) {}

        class ImageListBox::ImageListBoxItemRenderer : public ItemRenderer {
        private:
            QLabel* m_titleLabel;
            QLabel* m_subtitleLabel;
            QLabel* m_imageLabel;
        public:
            ImageListBoxItemRenderer(const QString& title, const QString& subtitle, const QPixmap& image, QWidget* parent)  :
            ItemRenderer(parent),
            m_titleLabel(nullptr),
            m_subtitleLabel(nullptr),
            m_imageLabel(nullptr) {
                createGui(title, subtitle, image);
            }

            void setDefaultColors(const QColor& foreground, const QColor& background) override {
                /*
                Item::setDefaultColours(foreground, background);
                m_subtitleText->SetForegroundColour(makeLighter(m_subtitleText->GetForegroundColour()));
                 */
            }
        private:
            void createGui(const QString& title, const QString& subtitle, const QPixmap& image) {
                m_titleLabel = new QLabel(title, this);
                m_subtitleLabel = new QLabel(subtitle, this);

                m_titleLabel->setStyleSheet("font-weight: bold;");

                auto* vSizer = new QVBoxLayout();
                vSizer->addWidget(m_titleLabel);
                vSizer->addWidget(m_subtitleLabel);

                auto* hSizer = new QHBoxLayout();

                if (!image.isNull()) {
                    m_imageLabel = new QLabel(this);
                    m_imageLabel->setPixmap(image);
                    hSizer->addWidget(m_imageLabel);
                }
                hSizer->addLayout(vSizer);

                setLayout(hSizer);
            }
        };

        ControlListBox::ItemRenderer* ImageListBox::createItemRenderer(QWidget* parent, const size_t index) {
            return new ImageListBoxItemRenderer(title(index), subtitle(index), image(index), parent);
        }

        QPixmap ImageListBox::image(const size_t index) const {
            return QPixmap();
        }
    }
}
