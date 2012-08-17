/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapWriter.h"

#include "Model/Assets/Texture.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Face.h"
#include "Model/Map/Map.h"
#include "IO/FileManager.h"
#include "Utilities/Console.h"
#include "Utilities/Utils.h"

#include <fstream>
#include <cassert>

namespace TrenchBroom {
    namespace IO {
        void MapWriter::writeFace(const Model::Face& face, std::ostream& stream) {
            std::string textureName = isBlank(face.textureName) ? Model::Assets::Texture::EMPTY : face.textureName;
            
            stream <<
            "( " <<
            face.points[0].x    << " " <<
            face.points[0].y    << " " <<
            face.points[0].z    << " " <<
            " ) ( " <<
            face.points[1].x    << " " <<
            face.points[1].y    << " " <<
            face.points[1].z    << " " <<
            " ) ( " <<
            face.points[2].x    << " " <<
            face.points[2].y    << " " <<
            face.points[2].z    << " " <<
            ") " <<
            textureName         << " " <<
            face.xOffset        << " " <<
            face.yOffset        << " " <<
            face.rotation       << " " <<
            face.xScale         << " " <<
            face.yScale         << std::endl;
        }

        void MapWriter::writeBrush(const Model::Brush& brush, std::ostream& stream) {
            stream << "{" << std::endl;
            for (unsigned int i = 0; i < brush.faces.size(); i++)
                writeFace(*brush.faces[i], stream);
            stream << "}" << std::endl;
        }

        void MapWriter::writeEntityHeader(const Model::Entity& entity, std::ostream& stream) {
            stream << "{" << std::endl;
            
            const Model::Properties& properties = entity.properties();
            for (Model::Properties::const_iterator it = properties.begin(); it != properties.end(); ++it) {
                const Model::PropertyKey& key = it->first;
                const Model::PropertyValue& value = it->second;
                stream << "\"" << key << "\" \"" << value << "\"" << std::endl;
            }
        }
        
        void MapWriter::writeEntityFooter(std::ostream& stream) {
            stream << "}" << std::endl;
        }

        void MapWriter::writeEntity(const Model::Entity& entity, std::ostream& stream) {
            writeEntityHeader(entity, stream);
            const Model::BrushList& brushes = entity.brushes();
            for (unsigned int i = 0; i < brushes.size(); i++)
                writeBrush(*brushes[i], stream);
            writeEntityFooter(stream);
        }

        void MapWriter::writeObjectsToStream(const Model::EntityList& entities, const Model::BrushList& brushes, std::ostream& stream) {
            assert(stream.good());

            // find worldspawn so that we can write it first
            Model::Entity* worldspawn = NULL;
            for (unsigned int i = 0; i < entities.size() && worldspawn == NULL; i++) {
                Model::Entity* entity = entities[i];
                if (entity->worldspawn())
                    worldspawn = entity;
            }
            
            if (worldspawn != NULL)
                writeEntity(*worldspawn, stream);
            
            worldspawn = NULL;
            
            typedef std::map<Model::Entity*, Model::BrushList> EntityBrushMap;
            EntityBrushMap entityToBrushes;
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush* brush = brushes[i];
                Model::Entity* entity = brush->entity();
                entityToBrushes[entity].push_back(brush);
                if (entity->worldspawn())
                    worldspawn = entity;
            }
            
            if (worldspawn != NULL) {
                Model::BrushList& brushList = entityToBrushes[worldspawn];
                writeEntityHeader(*worldspawn, stream);
                for (unsigned int i = 0; i < brushList.size(); i++)
                    writeBrush(*brushList[i], stream);
                writeEntityFooter(stream);
            }
            
            for (unsigned int i = 0; i < entities.size(); i++)
                if (entities[i] != worldspawn)
                    writeEntity(*entities[i], stream);
            
            EntityBrushMap::iterator it, end;
            for (it = entityToBrushes.begin(), end = entityToBrushes.end(); it != end; ++it) {
                Model::Entity* entity = it->first;
                if (entity != worldspawn) {
                    Model::BrushList& brushList = it->second;
                    writeEntityHeader(*entity, stream);
                    for (unsigned int i = 0; i < brushList.size(); i++)
                        writeBrush(*brushList[i], stream);
                    writeEntityFooter(stream);
                }
            }
        }
        
        void MapWriter::writeFacesToStream(const Model::FaceList& faces, std::ostream& stream) {
            assert(stream.good());
            
            for (unsigned int i = 0; i < faces.size(); i++)
                writeFace(*faces[i], stream);
        }

        void MapWriter::writeToStream(const Model::Map& map, std::ostream& stream) {
            assert(stream.good());
            
            const Model::EntityList& entities = map.entities();
            for (unsigned int i = 0; i < entities.size(); i++)
                writeEntity(*entities[i], stream);
        }
        
        void MapWriter::writeToFileAtPath(const Model::Map& map, const std::string& path, bool overwrite) {
            FileManager& fileManager = *FileManager::sharedFileManager;
            if (fileManager.exists(path) && !overwrite) {
                log(TB_LL_ERR, "Cannot save map file to %s because a file exists at that location", path.c_str());
                return;
            }
            
            const std::string directoryPath = fileManager.deleteLastPathComponent(path);
            if (!fileManager.exists(directoryPath))
                fileManager.makeDirectory(directoryPath);
            
            std::fstream stream(path.c_str(), std::ios::out | std::ios::trunc);
            writeToStream(map, stream);
        }
    }
}
