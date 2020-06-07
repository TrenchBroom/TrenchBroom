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

#ifndef TrenchBroom_FlagsEditor
#define TrenchBroom_FlagsEditor

#include <QWidget>
#include <QStringList>

#include <vector>

class QCheckBox;

namespace TrenchBroom {
    namespace View {
        class FlagsEditor : public QWidget {
            Q_OBJECT
        private:
            using CheckBoxList = std::vector<QCheckBox*>;
            using ValueList = std::vector<int>;

            size_t m_numCols;
            CheckBoxList m_checkBoxes;
            ValueList m_values;
        public:
            explicit FlagsEditor(size_t numCols, QWidget* parent = nullptr);

            void setFlags(const QStringList& labels, const QStringList& tooltips = QStringList());
            void setFlags(const QList<int>& values, const QStringList& labels, const QStringList& tooltips = QStringList());
            void setFlagValue(int set, int mixed = 0);

            size_t getNumFlags() const;
            bool isFlagSet(size_t index) const;
            bool isFlagMixed(size_t index) const;
            int getSetFlagValue() const;
            int getMixedFlagValue() const;
            QString getFlagLabel(size_t index) const;

            int lineHeight() const;
        signals:
            /**
             * Sent when a checkbox is clicked.
             * If (value & setFlag) != 0 it means the checkbox's bit value was just set, otherwise it was unset.
             *
             * @param index the index of the checkbox (not the bit position)
             * @param value the bit value represented by the checkbox
             * @param setFlag the bitwise OR of the values of all currently checked checkboxes (same as `getSetFlagValue()`)
             * @param mixedFlag the bitwise OR of the values of all currently mixed checkboxes (same as `getMixedFlagValue()`)
             */
            void flagChanged(size_t index, int value, int setFlag, int mixedFlag);
        };
    }
}

#endif /* defined(TrenchBroom_FlagsEditor) */
