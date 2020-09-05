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

#include "ChoosePathTypeDialog.h"

#include "IO/PathQt.h"
#include "IO/SystemPaths.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QRadioButton>
#include <QLabel>
#include <QDialogButtonBox>

namespace TrenchBroom {
    namespace View {
        ChoosePathTypeDialog::ChoosePathTypeDialog() :
        QDialog(nullptr),
        m_absPath(""),
        m_docRelativePath(""),
        m_gameRelativePath(""),
        m_appRelativePath("") {
            createGui();
        }

        ChoosePathTypeDialog::ChoosePathTypeDialog(QWidget* parent, const IO::Path& absPath, const IO::Path& docPath, const IO::Path& gamePath) :
        QDialog(parent),
        m_absPath(absPath),
        m_docRelativePath(makeRelativePath(absPath, docPath.deleteLastComponent())),
        m_gameRelativePath(makeRelativePath(absPath, gamePath)),
        m_appRelativePath(makeRelativePath(absPath, IO::SystemPaths::appDirectory())) {
            createGui();
        }

        void ChoosePathTypeDialog::createGui() {
            setWindowTitle(tr("Path Type"));
            setWindowIconTB(this);

            QLabel* infoText = new QLabel(tr("Paths can be stored either as absolute paths or as relative paths. Please choose how you want to store this path."));
            infoText->setMaximumWidth(370);
            infoText->setWordWrap(true);

            m_absRadio = new QRadioButton(tr("Absolute"));
            QFont boldFont = m_absRadio->font();
            boldFont.setBold(true);
            m_absRadio->setFont(boldFont);
            m_absRadio->setChecked(true);
            QLabel* absolutePathText = new QLabel(IO::pathAsQString(m_absPath));

            m_docRelativeRadio = new QRadioButton(tr("Relative to map file"));
            m_docRelativeRadio->setFont(boldFont);
            if (m_docRelativePath.isEmpty())
                m_docRelativeRadio->setEnabled(false);
            QLabel* mapRelativePathText = new QLabel(m_docRelativePath.isEmpty() ? tr("Could not build a path.") : IO::pathAsQString(m_docRelativePath));

            m_appRelativeRadio = new QRadioButton(tr("Relative to application executable"));
            m_appRelativeRadio->setFont(boldFont);
            if (m_appRelativePath.isEmpty())
                m_appRelativeRadio->setEnabled(false);
            QLabel* appRelativePathText = new QLabel(m_appRelativePath.isEmpty() ? tr("Could not build a path.") : IO::pathAsQString(m_appRelativePath));

            m_gameRelativeRadio = new QRadioButton(tr("Relative to game directory"));
            if (m_gameRelativePath.isEmpty())
                m_gameRelativeRadio->setEnabled(false);
            m_gameRelativeRadio->setFont(boldFont);
            QLabel* gameRelativePathText = new QLabel(m_gameRelativePath.isEmpty() ? tr("Could not build a path.") : IO::pathAsQString(m_gameRelativePath));

            auto* innerLayout = new QVBoxLayout();
            innerLayout->setContentsMargins(LayoutConstants::WideHMargin, LayoutConstants::WideVMargin, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            innerLayout->setSpacing(0);

            innerLayout->addWidget(infoText);
            innerLayout->addSpacing(LayoutConstants::WideVMargin);

            innerLayout->addWidget(m_absRadio);
            innerLayout->addWidget(absolutePathText);
            innerLayout->addSpacing(LayoutConstants::WideVMargin);

            innerLayout->addWidget(m_docRelativeRadio);
            innerLayout->addWidget(mapRelativePathText);
            innerLayout->addSpacing(LayoutConstants::WideVMargin);

            innerLayout->addWidget(m_appRelativeRadio);
            innerLayout->addWidget(appRelativePathText);
            innerLayout->addSpacing(LayoutConstants::WideVMargin);

            innerLayout->addWidget(m_gameRelativeRadio);
            innerLayout->addWidget(gameRelativePathText);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            outerLayout->setSpacing(LayoutConstants::MediumVMargin);
            outerLayout->addLayout(innerLayout);

            auto* okCancelButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            outerLayout->addLayout(wrapDialogButtonBox(okCancelButtons));

            insertTitleBarSeparator(outerLayout);

            setLayout(outerLayout);

            connect(okCancelButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(okCancelButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
       }

        const IO::Path& ChoosePathTypeDialog::path() const {
            if (m_docRelativeRadio->isChecked())
                return m_docRelativePath;
            if (m_appRelativeRadio->isChecked())
                return m_appRelativePath;
            if (m_gameRelativeRadio->isChecked())
                return m_gameRelativePath;
            return m_absPath;
        }

        IO::Path ChoosePathTypeDialog::makeRelativePath(const IO::Path& absPath, const IO::Path& newRootPath) {
            if (!newRootPath.canMakeRelative(absPath))
                return IO::Path("");
            return newRootPath.makeRelative(absPath);
        }
    }
}
