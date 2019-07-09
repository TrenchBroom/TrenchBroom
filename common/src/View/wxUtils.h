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

#ifndef TrenchBroom_wxUtils
#define TrenchBroom_wxUtils

#undef CursorShape
#include <QString>

#include "Color.h"
#include "StringUtils.h"

#include "View/ViewConstants.h"

#include <QBoxLayout>
#include <QColor>
#include <QSettings>
#include <QWidget>

#include <vector>

class QAbstractButton;
class QCompleter;
class QDialogButtonBox;
class QFont;
class QLayout;
class QLineEdit;
class QMainWindow;
class QPalette;
class QSettings;
class QSlider;
class QSplitter;
class QWidget;
class QButtonGroup;

namespace TrenchBroom {
    namespace View {
        class DisableWindowUpdates {
        private:
            QWidget* m_widget;
        public:
            explicit DisableWindowUpdates(QWidget* widget);
            ~DisableWindowUpdates();
        };

        QString windowSettingsPath(const QWidget* window, const QString& suffix = "");

        void saveWindowGeometry(QWidget* window);
        void restoreWindowGeometry(QWidget* window);

        template <typename T>
        void saveWindowState(const T* window) {
            ensure(window != nullptr, "window must not be null");

            const auto path = windowSettingsPath(window, "State");
            QSettings settings;
            settings.setValue(path, window->saveState());
        }

        template <typename T>
        void restoreWindowState(T* window) {
            ensure(window != nullptr, "window must not be null");

            const auto path = windowSettingsPath(window, "State");
            QSettings settings;
            window->restoreState(settings.value(path).toByteArray());
        }

        class MapFrame;
        MapFrame* findMapFrame(QWidget* widget);
#if 0
        wxFrame* findFrame(QWidget* window);

        void fitAll(QWidget* window);

        wxColor makeLighter(const wxColor& color);
        Color fromWxColor(const wxColor& color);
        wxColor toWxColor(const Color& color);

        std::vector<size_t> getListCtrlSelection(const wxListCtrl* listCtrl);
        void deselectAllListrCtrlItems(wxListCtrl* listCtrl);
#endif

        QAbstractButton* createBitmapButton(const String& image, const QString& tooltip, QWidget* parent = nullptr);
        QAbstractButton* createBitmapButton(const QIcon& icon, const QString& tooltip, QWidget* parent = nullptr);
        QAbstractButton* createBitmapToggleButton(const String& image, const QString& tooltip, QWidget* parent = nullptr);

        QWidget* createDefaultPage(const QString& message, QWidget* parent = nullptr);
        QSlider* createSlider(int min, int max);

        float getSliderRatio(const QSlider* slider);
        void setSliderRatio(QSlider* slider, float ratio);

        QLayout* wrapDialogButtonBox(QWidget* buttonBox);
        QLayout* wrapDialogButtonBox(QLayout* buttonBox);

        void addToMiniToolBarLayout(QBoxLayout* layout);

        template <typename... Rest>
        void addToMiniToolBarLayout(QBoxLayout* layout, int first, Rest... rest);

        template <typename... Rest>
        void addToMiniToolBarLayout(QBoxLayout* layout, QWidget* first, Rest... rest) {
            layout->addWidget(first);
            addToMiniToolBarLayout(layout, rest...);
        }

        template <typename... Rest>
        void addToMiniToolBarLayout(QBoxLayout* layout, int first, Rest... rest) {
            layout->addSpacing(first - LayoutConstants::NarrowHMargin);
            addToMiniToolBarLayout(layout, rest...);
        }

        template <typename... Rest>
        QLayout* createMiniToolBarLayout(QWidget* first, Rest... rest) {
            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin, 0);
            layout->setSpacing(LayoutConstants::NarrowHMargin);
            addToMiniToolBarLayout(layout, first, rest...);
            layout->addStretch(1);
            return layout;
        }
#if 0
        void setWindowIcon(wxTopLevelWindow* window);
        QStringList filterBySuffix(const QStringList& strings, const QString& suffix, bool caseSensitive = false);

        QString wxToQString(const QString& string);
#endif

        void setHint(QLineEdit* ctrl, const char* hint);
        void centerOnScreen(QWidget* window);

        QWidget* makeDefault(QWidget* widget);
        QWidget* makeEmphasized(QWidget* widget);
        QWidget* makeUnemphasized(QWidget* widget);
        QWidget* makeInfo(QWidget* widget);
        QWidget* makeHeader(QWidget* widget);
        QWidget* makeError(QWidget* widget);

        QWidget* makeSelected(QWidget* widget);
        QWidget* makeUnselected(QWidget* widget);

        QSettings& getSettings();
        Color fromQColor(const QColor& color);
        QColor toQColor(const Color& color);
        void setWindowIconTB(QWidget* window);
        void setDebugBackgroundColor(QWidget* widget, const QColor& color);

        void setDefaultWindowColor(QWidget* widget);
        void setBaseWindowColor(QWidget* widget);

        QLineEdit* createSearchBox();

        void checkButtonInGroup(QButtonGroup* group, int id, bool checked);
    }
}

#endif /* defined(TrenchBroom_wxUtils) */
