/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "FixPlanePointsCommand.h"

#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType FixPlanePointsCommand::Type = Command::freeType();

        FixPlanePointsCommand::Ptr FixPlanePointsCommand::snapPlanePoints(View::MapDocumentWPtr document, const Model::BrushList& brushes) {
            return Ptr(new FixPlanePointsCommand(document, SnapPoints, brushes));
        }
        
        FixPlanePointsCommand::Ptr FixPlanePointsCommand::findPlanePoints(View::MapDocumentWPtr document, const Model::BrushList& brushes) {
            return Ptr(new FixPlanePointsCommand(document, FindPoints, brushes));
        }

        FixPlanePointsCommand::FixPlanePointsCommand(View::MapDocumentWPtr document, const Action action, const Model::BrushList& brushes) :
        Command(Type, makeName(action, brushes), true, true),
        m_document(document),
        m_action(action),
        m_brushes(brushes) {
            assert(!brushes.empty());
        }
        
        String FixPlanePointsCommand::makeName(const Action action, const Model::BrushList& brushes) {
            switch (action) {
                case SnapPoints:
                    return "Snap plane points";
                case FindPoints:
                    return "Find plane points";
                default:
                    assert(false);
                    return "";
            }
        }
        
        struct SnapPlanePoints {
        private:
            const BBox3& m_worldBounds;
        public:
            SnapPlanePoints(const BBox3& worldBounds) :
            m_worldBounds(worldBounds) {}
            
            void operator()(Model::Brush* brush) const {
                brush->snapPlanePointsToInteger(m_worldBounds);
            }
        };
        
        struct FindPlanePoints {
        private:
            const BBox3& m_worldBounds;
        public:
            FindPlanePoints(const BBox3& worldBounds) :
            m_worldBounds(worldBounds) {}
            
            void operator()(Model::Brush* brush) const {
                brush->findIntegerPlanePoints(m_worldBounds);
            }
        };

        bool FixPlanePointsCommand::doPerformDo() {
            m_snapshot = Model::Snapshot(m_brushes);
            
            View::MapDocumentSPtr document = lock(m_document);
            Model::NotifyParent parentWillChange(document->objectWillChangeNotifier);
            Model::each(m_brushes.begin(), m_brushes.end(), parentWillChange, Model::MatchAll());
            
            document->objectWillChangeNotifier(m_brushes.begin(), m_brushes.end());
            switch (m_action) {
                case SnapPoints:
                    Model::each(m_brushes.begin(), m_brushes.end(), SnapPlanePoints(document->worldBounds()), Model::MatchAll());
                    break;
                case FindPoints:
                    Model::each(m_brushes.begin(), m_brushes.end(), FindPlanePoints(document->worldBounds()), Model::MatchAll());
                    break;
            }
            document->objectDidChangeNotifier(m_brushes.begin(), m_brushes.end());
            
            Model::NotifyParent parentDidChange(document->objectDidChangeNotifier);
            Model::each(m_brushes.begin(), m_brushes.end(), parentDidChange, Model::MatchAll());
            return true;
        }
        
        bool FixPlanePointsCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            Model::NotifyParent parentWillChange(document->objectWillChangeNotifier);
            Model::each(m_brushes.begin(), m_brushes.end(), parentWillChange, Model::MatchAll());
            
            document->objectWillChangeNotifier(m_brushes.begin(), m_brushes.end());
            m_snapshot.restore(document->worldBounds());
            document->objectDidChangeNotifier(m_brushes.begin(), m_brushes.end());
            
            Model::NotifyParent parentDidChange(document->objectDidChangeNotifier);
            Model::each(m_brushes.begin(), m_brushes.end(), parentDidChange, Model::MatchAll());
            return true;
        }
    }
}
