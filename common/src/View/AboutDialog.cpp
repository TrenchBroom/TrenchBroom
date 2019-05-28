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
#include "View/wxUtils.h"

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
                                             "Corey Jones (Documentation)\n"
                                             "Philipp Nahratow (Bug fixes, Linux builds)\n"
                                             "rebb (Shaders, bug fixes)\n"
                                             "chronicol (Documentation)\n"
                                             "bazhenovc (FreeImage texture loading)\n"
                                             "Scampie (Documentation)\n"
                                             "mittorn (Partial Half-Life support)\n"
                                             "Matthew Borkowski (CSG merging enhancements)\n"
                                             "Rohit Nirmal (Bug fixes)\n"
                                             "negke (FGD files)\n"
                                             "Jonathan Linat (Manual)\n"
                                             "Yuki Raven (Font size preference)\n"
                                             "mankeli (Bug fixes)\n"
                                             "Jonas Lund (Bug fixes)\n\n"
                                             "<b>3rd Party Libraries, Tools and Assets</b>\n"
                                             "wxWidgets (Cross platform GUI library)\n"
                                             "FreeType (Font rendering library)\n"
                                             "FreeImage (Image loading & manipulation library)\n"
                                             "GLEW (OpenGL extension library)\n"
                                             "tinyxml2 (XML parsing library)\n"
                                             "miniz (Archive library)\n"
                                             "optional-lite (C++ library)\n"
                                             "Google Test (C++ testing framework)\n"
                                             "Google Mock (C++ mocking framework)\n"
                                             "StackWalker (C++ stack trace analyzer)\n"
                                             "CMake (Cross platform build manager)\n"
                                             "Pandoc (Universal document converter)\n"
                                             "Source Sans Pro (Font)\n").replace("\n", "<br/>");

            AppInfoPanel* infoPanel = new AppInfoPanel(nullptr);

            QLabel* creditsText = new QLabel();
            creditsText->setText(creditsString);

            QHBoxLayout* outerSizer = new QHBoxLayout();
            outerSizer->setSizeConstraint(QLayout::SetFixedSize);
            outerSizer->setContentsMargins(0, 20, 0, 20);
            outerSizer->addSpacing(50);
            outerSizer->addWidget(infoPanel);
            outerSizer->addSpacing(50);
            outerSizer->addWidget(creditsText);
            outerSizer->addSpacing(50);
            setLayout(outerSizer);
        }
    }
}
