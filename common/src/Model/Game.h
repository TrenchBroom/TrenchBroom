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

#ifndef TrenchBroom_Game
#define TrenchBroom_Game

#include "TrenchBroom.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/EntityDefinitionLoader.h"
#include "IO/EntityModelLoader.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Model/Model_Forward.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class TextureManager;
    }

    namespace Model {
        class SmartTag;

        class Game : public IO::EntityDefinitionLoader, public IO::EntityModelLoader {
        public:
            enum class TexturePackageType {
                File,
                Directory
            };
        public:
            const std::string& gameName() const;
            bool isGamePathPreference(const IO::Path& prefPath) const;

            IO::Path gamePath() const;
            void setGamePath(const IO::Path& gamePath, Logger& logger);
            void setAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger& logger);

            using PathErrors = std::map<IO::Path, std::string>;
            PathErrors checkAdditionalSearchPaths(const IO::Path::List& searchPaths) const;

            CompilationConfig& compilationConfig();

            size_t maxPropertyLength() const;

            const std::vector<SmartTag>& smartTags() const;
        public: // loading and writing map files
            std::unique_ptr<World> newMap(MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const;
            std::unique_ptr<World> loadMap(MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const;
            void writeMap(World& world, const IO::Path& path) const;
            void exportMap(World& world, Model::ExportFormat format, const IO::Path& path) const;
        public: // parsing and serializing objects
            std::vector<Node*> parseNodes(const std::string& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const;
            std::vector<BrushFace*> parseBrushFaces(const std::string& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const;

            void writeNodesToStream(World& world, const std::vector<Node*>& nodes, std::ostream& stream) const;
            void writeBrushFacesToStream(World& world, const std::vector<BrushFace*>& faces, std::ostream& stream) const;
        public: // texture collection handling
            TexturePackageType texturePackageType() const;
            void loadTextureCollections(AttributableNode& node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const;
            bool isTextureCollection(const IO::Path& path) const;
            IO::Path::List findTextureCollections() const;
            IO::Path::List extractTextureCollections(const AttributableNode& node) const;
            void updateTextureCollections(AttributableNode& node, const IO::Path::List& paths) const;
            void reloadShaders();
        public: // entity definition handling
            bool isEntityDefinitionFile(const IO::Path& path) const;
            Assets::EntityDefinitionFileSpec::List allEntityDefinitionFiles() const;
            Assets::EntityDefinitionFileSpec extractEntityDefinitionFile(const AttributableNode& node) const;
            IO::Path findEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const;
        public: // mods
            std::vector<std::string> availableMods() const;
            std::vector<std::string> extractEnabledMods(const AttributableNode& node) const;
            std::string defaultMod() const;
        public: // flag configs for faces
            const GameConfig::FlagsConfig& surfaceFlags() const;
            const GameConfig::FlagsConfig& contentFlags() const;
        private: // subclassing interface
            virtual const std::string& doGameName() const = 0;
            virtual IO::Path doGamePath() const = 0;
            virtual void doSetGamePath(const IO::Path& gamePath, Logger& logger) = 0;
            virtual void doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger& logger) = 0;
            virtual PathErrors doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const = 0;

            virtual CompilationConfig& doCompilationConfig() = 0;
            virtual size_t doMaxPropertyLength() const = 0;

            virtual const std::vector<SmartTag>& doSmartTags() const = 0;

            virtual std::unique_ptr<World> doNewMap(MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const = 0;
            virtual std::unique_ptr<World> doLoadMap(MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const = 0;
            virtual void doWriteMap(World& world, const IO::Path& path) const = 0;
            virtual void doExportMap(World& world, Model::ExportFormat format, const IO::Path& path) const = 0;

            virtual std::vector<Node*> doParseNodes(const std::string& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const = 0;
            virtual std::vector<BrushFace*> doParseBrushFaces(const std::string& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const = 0;
            virtual void doWriteNodesToStream(World& world, const std::vector<Node*>& nodes, std::ostream& stream) const = 0;
            virtual void doWriteBrushFacesToStream(World& world, const std::vector<BrushFace*>& faces, std::ostream& stream) const = 0;

            virtual TexturePackageType doTexturePackageType() const = 0;
            virtual void doLoadTextureCollections(AttributableNode& node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const = 0;
            virtual bool doIsTextureCollection(const IO::Path& path) const = 0;
            virtual IO::Path::List doFindTextureCollections() const = 0;
            virtual IO::Path::List doExtractTextureCollections(const AttributableNode& node) const = 0;
            virtual void doUpdateTextureCollections(AttributableNode& node, const IO::Path::List& paths) const = 0;
            virtual void doReloadShaders() = 0;

            virtual bool doIsEntityDefinitionFile(const IO::Path& path) const = 0;
            virtual Assets::EntityDefinitionFileSpec::List doAllEntityDefinitionFiles() const = 0;
            virtual Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(const AttributableNode& node) const = 0;
            virtual IO::Path doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const = 0;

            virtual std::vector<std::string> doAvailableMods() const = 0;
            virtual std::vector<std::string> doExtractEnabledMods(const AttributableNode& node) const = 0;
            virtual std::string doDefaultMod() const = 0;

            virtual const GameConfig::FlagsConfig& doSurfaceFlags() const = 0;
            virtual const GameConfig::FlagsConfig& doContentFlags() const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Game) */
