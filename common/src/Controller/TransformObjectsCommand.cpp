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

#include "TransformObjectsCommand.h"

#include "Macros.h"
#include "Model/ModelUtils.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType TransformObjectsCommand::Type = Command::freeType();

        TransformObjectsCommand::Ptr TransformObjectsCommand::translateObjects(View::MapDocumentWPtr document, const Vec3& offset, const bool lockTextures, const Model::ObjectList& objects) {
            const Mat4x4 transform = translationMatrix(offset);
            return Ptr(new TransformObjectsCommand(document, Action_Translate, transform, lockTextures, objects));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::rotateObjects(View::MapDocumentWPtr document, const Vec3& center, const Vec3& axis, const FloatType angle, const bool lockTextures, const Model::ObjectList& objects) {
            const Mat4x4 transform = translationMatrix(center) * rotationMatrix(axis, angle) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(document, Action_Rotate, transform, lockTextures, objects));
        }
        
        TransformObjectsCommand::Ptr TransformObjectsCommand::flipObjects(View::MapDocumentWPtr document, const Vec3& center, Math::Axis::Type axis, const bool lockTextures, const Model::ObjectList& objects) {
            const Mat4x4 transform = translationMatrix(center) * mirrorMatrix<FloatType>(axis) * translationMatrix(-center);
            return Ptr(new TransformObjectsCommand(document, Action_Flip, transform, lockTextures, objects));
        }
        
        TransformObjectsCommand::TransformObjectsCommand(View::MapDocumentWPtr document, const Action action, const Mat4x4& transformation, const bool lockTextures, const Model::ObjectList& objects) :
        DocumentCommand(Type, makeName(action, objects), true, document),
        m_action(action),
        m_transformation(transformation),
        m_lockTextures(lockTextures),
        m_objects(objects) {
            assert(!objects.empty());
        }
        
        String TransformObjectsCommand::makeName(const Action action, const Model::ObjectList& objects) {
            StringStream actionName;
            switch (action) {
                case Action_Translate:
                    actionName << "Translate";
                    break;
                case Action_Rotate:
                    actionName << "Rotate";
                    break;
                case Action_Flip:
                    actionName << "Flip";
                    break;
                DEFAULT_SWITCH();
            }
            actionName << " " << (objects.size() == 1 ? " object" : " objects");
            return actionName.str();
        }
        
        bool TransformObjectsCommand::doPerformDo() {
            m_snapshot = Model::Snapshot(m_objects);

            View::MapDocumentSPtr document = lockDocument();
            const BBox3& worldBounds = document->worldBounds();

            Model::ObjectList allChangedObjects = Model::makeParentList(m_objects);
            VectorUtils::append(allChangedObjects, m_objects);
            document->objectsWillChangeNotifier(allChangedObjects);
            
            Model::each(m_objects.begin(), m_objects.end(),
                        Model::Transform(m_transformation, m_lockTextures, worldBounds),
                        Model::MatchAll());
            assert(Model::each(m_objects.begin(), m_objects.end(), Model::CheckBounds(worldBounds)));
            document->objectsDidChangeNotifier(allChangedObjects);
            
            return true;
        }
        
        bool TransformObjectsCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lockDocument();
            
            Model::ObjectList allChangedObjects = Model::makeParentList(m_objects);
            VectorUtils::append(allChangedObjects, m_objects);
            document->objectsWillChangeNotifier(allChangedObjects);
            
            m_snapshot.restore(document->worldBounds());
            document->objectsDidChangeNotifier(allChangedObjects);

            return true;
        }

        bool TransformObjectsCommand::doIsRepeatable() const {
            return true;
        }
        
        Command* TransformObjectsCommand::doRepeat(View::MapDocumentSPtr document) const {
            return new TransformObjectsCommand(document, m_action, m_transformation, m_lockTextures, document->selectedObjects());
        }

        bool TransformObjectsCommand::doCollateWith(Command::Ptr command) {
            Ptr other = Command::cast<TransformObjectsCommand>(command);
            if (other->m_lockTextures != m_lockTextures)
                return false;
            if (other->m_action != m_action)
                return false;
            m_transformation = m_transformation * other->m_transformation;
            return true;
        }
    }
}
