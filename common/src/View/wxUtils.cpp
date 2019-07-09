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

#include "wxUtils.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
//#include "View/BitmapStaticButton.h"
//#include "View/BitmapToggleButton.h"
#include "View/BorderLine.h"
#include "View/MapFrame.h"
#include "View/ViewConstants.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QDir>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QSlider>
#include <QSplitter>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>
#include <QStyle>
#include <QToolButton>
#include <QAbstractButton>
#include <QButtonGroup>

#include <list>
#include <cstdlib>

namespace TrenchBroom {
    namespace View {
        DisableWindowUpdates::DisableWindowUpdates(QWidget* widget) :
        m_widget(widget) {
            m_widget->setUpdatesEnabled(false);
        }

        DisableWindowUpdates::~DisableWindowUpdates() {
            m_widget->setUpdatesEnabled(true);
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
            QSettings settings;
            window->restoreGeometry(settings.value(path).toByteArray());
        }

        MapFrame* findMapFrame(QWidget* widget) {
            return dynamic_cast<MapFrame*>(widget->window());
        }

        // FIXME: Port this stuff as needed
#if 0

        wxFrame* findFrame(QWidget* window) {
            if (window == nullptr)
                return nullptr;
            return wxDynamicCast(wxGetTopLevelParent(window), wxFrame);
        }

        void fitAll(QWidget* window) {
            for (QWidget* child : window->GetChildren())
                fitAll(child);
            window->Fit();
        }

        wxColor makeLighter(const wxColor& color) {
            wxColor result = color.ChangeLightness(130);
            if (std::abs(result.Red()   - color.Red())   < 25 &&
                std::abs(result.Green() - color.Green()) < 25 &&
                std::abs(result.Blue()  - color.Blue())  < 25)
                result = color.ChangeLightness(70);
            return result;
        }
#endif

        void setHint(QLineEdit* ctrl, const char* hint) {
            ctrl->setPlaceholderText(hint);
        }

        void centerOnScreen(QWidget* window) {
            window->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    window->size(),
                    QApplication::desktop()->availableGeometry()
                )
            );
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

            auto font = widget->font();
            font.setPointSize(font.pointSize() - 2);
            widget->setFont(font);

            const auto defaultPalette = QPalette();
            auto palette = widget->palette();
            palette.setColor(QPalette::Normal, QPalette::WindowText, defaultPalette.color(QPalette::Disabled, QPalette::WindowText));
            palette.setColor(QPalette::Normal, QPalette::Text, defaultPalette.color(QPalette::Disabled, QPalette::WindowText));
            widget->setPalette(palette);
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

        QWidget* makeSelected(QWidget* widget) {
            const auto defaultPalette = QPalette();
            auto palette = widget->palette();
            palette.setColor(QPalette::Normal, QPalette::WindowText, defaultPalette.color(QPalette::Normal, QPalette::HighlightedText));
            palette.setColor(QPalette::Normal, QPalette::Text, defaultPalette.color(QPalette::Normal, QPalette::HighlightedText));
            widget->setPalette(palette);
            return widget;
        }

        QWidget* makeUnselected(QWidget* widget) {
            const auto defaultPalette = QPalette();
            auto palette = widget->palette();
            palette.setColor(QPalette::Normal, QPalette::WindowText, defaultPalette.color(QPalette::Normal, QPalette::WindowText));
            palette.setColor(QPalette::Normal, QPalette::Text, defaultPalette.color(QPalette::Normal, QPalette::Text));
            widget->setPalette(palette);
            return widget;
        }

        QSettings& getSettings() {
            static auto settings =
#if defined __linux__ || defined __FreeBSD__
                QSettings(QDir::homePath() % QString::fromLocal8Bit("/.TrenchBroom/.preferences"), QSettings::Format::IniFormat);
#elif defined __APPLE__
                QSettings(QStandardPaths::locate(QStandardPaths::ConfigLocation,
                                                 QString::fromLocal8Bit("TrenchBroom Preferences"),
                                                 QStandardPaths::LocateOption::LocateFile), QSettings::Format::IniFormat);
#else
                QSettings();
#endif
            return settings;
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
#if 0
        wxColor toWxColor(const Color& color) {
            const unsigned char r = static_cast<unsigned char>(color.r() * 255.0f);
            const unsigned char g = static_cast<unsigned char>(color.g() * 255.0f);
            const unsigned char b = static_cast<unsigned char>(color.b() * 255.0f);
            const unsigned char a = static_cast<unsigned char>(color.a() * 255.0f);
            return wxColor(r, g, b, a);
        }

        std::vector<size_t> getListCtrlSelection(const wxListCtrl* listCtrl) {
            ensure(listCtrl != nullptr, "listCtrl is null");

            std::vector<size_t> result(static_cast<size_t>(listCtrl->GetSelectedItemCount()));

            size_t i = 0;
            long itemIndex = listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            while (itemIndex >= 0) {
                result[i++] = static_cast<size_t>(itemIndex);
                itemIndex = listCtrl->GetNextItem(itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            }
            return result;
        }

        void deselectAllListrCtrlItems(wxListCtrl* listCtrl) {
            for (const auto index : getListCtrlSelection(listCtrl)) {
                listCtrl->SetItemState(static_cast<long>(index), 0, wxLIST_STATE_SELECTED);
            }
        }
#endif

        QAbstractButton* createBitmapButton(const String& image, const QString& tooltip, QWidget* parent) {
            return createBitmapButton(loadIconResourceQt(IO::Path(image)), tooltip, parent);
        }

        QAbstractButton* createBitmapButton(const QIcon& icon, const QString& tooltip, QWidget* parent) {
            ensure(!icon.availableSizes().empty(), "expected a non-empty icon. Fails when the image file couldn't be found.");

            // NOTE: according to http://doc.qt.io/qt-5/qpushbutton.html this would be more correctly
            // be a QToolButton, but the QToolButton doesn't have a flat style on macOS
            auto* button = new QToolButton(parent);
            button->setMinimumSize(icon.availableSizes().front());
            // button->setAutoDefault(false);
            button->setToolTip(tooltip);
            button->setIcon(icon);
            // button->setFlat(true);
            button->setStyleSheet("QToolButton { border: none; }");

            return button;
        }

        QAbstractButton* createBitmapToggleButton(const String& image, const QString& tooltip, QWidget* parent) {
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
            outerLayout->addWidget(new BorderLine(BorderLine::Direction_Horizontal));
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
            outerLayout->addWidget(new BorderLine(BorderLine::Direction_Horizontal));
            outerLayout->addLayout(innerLayout);

            return outerLayout;
        }

        void addToMiniToolBarLayout(QBoxLayout*) {}

#if 0
        QWidget* createDefaultPage(QWidget* parent, const QString& message) {
            QWidget* containerPanel = new QWidget(parent);

            QLabel* messageText = new QLabel(containerPanel, wxID_ANY, message);
            messageText->SetFont(messageText->GetFont().Bold());
            messageText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

            auto* justifySizer = new QHBoxLayout();
            justifySizer->addStretch(1);
            justifySizer->addSpacing(LayoutConstants::WideHMargin);
            justifySizer->addWidget(messageText, wxSizerFlags().Expand());
            justifySizer->addSpacing(LayoutConstants::WideHMargin);
            justifySizer->addStretch(1);

            auto* containerSizer = new QVBoxLayout();
            containerSizer->addSpacing(LayoutConstants::MediumVMargin);
            containerSizer->addWidget(justifySizer, wxSizerFlags().Expand());
            containerSizer->addSpacing(LayoutConstants::MediumVMargin);
            containerSizer->addStretch(1);

            containerPanel->setLayout(containerSizer);
            return containerPanel;
        }

#if 0
        wxSizer* wrapDialogButtonSizer(wxSizer* buttonSizer, QWidget* parent) {
            auto* hSizer = new QHBoxLayout();
            hSizer->addSpacing(LayoutConstants::DialogButtonLeftMargin);
            hSizer->addWidget(buttonSizer, wxSizerFlags().Expand().Proportion(1));
            hSizer->addSpacing(LayoutConstants::DialogButtonRightMargin);

            auto* vSizer = new QVBoxLayout();
            vSizer->addWidget(new BorderLine(parent, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            vSizer->addSpacing(LayoutConstants::DialogButtonTopMargin);
            vSizer->addWidget(hSizer, wxSizerFlags().Expand());
            vSizer->addSpacing(LayoutConstants::DialogButtonBottomMargin);
            return vSizer;
        }
#endif
#endif

        void setWindowIconTB(QWidget* window) {
            ensure(window != nullptr, "window is null");
            window->setWindowIcon(IO::loadIconResourceQt(IO::Path("AppIcon.png")));
        }

#if 0
        QStringList filterBySuffix(const QStringList& strings, const QString& suffix, const bool caseSensitive) {
            QStringList result;
            for (size_t i = 0; i < strings.size(); ++i) {
                const QString& str = strings[i];
                if (caseSensitive) {
                    if (str.EndsWith(suffix))
                        result.Add(str);
                } else {
                    if (str.Lower().EndsWith(suffix.Lower()))
                        result.Add(str);
                }
            }
            return result;
        }

        QString wxToQString(const QString& string) {
            const auto utf8 = string.ToUTF8();
            return QString::fromUtf8(utf8.data(), utf8.length());
        }
#endif

        void setDebugBackgroundColor(QWidget* widget, const QColor& color) {
            QPalette p = widget->palette();
            p.setColor(QPalette::Window, color);

            widget->setAutoFillBackground(true);
            widget->setPalette(p);
        }

        void setDefaultWindowColor(QWidget* widget) {
            auto palette = QPalette();
            palette.setColor(QPalette::Window, palette.color(QPalette::Normal, QPalette::Window));
            widget->setAutoFillBackground(true);
            widget->setPalette(palette);
        }

        void setBaseWindowColor(QWidget* widget) {
            auto palette = QPalette();
            palette.setColor(QPalette::Window, palette.color(QPalette::Normal, QPalette::Base));
            widget->setAutoFillBackground(true);
            widget->setPalette(palette);
        }

        QLineEdit* createSearchBox() {
            auto* widget = new QLineEdit();
            widget->setClearButtonEnabled(true);
            widget->setPlaceholderText(QLineEdit::tr("Search..."));

            QIcon icon = loadIconResourceQt(IO::Path("")); // FIXME:!
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
    }
}
