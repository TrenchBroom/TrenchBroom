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

#include "PreferenceManager.h"
#include "TrenchBroomApp.h"
#include "Model/GameFactory.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"
#include "View/MapFrame.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QSettings>
#include <QtGlobal>

extern void qt_set_sequence_auto_mnemonic(bool b);

int main(int argc, char *argv[])
{
    // Set OpenGL defaults
    // Needs to be done here before QApplication is created
    // (see: https://doc.qt.io/qt-5/qsurfaceformat.html#setDefaultFormat)
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    // Makes all QOpenGLWidget in the application share a single context
    // (default behaviour would be for QOpenGLWidget's in a single top-level window to share a context.)
    // see: http://doc.qt.io/qt-5/qopenglwidget.html#context-sharing
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    // Set up Hi DPI scaling
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    // Enables non-integer scaling (e.g. 150% scaling on Windows)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    // Workaround bug in Qt's Ctrl+Click = RMB emulation (a macOS feature.)
    // In Qt 5.13.0 / macOS 10.14.6, Ctrl+trackpad click+Drag produces no mouse events at all, but
    // it should produce RMB down/move events.
    // This environment variable disables Qt's emulation so we can implement it ourselves in InputEventRecorder::recordEvent
    qputenv("QT_MAC_DONT_OVERRIDE_CTRL_LMB", "1");

    // Disable Qt OpenGL buglist; since we require desktop OpenGL 2.1 there's no point in
    // having Qt disable it (also we've had reports of some Intel drivers being blocked that
    // actually work with TB.)
    qputenv("QT_OPENGL_BUGLIST", ":/opengl_buglist.json");

    TrenchBroom::PreferenceManager::createInstance<TrenchBroom::AppPreferenceManager>();
    TrenchBroom::View::TrenchBroomApp app(argc, argv);

    app.parseCommandLineAndShowFrame();
    return app.exec();
}
