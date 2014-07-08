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
        Command(Type, makeName(action, objects), true, true),
        m_document(document),
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

            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();

            notifyBefore(document);
            Model::each(m_objects.begin(), m_objects.end(),
                        Model::Transform(m_transformation, m_lockTextures, worldBounds),
                        Model::MatchAll());
            assert(Model::each(m_objects.begin(), m_objects.end(), Model::CheckBounds(worldBounds)));
            notifyAfter(document);
            
            return true;
        }
        
        bool TransformObjectsCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);
            
            notifyBefore(document);
            m_snapshot.restore(document->worldBounds());
            notifyAfter(document);
            return true;
        }

        void TransformObjectsCommand::notifyBefore(View::MapDocumentSPtr document) {
            Model::NotifyParent parentWillChange(document->objectWillChangeNotifier);
            Model::each(m_objects.begin(), m_objects.end(), parentWillChange, Model::MatchAll());
            
            Model::NotifyObject objectWillChange(document->objectWillChangeNotifier);
            Model::each(m_objects.begin(), m_objects.end(), objectWillChange, Model::MatchAll());
        }
        
        void TransformObjectsCommand::notifyAfter(View::MapDocumentSPtr document) {
            Model::NotifyObject objectDidChange(document->objectDidChangeNotifier);
            Model::each(m_objects.begin(), m_objects.end(), objectDidChange, Model::MatchAll());
            
            Model::NotifyParent parentDidChange(document->objectDidChangeNotifier);
            Model::each(m_objects.begin(), m_objects.end(), parentDidChange, Model::MatchAll());
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
