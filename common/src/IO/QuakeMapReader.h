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

#ifndef __TrenchBroom__QuakeMapReader__
#define __TrenchBroom__QuakeMapReader__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "IO/QuakeMapParser.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeBuilder;
    }
    
    namespace IO {
        class QuakeMapReader : public QuakeMapParser {
        private:
            typedef enum {
                EntityType_Layer,
                EntityType_Group,
                EntityType_Worldspawn,
                EntityType_Default
            } EntityType;
            
            typedef std::map<String, Model::Layer*> LayerMap;
            typedef std::map<String, Model::Group*> GroupMap;
            
            BBox3 m_worldBounds;
            const Model::BrushContentTypeBuilder* m_brushContentTypeBuilder;
            Model::World* m_world;
            Model::Node* m_parent;
            Model::Node* m_currentNode;
            Model::BrushFaceList m_faces;
            
            LayerMap m_layers;
            GroupMap m_groups;
        public:
            QuakeMapReader(const char* begin, const char* end, const Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger = NULL);
            QuakeMapReader(const String& str, const Model::BrushContentTypeBuilder* brushContentTypeBuilder, Logger* logger = NULL);
            ~QuakeMapReader();
            
            Model::World* read(const BBox3& worldBounds);
        private: // implement MapParser interface
            void onFormatDetected(Model::MapFormat::Type format);
            void onBeginEntity(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void onEndEntity(size_t startLine, size_t lineCount);
            void onBeginBrush(size_t line);
            void onEndBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes);
            void onBrushFace(size_t line, const Vec3& point1, const Vec3& point2, const Vec3& point3, const Model::BrushFaceAttribs& attribs, const Vec3& texAxisX, const Vec3& texAxisY);
        private: // helper methods
            void createLayer(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void createGroup(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void createEntity(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            Model::Node* findParentForEntity(size_t line, const Model::EntityAttribute::List& attributes) const;
            void createWorldspawn(size_t line, const Model::EntityAttribute::List& attributes, const ExtraAttributes& extraAttributes);
            void createBrush(size_t startLine, size_t lineCount, const ExtraAttributes& extraAttributes);
            
            EntityType entityType(const Model::EntityAttribute::List& attributes) const;
            bool isLayer(const String& classname, const Model::EntityAttribute::List& attributes) const;
            bool isGroup(const String& classname, const Model::EntityAttribute::List& attributes) const;
            bool isWorldspawn(const String& classname, const Model::EntityAttribute::List& attributes) const;

            const String& findAttribute(const Model::EntityAttribute::List& attributes, const String& name, const String& defaultValue = EmptyString) const;
            
            void setFilePosition(Model::Node* node, size_t startLine, size_t lineCount);
            void setExtraAttributes(Model::Node* node, const ExtraAttributes& extraAttributes);
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeMapReader__) */
