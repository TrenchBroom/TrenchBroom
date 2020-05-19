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

#ifndef TrenchBroom_CollectSelectableBrushFacesVisitor
#define TrenchBroom_CollectSelectableBrushFacesVisitor

#include "Model/CollectMatchingBrushFacesVisitor.h"

#include <functional>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushNode;
        class EditorContext;

        using FacePredicate = std::function<bool(const Model::BrushNode*, const BrushFace*)>;

        class MatchSelectableBrushFaces {
        private:
            const EditorContext& m_editorContext;
            FacePredicate m_predicate;
        private:
            bool testPredicate(const Model::BrushNode* brush, const BrushFace* face) const;
        public:
            MatchSelectableBrushFaces(const EditorContext& editorContext, FacePredicate predicate);
            bool operator()(const Model::BrushNode* brush, const BrushFace* face) const;
        };

        class CollectSelectableBrushFacesVisitor : public CollectMatchingBrushFacesVisitor<MatchSelectableBrushFaces> {
        public:
            CollectSelectableBrushFacesVisitor(const EditorContext& editorContext, FacePredicate predicate = FacePredicate());
        };
    }
}

#endif /* defined(TrenchBroom_CollectSelectableBrushFacesVisitor) */
