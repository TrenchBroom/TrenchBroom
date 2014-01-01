/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "MapWriter.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/EntityProperties.h"
#include "Model/Issue.h"
#include "Model/Map.h"
#include "Model/Object.h"

#include <cassert>
#include <fstream>

namespace TrenchBroom {
    namespace IO {
        MapWriter::~MapWriter() {}
        
        void MapWriter::writeObjectsToStream(const Model::ObjectList& objects, std::ostream& stream) {
            assert(stream.good());
            stream.unsetf(std::ios::floatfield);
            
            Model::Entity* worldspawn = NULL;
            
            Model::EntityList pointEntities;
            Model::EntityBrushesMap brushEntities;
            
            Model::ObjectList::const_iterator oIt, oEnd;
            for (oIt = objects.begin(), oEnd = objects.end(); oIt != oEnd; ++oIt) {
                Model::Object* object = *oIt;
                if (object->type() == Model::Object::OTEntity) {
                    pointEntities.push_back(static_cast<Model::Entity*>(object));
                } else if (object->type() == Model::Object::OTBrush) {
                    Model::Brush* brush = static_cast<Model::Brush*>(object);
                    Model::Entity* entity = brush->parent();
                    brushEntities[entity].push_back(brush);
                    if (entity->worldspawn())
                        worldspawn = entity;
                }
            }
            
            // write worldspawn first
            Model::BrushList::const_iterator bIt, bEnd;
            if (worldspawn != NULL)
                writeEntity(*worldspawn, brushEntities[worldspawn], stream);
            
            // write point entities
            Model::EntityList::const_iterator eIt, eEnd;
            for (eIt = pointEntities.begin(), eEnd = pointEntities.end(); eIt != eEnd; ++eIt) {
                Model::Entity* entity = *eIt;
                const Model::BrushList& brushes = entity->brushes(); // should be empty, but you never know
                writeEntity(*entity, brushes, stream);
            }
            
            // write brush entities except for worldspawn
            Model::EntityBrushesMap::const_iterator ebIt, ebEnd;
            for (ebIt = brushEntities.begin(), ebEnd = brushEntities.end(); ebIt != ebEnd; ++ebIt) {
                Model::Entity* entity = ebIt->first;
                if (entity != worldspawn) {
                    const Model::BrushList& brushes = ebIt->second;
                    writeEntity(*entity, brushes, stream);
                }
            }
        }
        
        void MapWriter::writeFacesToStream(const Model::BrushFaceList& faces, std::ostream& stream) {
            assert(stream.good());
            stream.unsetf(std::ios::floatfield);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                writeFace(**it, stream);
        }
        
        void MapWriter::writeToStream(const Model::Map& map, std::ostream& stream) {
            assert(stream.good());
            stream.unsetf(std::ios::floatfield);
            
            const Model::EntityList& entities = map.entities();
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                const Model::BrushList& brushes = entity->brushes();
                writeEntity(*entity, brushes, stream);
            }
        }
        
        void MapWriter::writeToFileAtPath(Model::Map& map, const Path& path, const bool overwrite) {
            if (IO::Disk::fileExists(IO::Disk::fixPath(path)) && !overwrite)
                throw FileSystemException("File already exists: " + path.asString());
            
            const Path directoryPath = path.deleteLastComponent();
            WritableDiskFileSystem fs(directoryPath, true); // ensures that the directory actually exists or is created if it doesn't
            
            FILE* stream = fopen(path.asString().c_str(), "w");
            if (stream == NULL)
                throw FileSystemException("Cannot open file: " + path.asString());
            
            try {
                size_t lineNumber = 1;
                const Model::EntityList& entities = map.entities();
                Model::EntityList::const_iterator it, end;
                for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                    Model::Entity* entity = *it;
                    lineNumber += writeEntity(*entity, entity->brushes(), lineNumber, stream);
                }
            } catch (...) {
                fclose(stream);
                throw;
            }
            fclose(stream);
        }

        size_t MapWriter::writeEntity(Model::Entity& entity, const Model::BrushList& brushes, const size_t lineNumber, FILE* stream) {
            size_t lineCount = 0;
            lineCount += writeEntityHeader(entity, stream);
            
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                lineCount += writeBrush(**it, lineNumber + lineCount, stream);
            
            lineCount += writeEntityFooter(stream);
            return lineCount;
        }

        size_t MapWriter::writeBrush(Model::Brush& brush, const size_t lineNumber, FILE* stream) {
            size_t lineCount = 0;
            std::fprintf(stream, "{\n"); ++lineCount;
            
            const Model::BrushFaceList& faces = brush.faces();
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                lineCount += writeFace(**it, lineNumber + lineCount, stream);
            
            std::fprintf(stream, "}\n"); ++lineCount;

            brush.setFilePosition(lineNumber, lineCount);
            return lineCount;
        }
        
        size_t MapWriter::writeEntityHeader(Model::Entity& entity, FILE* stream) {
            size_t lineCount = 0;
            std::fprintf(stream, "{\n"); ++lineCount;
            lineCount += writeExtraProperties(entity, stream);
            
            const Model::EntityProperty::List& properties = entity.properties();
            Model::EntityProperty::List::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it)
                lineCount += writeEntityProperty(*it, stream);
            return lineCount;
        }

        size_t MapWriter::writeEntityProperty(const Model::EntityProperty& property, FILE* stream) {
            std::fprintf(stream, "\"%s\" \"%s\"\n", property.key.c_str(), property.value.c_str());;
            return 1;
        }

        size_t MapWriter::writeEntityFooter(FILE* stream) {
            std::fprintf(stream, "}\n");
            return 1;
        }

        size_t MapWriter::writeExtraProperties(const Model::Object& object, FILE* stream) {
            const Model::IssueType ignoredIssues = object.ignoredIssues();
            if (ignoredIssues == 0)
                return 0;
            std::fprintf(stream, "/// ignoreIssues %lu\n", ignoredIssues);
            return 1;
        }

        void MapWriter::writeEntity(const Model::Entity& entity, const Model::BrushList& brushes, std::ostream& stream) {
            writeEntityHeader(entity, stream);
            Model::BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it)
                writeBrush(**it, stream);
            writeEntityFooter(stream);
        }

        void MapWriter::writeBrush(const Model::Brush& brush, std::ostream& stream) {
            stream << "{\n";
            const Model::BrushFaceList& faces = brush.faces();
            Model::BrushFaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it)
                writeFace(**it, stream);
            stream << "}\n";
        }
        
        void MapWriter::writeEntityHeader(const Model::Entity& entity, std::ostream& stream) {
            stream << "{\n";
            writeExtraProperties(entity, stream);
            
            const Model::EntityProperty::List& properties = entity.properties();
            Model::EntityProperty::List::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it)
                writeEntityProperty(*it, stream);
        }
        
        void MapWriter::writeEntityProperty(const Model::EntityProperty& property, std::ostream& stream) {
            stream << "\"" << property.key << "\" \"" << property.value << "\"" << "\n";
        }

        void MapWriter::writeEntityFooter(std::ostream& stream) {
            stream << "}\n";
        }

        void MapWriter::writeExtraProperties(const Model::Object& object, std::ostream& stream) {
            const Model::IssueType ignoredIssues = object.ignoredIssues();
            if (ignoredIssues != 0)
                stream << "/// ignoreIssues " << ignoredIssues << "\n";
        }
    }
}
