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

#include "GameEngineProfileEditor.h"

#include "Model/GameEngineProfile.h"
#include "IO/DiskIO.h"
#include "IO/PathQt.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/set_temp.h>
#include <kdl/string_compare.h>

#include <QBoxLayout>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>

namespace TrenchBroom {
    namespace View {
        GameEngineProfileEditor::GameEngineProfileEditor(QWidget* parent) :
        QWidget(parent),
        m_profile(nullptr),
        m_stackedWidget(nullptr),
        m_nameEdit(nullptr),
        m_pathEdit(nullptr),
        m_ignoreNotifications(false) {
            m_stackedWidget = new QStackedWidget();
            m_stackedWidget->addWidget(createDefaultPage("Select a game engine profile"));
            m_stackedWidget->addWidget(createEditorPage());

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(QMargins());
            setLayout(layout);
            layout->addWidget(m_stackedWidget);
        }

        GameEngineProfileEditor::~GameEngineProfileEditor() {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &GameEngineProfileEditor::profileDidChange);
            }
        }

        QWidget* GameEngineProfileEditor::createEditorPage() {
            auto* container = new QWidget();
            setBaseWindowColor(container);

            m_nameEdit = new QLineEdit();
            setHint(m_nameEdit, "Choose a name");

            m_pathEdit = new QLineEdit();
            setHint(m_pathEdit, "Click on the button to choose...");

            auto* button = new QPushButton("...");

            connect(m_nameEdit, &QLineEdit::textEdited, this, &GameEngineProfileEditor::nameChanged);
            connect(m_pathEdit, &QLineEdit::editingFinished, this, &GameEngineProfileEditor::pathChanged);
            connect(button, &QPushButton::clicked, this, &GameEngineProfileEditor::changePathClicked);

            auto* pathLayout = new QHBoxLayout();
            pathLayout->addWidget(m_pathEdit, 1);
            pathLayout->addWidget(button);

            auto* formLayout = new QFormLayout();
            formLayout->setContentsMargins(LayoutConstants::WideHMargin, LayoutConstants::WideVMargin, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
            formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            container->setLayout(formLayout);

            formLayout->addRow("Name", m_nameEdit);
            formLayout->addRow("Path", pathLayout);

            return container;
        }

        void GameEngineProfileEditor::updatePath(const QString& str) {
            const auto valid = isValidEnginePath(str);
            if (valid) {
                const auto path = IO::pathFromQString(str);
                m_profile->setPath(path);
                if (m_profile->name().empty()) {
                    m_profile->setName(path.lastComponent().deleteExtension().asString());
                }
                refresh();
            }

            if (valid || str.isEmpty()) {
                makeDefault(m_pathEdit);
            } else {
                makeError(m_pathEdit);
            }
        }

        void GameEngineProfileEditor::setProfile(Model::GameEngineProfile* profile) {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &GameEngineProfileEditor::profileDidChange);
            }
            m_profile = profile;
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.addObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &GameEngineProfileEditor::profileDidChange);
                m_stackedWidget->setCurrentIndex(1);
            } else {
                m_stackedWidget->setCurrentIndex(0);
            }
            refresh();
        }

        void GameEngineProfileEditor::profileWillBeRemoved() {
            setProfile(nullptr);
        }

        void GameEngineProfileEditor::profileDidChange() {
            refresh();
        }

        void GameEngineProfileEditor::refresh() {
            if (m_profile != nullptr && !m_ignoreNotifications) {
                m_nameEdit->setText(QString::fromStdString(m_profile->name()));
                m_pathEdit->setText(IO::pathAsQString(m_profile->path()));
            }
        }

        bool GameEngineProfileEditor::isValidEnginePath(const QString& str) const {
            try {
                const auto path = IO::pathFromQString(str);
                return IO::Disk::fileExists(path)
#ifdef __APPLE__
                || (IO::Disk::directoryExists(path) && kdl::ci::str_is_equal(path.extension(), "app"))
#endif
                ;
            } catch (...) {
                return false;
            }
        }

        void GameEngineProfileEditor::nameChanged(const QString& text) {
            ensure(m_profile != nullptr, "profile is null");

            const kdl::set_temp ignore(m_ignoreNotifications);
            m_profile->setName(text.toStdString());
        }

        void GameEngineProfileEditor::pathChanged() {
            ensure(m_profile != nullptr, "profile is null");

            const kdl::set_temp ignore(m_ignoreNotifications);
            updatePath(m_pathEdit->text());
        }

        void GameEngineProfileEditor::changePathClicked() {
            const QString pathStr = QFileDialog::getOpenFileName(this, tr("Choose Engine"), fileDialogDefaultDirectory(FileDialogDir::Engine));
            if (!pathStr.isEmpty()) {
                updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::Engine, pathStr);
                updatePath(pathStr);
            }
        }
    }
}
