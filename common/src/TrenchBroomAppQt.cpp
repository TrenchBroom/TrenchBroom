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
#include "View/ActionManager.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapFrame.h"

#include <QApplication>
#include <QSurfaceFormat>

#include <wx/app.h>


TestWindow::TestWindow()
{
    QWidget* widget = new QWidget();
    setCentralWidget(widget);

    QMenuBar* menu = TrenchBroom::View::ActionManager::instance().createMenuBarQt(true);
    setMenuBar(menu);
}

int main(int argc, char *argv[])
{
    wxInitialize();

//    wxApp* pApp = new wxApp();
//    wxApp::SetInstance(pApp);

    // Makes all QOpenGLWidget in the application share a single context
    // (default behaviour would be for QOpenGLWidget's in a single top-level window to share a context.)
    // see: http://doc.qt.io/qt-5/qopenglwidget.html#context-sharing
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv);
//    TestWindow window;
//    window.show();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    using namespace TrenchBroom;
    using namespace TrenchBroom::View;

    MapDocumentSPtr document = MapDocumentCommandFacade::newMapDocument();
    MapFrame* frame = new MapFrame(nullptr, document);
    frame->show();

    String gameName = "Quake";
    auto mapFormat = Model::MapFormat::Standard;

    Model::GameFactory &gameFactory = Model::GameFactory::instance();
    Model::GameSPtr game = gameFactory.createGame(gameName, frame->logger());
    ensure(game.get() != nullptr, "game is null");

    frame->newDocument(game, mapFormat);

    app.exec();
}
