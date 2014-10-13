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

#ifndef __TrenchBroom__MapSerializer__
#define __TrenchBroom__MapSerializer__

#include "Model/EntityAttributes.h"
#include "Model/ModelTypes.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class MapSerializer {
        protected:
            static const int FloatPrecision = 17;
        public:
            typedef std::auto_ptr<MapSerializer> Ptr;
            
            virtual ~MapSerializer();
            
            void beginEntity(const Model::Node* node);
            void endEntity(Model::Node* node);
            void entityAttribute(const Model::EntityAttribute& attribute);
            
            void beginBrush(const Model::Brush* brush);
            void endBrush(Model::Brush* brush);
            void brushFace(Model::BrushFace* face);
        private:
            virtual void doBeginEntity(const Model::Node* node) = 0;
            virtual void doEndEntity(Model::Node* node) = 0;
            virtual void doEntityAttribute(const Model::EntityAttribute& attribute) = 0;
            
            virtual void doBeginBrush(const Model::Brush* brush) = 0;
            virtual void doEndBrush(Model::Brush* brush) = 0;
            virtual void doBrushFace(Model::BrushFace* face) = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__MapSerializer__) */
