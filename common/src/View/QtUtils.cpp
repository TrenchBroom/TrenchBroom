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

#include "QtUtils.h"

#include "Color.h"

#include "Ensure.h"
#include "Macros.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/BorderLine.h"
#include "View/MapFrame.h"
#include "View/MapTextEncoding.h"
#include "View/ViewConstants.h"

#include <QtGlobal>
#include <QAbstractButton>
#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QColor>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFont>
#include <QKeySequence>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QSettings>
#include <QResizeEvent>
#include <QScreen>
#include <QString>
#include <QStringBuilder>
#include <QStandardPaths>
#include <QTableView>
#include <QTextCodec>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>

// QDesktopWidget was deprecated in Qt 5.10 and we should use QGuiApplication::screenAt in 5.10 and above
// Used in centerOnScreen
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QGuiApplication>
#else
#include <QApplication>
#include <QDesktopWidget>
#endif


namespace TrenchBroom {
    namespace View {
        DisableWindowUpdates::DisableWindowUpdates(QWidget* widget) :
        m_widget(widget) {
            m_widget->setUpdatesEnabled(false);
        }

        DisableWindowUpdates::~DisableWindowUpdates() {
            m_widget->setUpdatesEnabled(true);
        }

        SyncHeightEventFilter::SyncHeightEventFilter(QWidget* primary, QWidget* secondary, QObject* parent) :
        QObject(parent),
        m_primary(primary),
        m_secondary(secondary) {
            ensure(m_primary != nullptr, "primary is not null");
            ensure(m_secondary != nullptr, "secondary is not null");
            
            m_primary->installEventFilter(this);
        }
        
        SyncHeightEventFilter::~SyncHeightEventFilter() {
            if (m_primary) {
                m_primary->removeEventFilter(this);
            }
        }

        bool SyncHeightEventFilter::eventFilter(QObject* target, QEvent* event) {
            if (target == m_primary && event->type() == QEvent::Resize) {
                const auto* sizeEvent = static_cast<QResizeEvent*>(event);
                const auto height = sizeEvent->size().height();
                if (m_secondary->height() != height) {
                    m_secondary->setFixedHeight(height);
                }
                return false;
            } else {
                return QObject::eventFilter(target, event);
            }
        }

        static QString fileDialogDirToString(const FileDialogDir dir) {
            switch (dir) {
                case FileDialogDir::Map: return "Map";
                case FileDialogDir::TextureCollection: return "TextureCollection";
                case FileDialogDir::CompileTool: return "CompileTool";
                case FileDialogDir::Engine: return "Engine";
                case FileDialogDir::EntityDefinition: return "EntityDefinition";
                case FileDialogDir::GamePath: return "GamePath";
                switchDefault()
            }
        }

        static QString fileDialogDefaultDirectorySettingsPath(const FileDialogDir dir) {
            return QString::fromLatin1("FileDialog/%1/DefaultDirectory")
                .arg(fileDialogDirToString(dir));
        }

        QString fileDialogDefaultDirectory(const FileDialogDir dir) {
            const QString key = fileDialogDefaultDirectorySettingsPath(dir);

            const QSettings settings;
            const QString defaultDir = settings.value(key).toString();
            return defaultDir;
        }

        void updateFileDialogDefaultDirectoryWithFilename(FileDialogDir type, const QString& filename) {
            const QDir dirQDir = QFileInfo(filename).absoluteDir();
            const QString dirString = dirQDir.absolutePath();
            updateFileDialogDefaultDirectoryWithDirectory(type, dirString);
        }

        void updateFileDialogDefaultDirectoryWithDirectory(FileDialogDir type, const QString& newDefaultDirectory) {
            const QString key = fileDialogDefaultDirectorySettingsPath(type);

            QSettings settings;
            settings.setValue(key, newDefaultDirectory);
        }

        QString windowSettingsPath(const QWidget* window, const QString& suffix) {
            ensure(window != nullptr, "window must not be null");
            ensure(!window->objectName().isEmpty(), "window name must not be empty");

            return "Windows/" + window->objectName() + "/" + suffix;
        }

        void saveWindowGeometry(QWidget* window) {
            ensure(window != nullptr, "window must not be null");

            const auto path = windowSettingsPath(window, "Geometry");
            QSettings settings;
            settings.setValue(path, window->saveGeometry());
        }

        void restoreWindowGeometry(QWidget* window) {
            ensure(window != nullptr, "window must not be null");

            const auto path = windowSettingsPath(window, "Geometry");
            const QSettings settings;
            window->restoreGeometry(settings.value(path).toByteArray());
        }

        bool widgetOrChildHasFocus(const QWidget* widget) {
            ensure(widget != nullptr, "widget must not be null");
            
            const QObject* currentWidget = QApplication::focusWidget();
            while (currentWidget != nullptr) {
                if (currentWidget == widget) {
                    return true;
                }
                currentWidget = currentWidget->parent();
            }
            return false;
        }

        MapFrame* findMapFrame(QWidget* widget) {
            return dynamic_cast<MapFrame*>(widget->window());
        }

        void setHint(QLineEdit* ctrl, const char* hint) {
            ctrl->setPlaceholderText(hint);
        }

        void centerOnScreen(QWidget* window) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            const auto* screen = QGuiApplication::screenAt(window->mapToGlobal({ window->width() / 2, 0 }));
            if (screen == nullptr) {
                return;
            }
            const auto screenGeometry = screen->availableGeometry();
#else
            const auto screenGeometry = QApplication::desktop()->availableGeometry(window);
#endif
            window->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    window->size(),
                    screenGeometry));
        }

        QWidget* makeDefault(QWidget* widget) {
            widget->setFont(QFont());
            widget->setPalette(QPalette());
            return widget;
        }

        QWidget* makeEmphasized(QWidget* widget) {
            auto font = widget->font();
            font.setBold(true);
            widget->setFont(font);
            return widget;
        }

        QWidget* makeUnemphasized(QWidget* widget) {
            widget->setFont(QFont());
            return widget;
        }

        QWidget* makeInfo(QWidget* widget) {
            makeDefault(widget);

            widget = makeSmall(widget);

            const auto defaultPalette = QPalette();
            auto palette = widget->palette();
            // Set all color groups (active, inactive, disabled) to use the disabled color, so it's dimmer
            palette.setColor(QPalette::WindowText, defaultPalette.color(QPalette::Disabled, QPalette::WindowText));
            palette.setColor(QPalette::Text, defaultPalette.color(QPalette::Disabled, QPalette::Text));
            widget->setPalette(palette);
            return widget;
        }

        QWidget* makeSmall(QWidget* widget) {
            widget->setAttribute(Qt::WA_MacSmallSize);
            return widget;
        }

        QWidget* makeHeader(QWidget* widget) {
            makeDefault(widget);

            auto font = widget->font();
            font.setPointSize(2 * font.pointSize());
            font.setBold(true);
            widget->setFont(font);
            return widget;
        }

        QWidget* makeError(QWidget* widget) {
            auto palette = widget->palette();
            palette.setColor(QPalette::Normal, QPalette::WindowText, Qt::red);
            palette.setColor(QPalette::Normal, QPalette::Text, Qt::red);
            widget->setPalette(palette);
            return widget;
        }

        QWidget* makeSelected(QWidget* widget, const QPalette& defaultPalette) {
            auto palette = widget->palette();
            palette.setColor(QPalette::Normal, QPalette::WindowText, defaultPalette.color(QPalette::Normal, QPalette::HighlightedText));
            palette.setColor(QPalette::Normal, QPalette::Text, defaultPalette.color(QPalette::Normal, QPalette::HighlightedText));
            widget->setPalette(palette);
            return widget;
        }

        QWidget* makeUnselected(QWidget* widget, const QPalette& defaultPalette) {
            auto palette = widget->palette();
            palette.setColor(QPalette::Normal, QPalette::WindowText, defaultPalette.color(QPalette::Normal, QPalette::WindowText));
            palette.setColor(QPalette::Normal, QPalette::Text, defaultPalette.color(QPalette::Normal, QPalette::Text));
            widget->setPalette(palette);
            return widget;
        }

        Color fromQColor(const QColor& color) {
            return Color(static_cast<float>(color.redF()),
                         static_cast<float>(color.greenF()),
                         static_cast<float>(color.blueF()),
                         static_cast<float>(color.alphaF()));
        }

        QColor toQColor(const Color& color) {
            return QColor::fromRgb(int(color.r() * 255.0f), int(color.g() * 255.0f), int(color.b() * 255.0f), int(color.a() * 255.0f));
        }

        QAbstractButton* createBitmapButton(const std::string& image, const QString& tooltip, QWidget* parent) {
            return createBitmapButton(loadSVGIcon(IO::Path(image)), tooltip, parent);
        }

        QAbstractButton* createBitmapButton(const QIcon& icon, const QString& tooltip, QWidget* parent) {
            // NOTE: QIcon::availableSizes() is not high-dpi friendly, it returns pixels when we want logical sizes.
            // We rely on the fact that loadIconResourceQt inserts pixmaps in the order 1x then 2x, so the first
            // pixmap has the logical size.
            ensure(!icon.availableSizes().empty(), "expected a non-empty icon. Fails when the image file couldn't be found.");

            // NOTE: according to http://doc.qt.io/qt-5/qpushbutton.html this would be more correctly
            // be a QToolButton, but the QToolButton doesn't have a flat style on macOS
            auto* button = new QToolButton(parent);
            button->setMinimumSize(icon.availableSizes().front());
            // button->setAutoDefault(false);
            button->setToolTip(tooltip);
            button->setIcon(icon);
            // button->setFlat(true);
            button->setObjectName("toolButton_borderless");

            return button;
        }

        QAbstractButton* createBitmapToggleButton(const std::string& image, const QString& tooltip, QWidget* parent) {
            auto* button = createBitmapButton(image, tooltip, parent);
            button->setCheckable(true);
            return button;
        }

        QWidget* createDefaultPage(const QString& message, QWidget* parent) {
            auto* container = new QWidget(parent);
            auto* layout = new QVBoxLayout();

            auto* messageLabel = new QLabel(message);
            makeEmphasized(messageLabel);
            layout->addWidget(messageLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
            container->setLayout(layout);

            return container;
        }

        QSlider* createSlider(const int min, const int max) {
            auto* slider = new QSlider();
            slider->setMinimum(min);
            slider->setMaximum(max);
            slider->setTickPosition(QSlider::TicksBelow);
            slider->setTracking(true);
            slider->setOrientation(Qt::Horizontal);
            return slider;
        }

        float getSliderRatio(const QSlider* slider) {
            return float(slider->value() - slider->minimum()) / float(slider->maximum() - slider->minimum());
        }

        void setSliderRatio(QSlider* slider, float ratio) {
            const auto value = ratio * float(slider->maximum() - slider->minimum()) + float(slider->minimum());
            slider->setValue(int(value));
        }

        QLayout* wrapDialogButtonBox(QWidget* buttonBox) {
            auto* innerLayout = new QHBoxLayout();
            innerLayout->setContentsMargins(
                LayoutConstants::DialogButtonLeftMargin,
                LayoutConstants::DialogButtonTopMargin,
                LayoutConstants::DialogButtonRightMargin,
                LayoutConstants::DialogButtonBottomMargin);
            innerLayout->setSpacing(0);
            innerLayout->addWidget(buttonBox);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(QMargins());
            outerLayout->setSpacing(0);
            outerLayout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            outerLayout->addLayout(innerLayout);

            return outerLayout;
        }

        QLayout* wrapDialogButtonBox(QLayout* buttonBox) {
            auto* innerLayout = new QHBoxLayout();
            innerLayout->setContentsMargins(
                LayoutConstants::DialogButtonLeftMargin,
                LayoutConstants::DialogButtonTopMargin,
                LayoutConstants::DialogButtonRightMargin,
                LayoutConstants::DialogButtonBottomMargin);
            innerLayout->setSpacing(0);
            innerLayout->addLayout(buttonBox);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(QMargins());
            outerLayout->setSpacing(0);
            outerLayout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            outerLayout->addLayout(innerLayout);

            return outerLayout;
        }

        void addToMiniToolBarLayout(QBoxLayout*) {}

        void setWindowIconTB(QWidget* window) {
            ensure(window != nullptr, "window is null");
            window->setWindowIcon(QIcon(IO::loadPixmapResource(IO::Path("AppIcon.png"))));
        }

        void setDebugBackgroundColor(QWidget* widget, const QColor& color) {
            QPalette p = widget->palette();
            p.setColor(QPalette::Window, color);

            widget->setAutoFillBackground(true);
            widget->setPalette(p);
        }

        void setDefaultWindowColor(QWidget* widget) {
            widget->setAutoFillBackground(true);
            widget->setBackgroundRole(QPalette::Window);
        }

        void setBaseWindowColor(QWidget* widget) {
            widget->setAutoFillBackground(true);
            widget->setBackgroundRole(QPalette::Base);
        }

        void setHighlightWindowColor(QWidget* widget) {
            widget->setAutoFillBackground(true);
            widget->setBackgroundRole(QPalette::Highlight);
        }

        QLineEdit* createSearchBox() {
            auto* widget = new QLineEdit();
            widget->setClearButtonEnabled(true);
            widget->setPlaceholderText(QLineEdit::tr("Search..."));

            QIcon icon = loadSVGIcon(IO::Path("Search.svg"));
            widget->addAction(icon, QLineEdit::LeadingPosition);
            return widget;
        }

        void checkButtonInGroup(QButtonGroup* group, const int id, const bool checked) {
            QAbstractButton* button = group->button(id);
            if (button == nullptr) {
                return;
            }
            button->setChecked(checked);
        }

        void checkButtonInGroup(QButtonGroup* group, const QString& objectName, bool checked) {
            for (QAbstractButton* button : group->buttons()) {
                if (button->objectName() == objectName) {
                    button->setChecked(checked);
                    return;
                }
            }
        }

        void insertTitleBarSeparator(QVBoxLayout* layout) {
#ifdef _WIN32
            layout->insertWidget(0, new BorderLine(), 1);
#endif
            unused(layout);
        }

        AutoResizeRowsEventFilter::AutoResizeRowsEventFilter(QTableView* tableView) :
        QObject(tableView),
        m_tableView(tableView) {
            m_tableView->installEventFilter(this);
        }

        bool AutoResizeRowsEventFilter::eventFilter(QObject* watched, QEvent* event) {
            if (watched == m_tableView && event->type() == QEvent::Show) {
                m_tableView->resizeRowsToContents();
                m_tableView->removeEventFilter(this);
            }
            return QObject::eventFilter(watched, event);
        }

        void autoResizeRows(QTableView* tableView) {
            tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            tableView->installEventFilter(new AutoResizeRowsEventFilter(tableView));
            tableView->resizeRowsToContents();
        }

        void deleteChildWidgetsLaterAndDeleteLayout(QWidget* widget) {
            const QList<QWidget*> children = widget->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly);
            for (QWidget* childWidget : children) {
                childWidget->deleteLater();
            }

            delete widget->layout();
        }

        void clearLayout(QLayout* layout) {
            // https://doc.qt.io/qt-5/qlayout.html#takeAt
            QLayoutItem* child;
            while ((child = layout->takeAt(0)) != nullptr) {
                delete child->widget();
                delete child;
            }
        }

        void showModelessDialog(QDialog* dialog) {
            // https://doc.qt.io/qt-5/qdialog.html#code-examples
            dialog->show();
            dialog->raise();
            dialog->activateWindow();
        }

        static QTextCodec* codecForEncoding(const MapTextEncoding encoding) {
            switch (encoding) {
            case MapTextEncoding::Quake:
                // Quake uses the full 1-255 range for its bitmap font.
                // So using a "just assume UTF-8" approach would not work here.
                // See: https://github.com/TrenchBroom/TrenchBroom/issues/3122
                return QTextCodec::codecForLocale();
            case MapTextEncoding::Iso88591:
                return QTextCodec::codecForName("ISO 8859-1");
            case MapTextEncoding::Utf8:
                return QTextCodec::codecForName("UTF-8");
            switchDefault()
            }
        }

        QString mapStringToUnicode(const MapTextEncoding encoding, const std::string& string) {
            QTextCodec* codec = codecForEncoding(encoding);
            ensure(codec != nullptr, "null codec");

            return codec->toUnicode(QByteArray::fromStdString(string));
        }

        std::string mapStringFromUnicode(const MapTextEncoding encoding, const QString& string) {
            QTextCodec* codec = codecForEncoding(encoding);
            ensure(codec != nullptr, "null codec");

            return codec->fromUnicode(string).toStdString();
        }

        QString nativeModifierLabel(const int modifier) {
            assert(modifier == Qt::META || modifier == Qt::SHIFT
                || modifier == Qt::CTRL || modifier == Qt::ALT);

            const auto keySequence = QKeySequence(modifier);

            // QKeySequence doesn't totally support being given just a modifier
            // but it does seem to handle the key codes like Qt::SHIFT, which
            // it turns into native text as "Shift+" or the Shift symbol on macOS,
            // and portable text as "Shift+".

            QString nativeLabel = keySequence.toString(QKeySequence::NativeText);
            if (nativeLabel.endsWith("+")) {
                // On Linux we get nativeLabel as something like "Ctrl+"
                // On macOS it's just the special Command character, with no +
                nativeLabel.chop(1); // Remove last character
            }

            return nativeLabel;
        }
    }
}
