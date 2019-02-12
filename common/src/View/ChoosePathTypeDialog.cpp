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

#include "IO/SystemPaths.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QRadioButton>
#include <QLabel>
#include <QGridLayout>
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
            QLabel* absolutePathText = new QLabel(m_absPath.asQString());
            
            m_docRelativeRadio = new QRadioButton(tr("Relative to map file"));
            m_docRelativeRadio->setFont(boldFont);
            if (m_docRelativePath.isEmpty())
                m_docRelativeRadio->setEnabled(false);
            QLabel* mapRelativePathText = new QLabel(m_docRelativePath.isEmpty() ? tr("Could not build a path.") : m_docRelativePath.asQString());
            
            m_appRelativeRadio = new QRadioButton(tr("Relative to application executable"));
            m_appRelativeRadio->setFont(boldFont);
            if (m_appRelativePath.isEmpty())
                m_appRelativeRadio->setEnabled(false);
            QLabel* appRelativePathText = new QLabel(m_appRelativePath.isEmpty() ? tr("Could not build a path.") : m_appRelativePath.asQString());
            
            m_gameRelativeRadio = new QRadioButton(tr("Relative to game directory"));
            if (m_gameRelativePath.isEmpty())
                m_gameRelativeRadio->setEnabled(false);
            m_gameRelativeRadio->setFont(boldFont);
            QLabel* gameRelativePathText = new QLabel(m_gameRelativePath.isEmpty() ? tr("Could not build a path.") : m_gameRelativePath.asQString());

            auto* innerSizer = new QVBoxLayout();
            
            innerSizer->addWidget(infoText);

            innerSizer->addWidget(m_absRadio);
            innerSizer->addWidget(absolutePathText);

            innerSizer->addWidget(m_docRelativeRadio);
            innerSizer->addWidget(mapRelativePathText);

            innerSizer->addWidget(m_appRelativeRadio);
            innerSizer->addWidget(appRelativePathText);

            innerSizer->addWidget(m_gameRelativeRadio);
            innerSizer->addWidget(gameRelativePathText);

            auto* okCancelButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
            innerSizer->addWidget(okCancelButtons);
            
            setLayout(innerSizer);

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
