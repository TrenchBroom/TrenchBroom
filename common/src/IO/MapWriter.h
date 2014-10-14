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

#ifndef __TrenchBroom__MapWriter__
#define __TrenchBroom__MapWriter__

#include "Model/EntityAttributes.h"
#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
#include "Model/NodeVisitor.h"

#include <iostream>

namespace TrenchBroom {
    namespace IO {
        class Path;

        class MapSerializer;
        class BrushWriter : public Model::NodeVisitor {
        private:
            MapSerializer& m_serializer;
        public:
            BrushWriter(MapSerializer& serializer);
        private:
            void doVisit(Model::World* world);
            void doVisit(Model::Layer* layer);
            void doVisit(Model::Group* group);
            void doVisit(Model::Entity* entity);
            void doVisit(Model::Brush* brush);
        };
        
        class MapWriter : public Model::NodeVisitor {
        private:
            MapSerializer& m_serializer;
            BrushWriter m_brushWriter;
            const Model::EntityAttribute::List m_parentAttributes;
        public:
            MapWriter(MapSerializer& serializer, const Model::Node* currentParent = NULL);
            
            static void writeToFile(Model::World* map, const Path& path, bool overwrite);
            static void writeToStream(Model::World* map, std::ostream& stream);
            static void writeToStream(Model::MapFormat::Type format, const Model::NodeList& nodes, std::ostream& stream);
            static void writeToStream(Model::MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream);
        private:
            void doVisit(Model::World* world);
            void doVisit(Model::Layer* layer);
            void doVisit(Model::Group* group);
            void doVisit(Model::Entity* entity);
            void doVisit(Model::Brush* brush);
            
            void writeDefaultLayer(const Model::EntityAttribute::List& attrs, const Model::EntityAttribute::List& extra, Model::Node* node);
            void writeContainer(const Model::EntityAttribute::List& attrs, const Model::EntityAttribute::List& extra, Model::Node* node);
            void writeEntity(const Model::EntityAttribute::List& attrs, const Model::EntityAttribute::List& extra, Model::Node* node);

            Model::EntityAttribute::List layerAttributes(const Model::Layer* layer);
            Model::EntityAttribute::List groupAttributes(const Model::Group* group);
            
            void writeAttributes(const Model::EntityAttribute::List& attrs);
        };
    }
}

#endif /* defined(__TrenchBroom__MapWriter__) */
