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

#include <QHBoxLayout>
#include <QLabel>

namespace TrenchBroom
{
namespace View
{
AboutDialog* AboutDialog::instance = nullptr;

void AboutDialog::showAboutDialog()
{
  if (!AboutDialog::instance)
  {
    AboutDialog::instance = new AboutDialog{};
    AboutDialog::instance->show();
  }
  else
  {
    AboutDialog::instance->show();
    AboutDialog::instance->raise();
  }
}

void AboutDialog::closeAboutDialog()
{
  if (AboutDialog::instance)
  {
    AboutDialog::instance->close();
  }
}

AboutDialog::~AboutDialog()
{
  instance = nullptr;
}

AboutDialog::AboutDialog()
{
  // This makes it so the About dialog doesn't prevent the application from quitting
  setAttribute(Qt::WA_QuitOnClose, false);
  createGui();
}

void AboutDialog::createGui()
{
  const QString creditsString = tr(R"(
github.com/TrenchBroom/TrenchBroom<br />
<br />
<b>Developers</b><br />
<br />
Kristian Duske<br />
Eric Wasylishen<br />
<br />
<b>Contributors</b><br />
20kdc,
aapokaapo,
Amara M. Kilic,
Ari Vuollet,
bazhenovc,
chronicol,
Corey Jones,
iOrange,
Jonas Lund,
Jonathan Linat,
Josh Palmer,
mankeli,
Matthew Borkowski,
mittorn,
negke,
neogeographica,
Philipp Nahratow,
rebb,
Rohit Nirmal,
Scampie,
xaGe,
Yuki Raven
<br /><br />
<b>3rd Party Libraries, Tools and Assets</b><br />
Qt (Cross platform GUI library)<br />
FreeType (Font rendering library)<br />
FreeImage (Image loading & manipulation library)<br />
GLEW (OpenGL extension library)<br />
tinyxml2 (XML parsing library)<br />
miniz (Archive library)<br />
Assimp (Asset importer library)<br />
Catch 2 (C++ testing framework)<br />
StackWalker (C++ stack trace analyzer)<br />
CMake (Cross platform build manager)<br />
Vcpkg (C/C++ dependency manager)<br />
Pandoc (Universal document converter)<br />
Source Sans Pro (Font)<br />
Font Awesome 5 Free (Icons)<br />)");
  setWindowIconTB(this);

  auto* infoPanel = new AppInfoPanel{};
  auto* creditsText = new QLabel{creditsString};
  creditsText->setWordWrap(true);
  creditsText->setMaximumWidth(300);

  auto* layout = new QHBoxLayout{};
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setContentsMargins(0, 20, 0, 20);
  layout->addSpacing(50);
  layout->addWidget(infoPanel);
  layout->addSpacing(50);
  layout->addWidget(creditsText);
  layout->addSpacing(50);
  setLayout(layout);
}
} // namespace View
} // namespace TrenchBroom
