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

#include "TrenchBroomAppQt.h"

#include "Model/GameFactory.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapFrame.h"
#include "TrenchBroomApp.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QSettings>

extern void qt_set_sequence_auto_mnemonic(bool b);

int main(int argc, char *argv[])
{
    // Makes all QOpenGLWidget in the application share a single context
    // (default behaviour would be for QOpenGLWidget's in a single top-level window to share a context.)
    // see: http://doc.qt.io/qt-5/qopenglwidget.html#context-sharing
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    // We can't use auto mnemonics in TrenchBroom. e.g. by default with Qt, Alt+D opens the "Debug" menu,
    // Alt+S activates the "Show default properties" checkbox in the entity inspector.
    // Flying with Alt held down and pressing WASD is a fundamental behaviour in TB, so we can't have
    // shortcuts randomly activating.
    qt_set_sequence_auto_mnemonic(false);

    TrenchBroom::View::TrenchBroomApp app(argc, argv);

    app.exec();
}
