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

#include "GameImpl.h"

#include "Assets/Palette.h"
#include "Assets/TextureCollectionSpec.h"
#include "IO/BrushFaceReader.h"
#include "IO/Bsp29Parser.h"
#include "IO/DefParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/FgdParser.h"
#include "IO/FileSystem.h"
#include "IO/IOUtils.h"
#include "IO/MapParser.h"
#include "IO/MdlParser.h"
#include "IO/Md2Parser.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/WorldReader.h"
#include "IO/SystemPaths.h"
#include "IO/WadTextureLoader.h"
#include "IO/WalTextureLoader.h"
#include "Model/EntityAttributes.h"
#include "Model/World.h"

#include "Exceptions.h"

#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        GameImpl::GameImpl(const GameConfig& config, const IO::Path& gamePath) :
        m_config(config),
        m_gamePath(gamePath),
        m_fs(m_config.fileSystemConfig().packageFormat,
             m_gamePath,
             m_config.fileSystemConfig().searchPath,
             m_additionalSearchPaths),
        m_palette(new Assets::Palette(config.findConfigFile(config.textureConfig().palette))) {}
        
        GameImpl::~GameImpl() {
            delete m_palette;
            m_palette = NULL;
        }

        const String& GameImpl::doGameName() const {
            return m_config.name();
        }

        IO::Path GameImpl::doGamePath() const {
            return m_gamePath;
        }

        void GameImpl::doSetGamePath(const IO::Path& gamePath) {
            m_gamePath = gamePath;
            m_fs = IO::GameFileSystem(m_config.fileSystemConfig().packageFormat,
                                      m_gamePath,
                                      m_config.fileSystemConfig().searchPath,
                                      m_additionalSearchPaths);
        }
        
        void GameImpl::doSetAdditionalSearchPaths(const IO::Path::List& searchPaths) {
            m_additionalSearchPaths = searchPaths;
            m_fs = IO::GameFileSystem(m_config.fileSystemConfig().packageFormat,
                                      m_gamePath,
                                      m_config.fileSystemConfig().searchPath,
                                      m_additionalSearchPaths);
        }

        World* GameImpl::doNewMap(const MapFormat::Type format, const BBox3& worldBounds) const {
            return new World(format, brushContentTypeBuilder(), worldBounds);
        }
        
        World* GameImpl::doLoadMap(const MapFormat::Type format, const BBox3& worldBounds, const IO::Path& path, Logger* logger) const {
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::WorldReader reader(file->begin(), file->end(), brushContentTypeBuilder(), logger);
            return reader.read(format, worldBounds);
        }
        
        void GameImpl::doWriteMap(World* world, const IO::Path& path) const {
            const String mapFormatName = formatName(world->format());
            
            IO::OpenFile openFile(path, true);
            FILE* stream = openFile.file();
            IO::writeGameComment(stream, gameName(), mapFormatName);
            
            IO::NodeWriter writer(world, stream);
            writer.writeMap();
        }

        NodeList GameImpl::doParseNodes(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const {
            IO::NodeReader reader(str, world, logger);
            return reader.read(worldBounds);
        }
        
        BrushFaceList GameImpl::doParseBrushFaces(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const {
            IO::BrushFaceReader reader(str, world, logger);
            return reader.read(worldBounds);
        }
        
        void GameImpl::doWriteNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }
    
        void GameImpl::doWriteBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }
    
        bool GameImpl::doIsTextureCollection(const IO::Path& path) const {
            const String& type = m_config.textureConfig().type;
            if (type == "wad")
                return StringUtils::caseInsensitiveEqual(path.extension(), "wad");
            if (type == "wal")
                return IO::Disk::directoryExists(path);
            return false;
        }

        IO::Path::List GameImpl::doFindBuiltinTextureCollections() const {
            try {
                const IO::Path& searchPath = m_config.textureConfig().builtinTexturesSearchPath;
                if (!searchPath.isEmpty())
                    return m_fs.findItems(searchPath, IO::FileSystem::TypeMatcher(false, true));
                return IO::Path::List();
            } catch (FileSystemException& e) {
                throw GameException("Cannot find builtin textures: " + String(e.what()));
            }
        }
        
        StringList GameImpl::doExtractExternalTextureCollections(const World* world) const {
            assert(world != NULL);
            
            const String& property = m_config.textureConfig().attribute;
            if (property.empty())
                return EmptyStringList;
            
            const AttributeValue& pathsValue = world->attribute(property);
            if (pathsValue.empty())
                return EmptyStringList;
            
            return StringUtils::splitAndTrim(pathsValue, ';');
        }
        
        void GameImpl::doUpdateExternalTextureCollections(World* world, const StringList& collections) const {
            const String& attribute = m_config.textureConfig().attribute;
            if (attribute.empty())
                return;
            
            const String value = StringUtils::join(collections, ';');
            // to avoid backslashes being misinterpreted as escape sequences
            const String formatted = StringUtils::replaceAll(value, "\\", "/");
            world->addOrUpdateAttribute(attribute, formatted);
        }

        Assets::TextureCollection* GameImpl::doLoadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            const String& type = m_config.textureConfig().type;
            if (type == "wad")
                return loadWadTextureCollection(spec);
            if (type == "wal")
                return loadWalTextureCollection(spec);
            throw GameException("Unknown texture collection type '" + type + "'");
        }
        
        bool GameImpl::doIsEntityDefinitionFile(const IO::Path& path) const {
            const String extension = path.extension();
            if (StringUtils::caseInsensitiveEqual("fgd", extension))
                return true;
            if (StringUtils::caseInsensitiveEqual("def", extension))
                return true;
            return false;
        }

        Assets::EntityDefinitionList GameImpl::doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const {
            const String extension = path.extension();
            const Color& defaultColor = m_config.entityConfig().defaultColor;
            
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::FgdParser parser(file->begin(), file->end(), defaultColor);
                return parser.parseDefinitions(status);
            }
            if (StringUtils::caseInsensitiveEqual("def", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::DefParser parser(file->begin(), file->end(), defaultColor);
                return parser.parseDefinitions(status);
            }
            throw GameException("Unknown entity definition format: '" + path.asString() + "'");
        }
        
        Assets::EntityDefinitionFileSpec::List GameImpl::doAllEntityDefinitionFiles() const {
            const IO::Path::List paths = m_config.entityConfig().defFilePaths;
            const size_t count = paths.size();

            Assets::EntityDefinitionFileSpec::List result(count);
            for (size_t i = 0; i < count; ++i)
                result[i] = Assets::EntityDefinitionFileSpec::builtin(paths[i]);
            
            return result;
        }

        Assets::EntityDefinitionFileSpec GameImpl::doExtractEntityDefinitionFile(const World* world) const {
            const AttributeValue& defValue = world->attribute(AttributeNames::EntityDefinitions);
            if (defValue.empty())
                return defaultEntityDefinitionFile();
            return Assets::EntityDefinitionFileSpec::parse(defValue);
        }
        
        Assets::EntityDefinitionFileSpec GameImpl::defaultEntityDefinitionFile() const {
            const IO::Path::List paths = m_config.entityConfig().defFilePaths;
            if (paths.empty())
                throw GameException("No entity definition files found for game '" + gameName() + "'");
            
            const IO::Path& path = paths.front();
            return Assets::EntityDefinitionFileSpec::builtin(path);
        }

        IO::Path GameImpl::doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const {
            if (!spec.valid())
                throw GameException("Invalid entity definition file spec");

            const IO::Path& path = spec.path();
            if (spec.builtin()) {
                return m_config.findConfigFile(path);
            } else {
                if (path.isAbsolute())
                    return path;
                return IO::Disk::resolvePath(searchPaths, path);
            }
        }

        Assets::EntityModel* GameImpl::doLoadEntityModel(const IO::Path& path) const {
            try {
                const IO::MappedFile::Ptr file = m_fs.openFile(path);
                assert(file != NULL);
                
                const String modelName = path.lastComponent().asString();
                const String extension = StringUtils::toLower(path.extension());
                const StringSet supported = m_config.entityConfig().modelFormats;
                
                if (extension == "mdl" && supported.count("mdl") > 0)
                    return loadMdlModel(modelName, file);
                if (extension == "md2" && supported.count("md2") > 0)
                    return loadMd2Model(modelName, file);
                if (extension == "bsp" && supported.count("bsp") > 0)
                    return loadBspModel(modelName, file);
                throw GameException("Unsupported model format '" + path.asString() + "'");
            } catch (FileSystemException& e) {
                throw GameException("Cannot load model: " + String(e.what()));
            }
        }
        
        Assets::TextureCollection* GameImpl::loadWadTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            assert(m_palette != NULL);
            
            IO::WadTextureLoader loader(*m_palette);
            return loader.loadTextureCollection(spec);
        }
        
        Assets::TextureCollection* GameImpl::loadWalTextureCollection(const Assets::TextureCollectionSpec& spec) const {
            assert(m_palette != NULL);
            
            const IO::Path& path = spec.path();
            if (path.isAbsolute()) {
                IO::DiskFileSystem diskFS(path.deleteLastComponent());
                IO::WalTextureLoader loader(diskFS, *m_palette);
                const Assets::TextureCollectionSpec newSpec(spec.name(), path.lastComponent());
                return loader.loadTextureCollection(newSpec);
            } else {
                IO::WalTextureLoader loader(m_fs, *m_palette);
                return loader.loadTextureCollection(spec);
            }
        }
        
        Assets::EntityModel* GameImpl::loadBspModel(const String& name, const IO::MappedFile::Ptr& file) const {
            assert(m_palette != NULL);
            
            IO::Bsp29Parser parser(name, file->begin(), file->end(), *m_palette);
            return parser.parseModel();
        }
        
        Assets::EntityModel* GameImpl::loadMdlModel(const String& name, const IO::MappedFile::Ptr& file) const {
            assert(m_palette != NULL);
            
            IO::MdlParser parser(name, file->begin(), file->end(), *m_palette);
            return parser.parseModel();
        }
        
        Assets::EntityModel* GameImpl::loadMd2Model(const String& name, const IO::MappedFile::Ptr& file) const {
            assert(m_palette != NULL);
            
            IO::Md2Parser parser(name, file->begin(), file->end(), *m_palette, m_fs);
            return parser.parseModel();
        }

        const BrushContentType::List& GameImpl::doBrushContentTypes() const {
            return m_config.brushContentTypes();
        }

        StringList GameImpl::doAvailableMods() const {
            StringList result;
            if (m_gamePath.isEmpty() || !IO::Disk::directoryExists(m_gamePath))
                return result;
            
            const String& defaultMod = m_config.fileSystemConfig().searchPath.lastComponent().asString();
            const IO::DiskFileSystem fs(m_gamePath);
            const IO::Path::List subDirs = fs.findItems(IO::Path(""), IO::FileSystem::TypeMatcher(false, true));
            for (size_t i = 0; i < subDirs.size(); ++i) {
                const String mod = subDirs[i].lastComponent().asString();
                if (!StringUtils::caseInsensitiveEqual(mod, defaultMod))
                    result.push_back(mod);
            }
            return result;
        }

        StringList GameImpl::doExtractEnabledMods(const World* world) const {
            StringList mods;
            const AttributeValue& modStr = world->attribute(AttributeNames::Mods);
            if (modStr.empty())
                return mods;
            
            return StringUtils::splitAndTrim(modStr, ';');
        }
        
        const GameConfig::FlagsConfig& GameImpl::doSurfaceFlags() const {
            return m_config.faceAttribsConfig().surfaceFlags;
        }
        
        const GameConfig::FlagsConfig& GameImpl::doContentFlags() const {
            return m_config.faceAttribsConfig().contentFlags;
        }
    }
}
