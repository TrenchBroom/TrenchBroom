/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_GameImpl
#define TrenchBroom_GameImpl

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/FileSystemHierarchy.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/ModelTypes.h"


namespace TrenchBroom {
    class Logger;
    
    namespace Model {
        class GameImpl : public Game {
        private:
            typedef std::shared_ptr<IO::MapWriter> MapWriterPtr;
            
            GameConfig& m_config;
            IO::Path m_gamePath;
            IO::Path::Array m_additionalSearchPaths;
            
            IO::FileSystemHierarchy m_gameFS;
        public:
            GameImpl(GameConfig& config, const IO::Path& gamePath);
        private:
            void initializeFileSystem();
            void addPackages(const IO::Path& searchPath);
        private:
            const String& doGameName() const;
            IO::Path doGamePath() const;
            void doSetGamePath(const IO::Path& gamePath);
            void doSetAdditionalSearchPaths(const IO::Path::Array& searchPaths);

            CompilationConfig& doCompilationConfig();

            size_t doMaxPropertyLength() const;

            World* doNewMap(MapFormat::Type format, const BBox3& worldBounds) const;
            World* doLoadMap(MapFormat::Type format, const BBox3& worldBounds, const IO::Path& path, Logger* logger) const;
            void doWriteMap(World* world, const IO::Path& path) const;
            void doExportMap(World* world, Model::ExportFormat format, const IO::Path& path) const;

            NodeArray doParseNodes(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const;
            BrushFaceArray doParseBrushFaces(const String& str, World* world, const BBox3& worldBounds, Logger* logger) const;
            
            void doWriteNodesToStream(World* world, const Model::NodeArray& nodes, std::ostream& stream) const;
            void doWriteBrushFacesToStream(World* world, const BrushFaceArray& faces, std::ostream& stream) const;
            
            TexturePackageType doTexturePackageType() const;
            void doLoadTextureCollections(World* world, const IO::Path& documentPath, Assets::TextureManager& textureManager) const;
            IO::Path::Array textureCollectionSearchPaths(const IO::Path& documentPath) const;
            
            bool doIsTextureCollection(const IO::Path& path) const;
            IO::Path::Array doFindTextureCollections() const;
            IO::Path::Array doExtractTextureCollections(const World* world) const;
            void doUpdateTextureCollections(World* world, const IO::Path::Array& paths) const;
            
            bool doIsEntityDefinitionFile(const IO::Path& path) const;
            Assets::EntityDefinitionArray doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const;
            Assets::EntityDefinitionFileSpec::Array doAllEntityDefinitionFiles() const;
            Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const World* world) const;
            Assets::EntityDefinitionFileSpec defaultEntityDefinitionFile() const;
            IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::Array& searchPaths) const;
            Assets::EntityModel* doLoadEntityModel(const IO::Path& path) const;

            Assets::EntityModel* loadBspModel(const String& name, const IO::MappedFile::Ptr& file) const;
            Assets::EntityModel* loadMdlModel(const String& name, const IO::MappedFile::Ptr& file) const;
            Assets::EntityModel* loadMd2Model(const String& name, const IO::MappedFile::Ptr& file) const;
            Assets::Palette loadTexturePalette() const;
            
            const BrushContentType::Array& doBrushContentTypes() const;

            StringArray doAvailableMods() const;
            StringArray doExtractEnabledMods(const World* world) const;
            String doDefaultMod() const;

            const GameConfig::FlagsConfig& doSurfaceFlags() const;
            const GameConfig::FlagsConfig& doContentFlags() const;
        private:
            void writeLongAttribute(AttributableNode* node, const AttributeName& baseName, const AttributeValue& value, size_t maxLength) const;
            String readLongAttribute(const AttributableNode* node, const AttributeName& baseName) const;
        };
    }
}

#endif /* defined(TrenchBroom_GameImpl) */
