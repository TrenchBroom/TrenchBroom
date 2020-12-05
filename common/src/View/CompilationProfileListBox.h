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

#include "View/ControlListBox.h"

class QPoint;

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig;
        class CompilationProfile;
    }

    namespace View {
        class ElidedLabel;

        class CompilationProfileItemRenderer : public ControlListBoxItemRenderer {
            Q_OBJECT
        private:
            Model::CompilationProfile* m_profile;
            ElidedLabel* m_nameText;
            ElidedLabel* m_taskCountText;
        public:
            explicit CompilationProfileItemRenderer(Model::CompilationProfile& profile, QWidget* parent = nullptr);
            ~CompilationProfileItemRenderer() override;
        private:
            void updateItem() override;
        };

        class CompilationProfileListBox : public ControlListBox {
            Q_OBJECT
        private:
            const Model::CompilationConfig& m_config;
        public:
            explicit CompilationProfileListBox(const Model::CompilationConfig& config, QWidget* parent = nullptr);
        public:
            void reloadProfiles();
            void updateProfiles();
        private:
            size_t itemCount() const override;
            ControlListBoxItemRenderer* createItemRenderer(QWidget* parent, size_t index) override;
        signals:
            void profileContextMenuRequested(const QPoint& globalPos, Model::CompilationProfile* profile);
        };
    }
}

