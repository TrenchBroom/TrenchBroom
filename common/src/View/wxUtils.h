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

#include <vector>

class QAbstractButton;
class QDialogButtonBox;
class QFont;
class QLayout;
class QLineEdit;
class QMainWindow;
class QPalette;
class QSettings;
class QSlider;
class QWidget;
class QButtonGroup;

namespace TrenchBroom {
    namespace View {
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

        QLayout* wrapDialogButtonBox(QDialogButtonBox* buttonBox);

        template <typename Button>
        void addToMiniToolBarLayout(QBoxLayout* layout, Button* button) {
            layout->addWidget(button);
        }

        template <typename Button, typename... Rest>
        void addToMiniToolBarLayout(QBoxLayout* layout, Button* first, Rest... buttons) {
            layout->addWidget(first);
            addToMiniToolBarLayout(layout, buttons...);
        }

        template <typename Button, typename... Rest>
        QLayout* createMiniToolBarLayout(Button* first, Rest... buttons) {
            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(LayoutConstants::NarrowHMargin, 0, LayoutConstants::NarrowHMargin, 0);
            layout->setSpacing(LayoutConstants::NarrowHMargin);
            addToMiniToolBarLayout(layout, first, buttons...);
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
