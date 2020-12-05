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

#pragma once

#include <QLineEdit>
#include <QRegularExpression>

class QCompleter;

namespace TrenchBroom {
    namespace View {
        class MultiCompletionLineEdit : public QLineEdit {
        private:
            QCompleter* m_multiCompleter;
            QRegularExpression m_leftDelimiter;
            QRegularExpression m_rightDelimiter;
        public:
            explicit MultiCompletionLineEdit(QWidget* parent = nullptr);
            explicit MultiCompletionLineEdit(const QString& contents, QWidget* parent = nullptr);
            ~MultiCompletionLineEdit() override;
        public:
            void setWordDelimiter(const QRegularExpression& leftDelimiter);
            void setWordDelimiters(const QRegularExpression& leftDelimiter, const QRegularExpression& rightDelimiter);
            void setMultiCompleter(QCompleter* completer);
        protected:
            void keyPressEvent(QKeyEvent* event) override;
        private:
            void updateCompleter(bool showCompleter);

            int findLeftBoundary() const;
            int findRightBoundary() const;

            int findFirstMatch(const QString& str, const QRegularExpression& expression) const;
            int findLastMatch(const QString& str, const QRegularExpression& expression) const;
        private slots:
            void triggerCompletion();
            void insertCompletion(const QString& string);
        };
    }
}

