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

#include "CompilationProfileListBox.h"

#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "View/ElidedLabel.h"
#include "View/QtUtils.h"

#include <QBoxLayout>

namespace TrenchBroom {
    namespace View {
        CompilationProfileItemRenderer::CompilationProfileItemRenderer(Model::CompilationProfile& profile, QWidget* parent) :
        ControlListBoxItemRenderer(parent),
        m_profile(&profile),
        m_nameText(nullptr),
        m_taskCountText(nullptr) {
            setContextMenuPolicy(Qt::CustomContextMenu); // request customContextMenuRequested() to be emitted

            m_nameText = new ElidedLabel("", Qt::ElideRight);
            m_taskCountText = new ElidedLabel("", Qt::ElideMiddle);

            makeEmphasized(m_nameText);
            makeInfo(m_taskCountText);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(QMargins());
            layout->setSpacing(0);
            layout->addWidget(m_nameText);
            layout->addWidget(m_taskCountText);

            setLayout(layout);

            addObservers();
        }

        CompilationProfileItemRenderer::~CompilationProfileItemRenderer() {
            if (m_profile != nullptr) {
                removeObservers();
            }
        }

        void CompilationProfileItemRenderer::addObservers() {
            m_profile->profileWillBeRemoved.addObserver(this, &CompilationProfileItemRenderer::profileWillBeRemoved);
            m_profile->profileDidChange.addObserver(this, &CompilationProfileItemRenderer::profileDidChange);
        }

        void CompilationProfileItemRenderer::removeObservers() {
            m_profile->profileWillBeRemoved.removeObserver(this, &CompilationProfileItemRenderer::profileWillBeRemoved);
            m_profile->profileDidChange.removeObserver(this, &CompilationProfileItemRenderer::profileDidChange);
        }

        void CompilationProfileItemRenderer::profileWillBeRemoved() {
            if (m_profile != nullptr) {
                removeObservers();
                m_profile = nullptr;
            }
        }

        void CompilationProfileItemRenderer::profileDidChange() {
            updateItem();
        }

        void CompilationProfileItemRenderer::updateItem() {
            if (m_profile == nullptr) {
                m_nameText->setText("");
                m_taskCountText->setText("");
            } else {
                m_nameText->setText(QString::fromStdString(m_profile->name()));
                m_taskCountText->setText(QString::number(m_profile->taskCount()) + " tasks");
            }
        }

        CompilationProfileListBox::CompilationProfileListBox(const Model::CompilationConfig& config, QWidget* parent) :
        ControlListBox("Click the '+' button to create a compilation profile.", true, parent),
        m_config(config) {
            m_config.profilesDidChange.addObserver(this, &CompilationProfileListBox::profilesDidChange);
            reload();
        }

        CompilationProfileListBox::~CompilationProfileListBox() {
            m_config.profilesDidChange.removeObserver(this, &CompilationProfileListBox::profilesDidChange);
        }

        void CompilationProfileListBox::profilesDidChange() {
            reload();
        }

        size_t CompilationProfileListBox::itemCount() const {
            return m_config.profileCount();
        }

        ControlListBoxItemRenderer* CompilationProfileListBox::createItemRenderer(QWidget* parent, const size_t index) {
            auto* profile = m_config.profile(index);
            auto* renderer = new CompilationProfileItemRenderer(*profile, parent);
            connect(renderer, &QWidget::customContextMenuRequested, this, [=](const QPoint& pos){
                emit this->profileContextMenuRequested(renderer->mapToGlobal(pos), profile);
            });
            return renderer;
        }
    }
}
