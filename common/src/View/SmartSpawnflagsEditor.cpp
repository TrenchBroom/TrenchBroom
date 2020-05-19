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

#include "SmartSpawnflagsEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/WorldNode.h"
#include "View/FlagsEditor.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"

#include <kdl/set_temp.h>

#include <cassert>
#include <memory>
#include <vector>

#include <QScrollArea>
#include <QVBoxLayout>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class SmartSpawnflagsEditor::UpdateSpawnflag : public Model::NodeVisitor {
        private:
            std::shared_ptr<MapDocument> m_document;
            const std::string& m_name;
            size_t m_flagIndex;
            bool m_setFlag;
        public:
            UpdateSpawnflag(std::shared_ptr<MapDocument> document, const std::string& name, const size_t flagIndex, const bool setFlag) :
            m_document(document),
            m_name(name),
            m_flagIndex(flagIndex),
            m_setFlag(setFlag) {}

            void doVisit(Model::WorldNode*) override  { m_document->updateSpawnflag(m_name, m_flagIndex, m_setFlag); }
            void doVisit(Model::LayerNode*) override  {}
            void doVisit(Model::GroupNode*) override  {}
            void doVisit(Model::Entity*) override { m_document->updateSpawnflag(m_name, m_flagIndex, m_setFlag); }
            void doVisit(Model::BrushNode*) override  {}
        };

        SmartSpawnflagsEditor::SmartSpawnflagsEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        SmartAttributeEditor(document, parent),
        m_scrolledWindow(nullptr),
        m_flagsEditor(nullptr),
        m_ignoreUpdates(false) {
            createGui();
        }

        void SmartSpawnflagsEditor::createGui() {
            assert(m_scrolledWindow == nullptr);

            m_scrolledWindow = new QScrollArea();

            m_flagsEditor = new FlagsEditor(NumCols);
            connect(m_flagsEditor, &FlagsEditor::flagChanged, this, &SmartSpawnflagsEditor::flagChanged);

            m_scrolledWindow->setWidget(m_flagsEditor);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(m_scrolledWindow, 1);
            setLayout(layout);
        }

        void SmartSpawnflagsEditor::doUpdateVisual(const std::vector<Model::AttributableNode*>& attributables) {
            assert(!attributables.empty());
            if (m_ignoreUpdates)
                return;

            QStringList labels;
            QStringList tooltips;
            getFlags(attributables, labels, tooltips);
            m_flagsEditor->setFlags(labels, tooltips);

            int set, mixed;
            getFlagValues(attributables, set, mixed);
            m_flagsEditor->setFlagValue(set, mixed);
        }

        void SmartSpawnflagsEditor::getFlags(const std::vector<Model::AttributableNode*>& attributables, QStringList& labels, QStringList& tooltips) const {
            QStringList defaultLabels;

            // Initialize the labels and tooltips.
            for (size_t i = 0; i < NumFlags; ++i) {
                QString defaultLabel;
                defaultLabel = QString::number(1 << i);

                defaultLabels.push_back(defaultLabel);
                labels.push_back(defaultLabel);
                tooltips.push_back("");
            }

            for (size_t i = 0; i < NumFlags; ++i) {
                bool firstPass = true;
                for (const Model::AttributableNode* attributable : attributables) {
                    const int indexI = static_cast<int>(i);
                    QString label = defaultLabels[indexI];
                    QString tooltip = "";

                    const Assets::FlagsAttributeOption* flagDef = Assets::EntityDefinition::safeGetSpawnflagsAttributeOption(attributable->definition(), i);
                    if (flagDef != nullptr) {
                        label = QString::fromStdString(flagDef->shortDescription());
                        tooltip = QString::fromStdString(flagDef->longDescription());
                    }

                    if (firstPass) {
                        labels[indexI] = label;
                        tooltips[indexI] = tooltip;
                        firstPass = false;
                    } else {
                        if (labels[indexI] != label) {
                            labels[indexI] = defaultLabels[indexI];
                            tooltips[indexI].clear();
                        }
                    }
                }
            }
        }

        void SmartSpawnflagsEditor::getFlagValues(const std::vector<Model::AttributableNode*>& attributables, int& setFlags, int& mixedFlags) const {
            if (attributables.empty()) {
                setFlags = 0;
                mixedFlags = 0;
                return;
            }

            auto it = std::begin(attributables);
            auto end = std::end(attributables);
            setFlags = getFlagValue(*it);
            mixedFlags = 0;

            while (++it != end)
                combineFlags(NumFlags, getFlagValue(*it), setFlags, mixedFlags);
        }

        int SmartSpawnflagsEditor::getFlagValue(const Model::AttributableNode* attributable) const {
            if (!attributable->hasAttribute(name()))
                return 0;

            const std::string& value = attributable->attribute(name());
            return std::atoi(value.c_str());
        }

        void SmartSpawnflagsEditor::flagChanged(const size_t index, const int /* setFlag */, const int /* mixedFlag */) {
            const std::vector<Model::AttributableNode*>& toUpdate = attributables();
            if (toUpdate.empty())
                return;

            const bool set = m_flagsEditor->isFlagSet(index);

            const kdl::set_temp ignoreUpdates(m_ignoreUpdates);

            const Transaction transaction(document(), "Set Spawnflags");
            UpdateSpawnflag visitor(document(), name(), index, set);
            Model::Node::accept(std::begin(toUpdate), std::end(toUpdate), visitor);
        }
    }
}
