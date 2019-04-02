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

#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QStringBuilder>

#include <list>
#include <cstdlib>

namespace TrenchBroom {
    namespace View {
        // FIXME: Port this stuff as needed
#if 0
        MapFrame* findMapFrame(QWidget* window) {
            // FIXME:
            return nullptr;//wxDynamicCast(findFrame(window), MapFrame);
        }

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
        QSettings getSettings() {
            QString path = QDir::homePath() % QString::fromLocal8Bit("/.TrenchBroom/.preferences");
            return QSettings(path, QSettings::IniFormat);
        }

        Color fromQColor(const QColor& color) {
            return Color(static_cast<float>(color.redF()),
                         static_cast<float>(color.greenF()),
                         static_cast<float>(color.blueF()),
                         static_cast<float>(color.alphaF()));
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

        QAbstractButton* createBitmapButton(QWidget* parent, const String& image, const QString& tooltip) {
            QIcon icon = loadIconResourceQt(IO::Path(image));

            // NOTE: according to http://doc.qt.io/qt-5/qpushbutton.html this would be more correctly
            // be a QToolButton, but the QToolButton doesn't have a flat style on macOS
            auto* button = new QPushButton(parent);
            button->setAutoDefault(false);
            button->setToolTip(tooltip);
            button->setIcon(icon);
            button->setFlat(true);

            return button;
        }

#if 0
        QWidget* createBitmapToggleButton(QWidget* parent, const String& upImage, const String& downImage, const String& tooltip) {
            auto upBitmap = IO::loadImageResource(upImage);
            auto downBitmap = IO::loadImageResource(downImage);

            auto* button = new BitmapToggleButton(parent, wxID_ANY, upBitmap, downBitmap);
            button->setToolTip(tooltip);
            return button;
        }

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
            containerSizer->addSpacing(LayoutConstants::WideVMargin);
            containerSizer->addWidget(justifySizer, wxSizerFlags().Expand());
            containerSizer->addSpacing(LayoutConstants::WideVMargin);
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
            window->setWindowIcon(IO::loadIconResourceQt(IO::Path("Resources/WindowIcon")));
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

        QLineEdit* createSearchBox() {
            auto* widget = new QLineEdit();
            widget->setClearButtonEnabled(true);
            widget->setPlaceholderText(QLineEdit::tr("Search..."));

            QIcon icon = loadIconResourceQt(IO::Path("")); // FIXME:!
            widget->addAction(icon, QLineEdit::LeadingPosition);
            return widget;
        }
    }
}
