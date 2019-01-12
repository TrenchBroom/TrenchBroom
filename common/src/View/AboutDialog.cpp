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
            TrenchBroom::View::setWindowIcon(this);
            
            AppInfoPanel* infoPanel = new AppInfoPanel(nullptr);
            
            QLabel* creditsText = new QLabel();
            creditsText->setText(tr("<b>Developed by Kristian Duske</b><br>"
                                 "github.com/kduske/TrenchBroom<br><br>"
                                 "<b>Contributors</b><br>"
                                 "Corey Jones (Documentation)<br>"
                                 "Eric Wasylishen (Code, bug fixes)<br>"
                                 "Jonas Lund (Bug fixes)<br>"
                                 "negke (FGD files)<br>"
                                 "Philipp Nahratow (Bug fixes, Linux builds)<br>"
                                 "rebb (Shaders, bug fixes)<br>"
                                 "Rohit Nirmal (Bug fixes)<br>"
                                 "Scampie (Documentation)<br><br>"
                                 "<b>3rd Party Libraries, Tools and Assets</b><br>"
                                 "wxWidgets (Cross platform GUI library)<br>"
                                 "FreeType (Font rendering library)<br>"
                                 "FreeImage (Image loading & manipulation library)<br>"
                                 "GLEW (OpenGL extension library)<br>"
                                 "Google Test (C++ testing framework)<br>"
                                 "Google Mock (C++ mocking framework)<br>"
                                 "StackWalker (C++ stack trace analyzer)<br>"
                                 "CMake (Cross platform build manager)<br>"
                                 "Pandoc (Universal document converter)<br>"
                                 "Source Sans Pro (Font)<br>"));

            QHBoxLayout* outerSizer = new QHBoxLayout();
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
