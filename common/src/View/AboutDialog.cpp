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

#include "AboutDialog.h"

#include "View/AppInfoPanel.h"
#include "View/QtUtils.h"

#include <QLabel>
#include <QHBoxLayout>

namespace TrenchBroom {
    namespace View {
        AboutDialog* AboutDialog::instance = nullptr;

        void AboutDialog::showAboutDialog() {
            if (AboutDialog::instance == nullptr) {
                AboutDialog::instance = new AboutDialog();
                AboutDialog::instance->show();
            } else {
                AboutDialog::instance->show();
                AboutDialog::instance->raise();
            }
        }

        void AboutDialog::closeAboutDialog() {
            if (AboutDialog::instance != nullptr)
                AboutDialog::instance->close();
        }

        AboutDialog::~AboutDialog() {
            instance = nullptr;
        }

        AboutDialog::AboutDialog() :
        QDialog() {
            createGui();
        }

        void AboutDialog::createGui() {
            const QString creditsString = tr("github.com/kduske/TrenchBroom\n\n"
                                             "<b>Developers</b>\n"
                                             "Kristian Duske\n"
                                             "Eric Wasylishen\n\n"
                                             "<b>Contributors</b>\n"
                                             "20kdc, "
                                             "aapokaapo, "
                                             "Ari Vuollet, "
                                             "bazhenovc, "
                                             "chronicol, "
                                             "Corey Jones, "
                                             "Jonas Lund, "
                                             "Jonathan Linat, "
                                             "Josh Palmer, "
                                             "mankeli, "
                                             "Matthew Borkowski, "
                                             "mittorn, "
                                             "negke, "
                                             "Philipp Nahratow, "
                                             "rebb, "
                                             "Rohit Nirmal, "
                                             "Scampie, "
                                             "Yuki Raven\n\n"
                                             "<b>3rd Party Libraries, Tools and Assets</b>\n"
                                             "Qt (Cross platform GUI library)\n"
                                             "FreeType (Font rendering library)\n"
                                             "FreeImage (Image loading & manipulation library)\n"
                                             "GLEW (OpenGL extension library)\n"
                                             "tinyxml2 (XML parsing library)\n"
                                             "miniz (Archive library)\n"
                                             "any-lite (C++ library)\n"
                                             "optional-lite (C++ library)\n"
                                             "Google Test (C++ testing framework)\n"
                                             "Google Mock (C++ mocking framework)\n"
                                             "StackWalker (C++ stack trace analyzer)\n"
                                             "CMake (Cross platform build manager)\n"
                                             "Pandoc (Universal document converter)\n"
                                             "Source Sans Pro (Font)\n").replace("\n", "<br/>");
            setWindowIconTB(this);

            auto* infoPanel = new AppInfoPanel();
            auto* creditsText = new QLabel(creditsString);
            creditsText->setWordWrap(true);
            creditsText->setMaximumWidth(300);

            auto* layout = new QHBoxLayout();
            layout->setSizeConstraint(QLayout::SetFixedSize);
            layout->setContentsMargins(0, 20, 0, 20);
            layout->addSpacing(50);
            layout->addWidget(infoPanel);
            layout->addSpacing(50);
            layout->addWidget(creditsText);
            layout->addSpacing(50);
            setLayout(layout);

        }
    }
}
