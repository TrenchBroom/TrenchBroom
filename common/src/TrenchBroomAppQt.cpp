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

#include "View/ActionManager.h"

#include <QApplication>

TestWindow::TestWindow()
{
    QWidget* widget = new QWidget();
    setCentralWidget(widget);

    QMenuBar* menu = TrenchBroom::View::ActionManager::instance().createMenuBarQt(true);
    setMenuBar(menu);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Makes all QOpenGLWidget in the application share a single context
    // (default behaviour would be for QOpenGLWidget's in a single top-level window to share a context.)
    // see: http://doc.qt.io/qt-5/qopenglwidget.html#context-sharing
    app.setAttribute(Qt::AA_ShareOpenGLContexts);

    TestWindow window;
    window.show();
    app.exec();
}
