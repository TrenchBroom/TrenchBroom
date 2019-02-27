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

#include "GameImpl.h"

#include "Macros.h"
#include "Assets/Palette.h"
#include "IO/BrushFaceReader.h"
#include "IO/Bsp29Parser.h"
#include "IO/DefParser.h"
#include "IO/DkmParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/EntParser.h"
#include "IO/FgdParser.h"
#include "IO/FileMatcher.h"
#include "IO/FileSystem.h"
#include "IO/IOUtils.h"
#include "IO/MapParser.h"
#include "IO/MdlParser.h"
#include "IO/Md2Parser.h"
#include "IO/Md3Parser.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/ObjSerializer.h"
#include "IO/WorldReader.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "IO/TextureLoader.h"
#include "IO/ZipFileSystem.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/EntityAttributes.h"
#include "Model/GameConfig.h"
#include "Model/Layer.h"
#include "Model/World.h"

#include "Exceptions.h"

#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        GameImpl::GameImpl(GameConfig& config, const IO::Path& gamePath, Logger& logger) :
        m_config(config),
        m_gamePath(gamePath) {
            initializeFileSystem(logger);
        }

        void GameImpl::initializeFileSystem(Logger& logger) {
            m_fs.initialize(m_config, m_gamePath, m_additionalSearchPaths, logger);
        }

        const String& GameImpl::doGameName() const {
            return m_config.name();
        }

        IO::Path GameImpl::doGamePath() const {
            return m_gamePath;
        }

        void GameImpl::doSetGamePath(const IO::Path& gamePath, Logger& logger) {
            if (gamePath != m_gamePath) {
                m_gamePath = gamePath;
                initializeFileSystem(logger);
            }
        }

        void GameImpl::doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger& logger) {
            if (searchPaths != m_additionalSearchPaths) {
                m_additionalSearchPaths = searchPaths;
                initializeFileSystem(logger);
            }
        }

        Game::PathErrors GameImpl::doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const {
            PathErrors result;
            for (const auto& searchPath : searchPaths) {
                const auto absPath = m_gamePath + searchPath;
                if (!absPath.isAbsolute() || !IO::Disk::directoryExists(absPath)) {
                    result.insert(std::make_pair(searchPath, "Directory not found: '" + searchPath.asString() + "'"));
                }
            }
            return result;
        }

        CompilationConfig& GameImpl::doCompilationConfig() {
            return m_config.compilationConfig();
        }

        size_t GameImpl::doMaxPropertyLength() const {
            return m_config.maxPropertyLength();
        }

        const std::vector<SmartTag>& GameImpl::doSmartTags() const {
            return m_config.smartTags();
        }

        std::unique_ptr<World> GameImpl::doNewMap(const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const {
            const auto initialMapFilePath = m_config.findInitialMap(formatName(format));
            if (!initialMapFilePath.isEmpty() && IO::Disk::fileExists(initialMapFilePath)) {
                return doLoadMap(format, worldBounds, initialMapFilePath, logger);
            } else {
                auto world = std::make_unique<World>(format, worldBounds);

                const Model::BrushBuilder builder(world.get(), worldBounds);
                auto* brush = builder.createCuboid(vm::vec3(128.0, 128.0, 32.0), Model::BrushFace::NoTextureName);
                world->defaultLayer()->addChild(brush);

                if (format == MapFormat::Valve) {
                    world->addOrUpdateAttribute(AttributeNames::ValveVersion, "220");
                }

                return world;
            }
        }

        std::unique_ptr<World> GameImpl::doLoadMap(const MapFormat format, const vm::bbox3& worldBounds, const IO::Path& path, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            const auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::WorldReader reader(file->begin(), file->end());
            return reader.read(format, worldBounds, parserStatus);
        }

        void GameImpl::doWriteMap(World& world, const IO::Path& path) const {
            const auto mapFormatName = formatName(world.format());

            IO::OpenFile open(path, true);
            IO::writeGameComment(open.file, gameName(), mapFormatName);

            IO::NodeWriter writer(world, open.file);
            writer.writeMap();
        }

        void GameImpl::doExportMap(World& world, const Model::ExportFormat format, const IO::Path& path) const {
            IO::OpenFile open(path, true);

            switch (format) {
                case Model::WavefrontObj:
                    IO::NodeWriter(world, new IO::ObjFileSerializer(open.file)).writeMap();
                    break;
            }
        }

        NodeList GameImpl::doParseNodes(const String& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::NodeReader reader(str, world);
            return reader.read(worldBounds, parserStatus);
        }

        BrushFaceList GameImpl::doParseBrushFaces(const String& str, World& world, const vm::bbox3& worldBounds, Logger& logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::BrushFaceReader reader(str, world);
            return reader.read(worldBounds, parserStatus);
        }

        void GameImpl::doWriteNodesToStream(World& world, const Model::NodeList& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }

        void GameImpl::doWriteBrushFacesToStream(World& world, const BrushFaceList& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }

        Game::TexturePackageType GameImpl::doTexturePackageType() const {
            using Model::GameConfig;
            switch (m_config.textureConfig().package.type) {
                case GameConfig::TexturePackageConfig::PT_File:
                    return TexturePackageType::File;
                case GameConfig::TexturePackageConfig::PT_Directory:
                    return TexturePackageType::Directory;
                case GameConfig::TexturePackageConfig::PT_Unset:
                    throw GameException("Texture package type is not set in game configuration");
                switchDefault()
            }
        }

        void GameImpl::doLoadTextureCollections(AttributableNode& node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger& logger) const {
            const auto paths = extractTextureCollections(node);

            const auto fileSearchPaths = textureCollectionSearchPaths(documentPath);
            IO::TextureLoader textureLoader(m_fs, fileSearchPaths, m_config.textureConfig(), logger);
            textureLoader.loadTextures(paths, textureManager);
        }

        IO::Path::List GameImpl::textureCollectionSearchPaths(const IO::Path& documentPath) const {
            IO::Path::List result;

            // Search for assets relative to the map file.
            result.push_back(documentPath);

            // Search for assets relative to the location of the game.
            result.push_back(m_gamePath);

            // Search for assets relative to the application itself.
            result.push_back(IO::SystemPaths::appDirectory());

            return result;
        }

        bool GameImpl::doIsTextureCollection(const IO::Path& path) const {
            const auto& packageConfig = m_config.textureConfig().package;
            switch (packageConfig.type) {
                case GameConfig::TexturePackageConfig::PT_File:
                    return path.hasExtension(packageConfig.fileFormat.extensions, false);
                case GameConfig::TexturePackageConfig::PT_Directory:
                case GameConfig::TexturePackageConfig::PT_Unset:
                    return false;
                switchDefault()
            }
        }

        IO::Path::List GameImpl::doFindTextureCollections() const {
            try {
                const auto& searchPath = m_config.textureConfig().package.rootDirectory;
                if (!searchPath.isEmpty() && m_fs.directoryExists(searchPath)) {
                    return m_fs.findItems(searchPath, IO::FileTypeMatcher(false, true));
                }
                return IO::Path::List();
            } catch (FileSystemException& e) {
                throw GameException("Could not find texture collections: " + String(e.what()));
            }
        }

        IO::Path::List GameImpl::doExtractTextureCollections(const AttributableNode& node) const {
            const auto& property = m_config.textureConfig().attribute;
            if (property.empty()) {
                return IO::Path::List(0);
            }

            const auto& pathsValue = node.attribute(property);
            if (pathsValue.empty()) {
                return IO::Path::List(0);
            }

            return IO::Path::asPaths(StringUtils::splitAndTrim(pathsValue, ';'));
        }

        void GameImpl::doUpdateTextureCollections(AttributableNode& node, const IO::Path::List& paths) const {
            const auto& attribute = m_config.textureConfig().attribute;
            if (attribute.empty()) {
                return;
            }

            const auto value = StringUtils::join(IO::Path::asStrings(paths, '/'), ';');
            node.addOrUpdateAttribute(attribute, value);
        }

        void GameImpl::doReloadShaders() {
            m_fs.reloadShaders();
        }

        bool GameImpl::doIsEntityDefinitionFile(const IO::Path& path) const {
            const auto extension = path.extension();
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                return true;
            } else if (StringUtils::caseInsensitiveEqual("def", extension)) {
                return true;
            } else {
                return false;
            }
        }

        Assets::EntityDefinitionList GameImpl::doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const {
            const auto extension = path.extension();
            const auto& defaultColor = m_config.entityConfig().defaultColor;

            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                const auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::FgdParser parser(file->begin(), file->end(), defaultColor, file->path());
                return parser.parseDefinitions(status);
            } else if (StringUtils::caseInsensitiveEqual("def", extension)) {
                const auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::DefParser parser(file->begin(), file->end(), defaultColor);
                return parser.parseDefinitions(status);
            } else if (StringUtils::caseInsensitiveEqual("ent", extension)) {
                const auto file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::EntParser parser(file->begin(), file->end(), defaultColor);
                return parser.parseDefinitions(status);
            } else {
                throw GameException("Unknown entity definition format: '" + path.asString() + "'");
            }
        }

        Assets::EntityDefinitionFileSpec::List GameImpl::doAllEntityDefinitionFiles() const {
            const auto paths = m_config.entityConfig().defFilePaths;
            const auto count = paths.size();

            Assets::EntityDefinitionFileSpec::List result(count);
            for (size_t i = 0; i < count; ++i) {
                result[i] = Assets::EntityDefinitionFileSpec::builtin(paths[i]);
            }

            return result;
        }

        Assets::EntityDefinitionFileSpec GameImpl::doExtractEntityDefinitionFile(const AttributableNode& node) const {
            const auto& defValue = node.attribute(AttributeNames::EntityDefinitions);
            if (defValue.empty()) {
                return defaultEntityDefinitionFile();
            }
            return Assets::EntityDefinitionFileSpec::parse(defValue);
        }

        Assets::EntityDefinitionFileSpec GameImpl::defaultEntityDefinitionFile() const {
            const auto paths = m_config.entityConfig().defFilePaths;
            if (paths.empty()) {
                throw GameException("No entity definition files found for game '" + gameName() + "'");
            }

            const auto& path = paths.front();
            return Assets::EntityDefinitionFileSpec::builtin(path);
        }

        IO::Path GameImpl::doFindEntityDefinitionFile(const Assets::EntityDefinitionFileSpec& spec, const IO::Path::List& searchPaths) const {
            if (!spec.valid()) {
                throw GameException("Invalid entity definition file spec");
            }

            const auto& path = spec.path();
            if (spec.builtin()) {
                return m_config.findConfigFile(path);
            } else {
                if (path.isAbsolute()) {
                    return path;
                } else {
                    return IO::Disk::resolvePath(searchPaths, path);
                }
            }
        }

        Assets::EntityModel* GameImpl::doLoadEntityModel(const IO::Path& path, Logger& logger) const {
            try {
                const auto file = m_fs.openFile(path);
                ensure(file.get() != nullptr, "file is null");

                const auto modelName = path.lastComponent().asString();
                const auto extension = StringUtils::toLower(path.extension());
                const auto supported = m_config.entityConfig().modelFormats;

                if (extension == "mdl" && supported.count("mdl") > 0) {
                    return loadMdlModel(modelName, file, logger);
                } else if (extension == "md2" && supported.count("md2") > 0) {
                    return loadMd2Model(modelName, file, logger);
                } else if (extension == "md3" && supported.count("md3") > 0) {
                    return loadMd3Model(modelName, file, logger);
                } else if (extension == "bsp" && supported.count("bsp") > 0) {
                    return loadBspModel(modelName, file, logger);
                } else if (extension == "dkm" && supported.count("dkm") > 0) {
                    return loadDkmModel(modelName, file, logger);
                } else {
                    throw GameException("Unsupported model format '" + path.asString() + "'");
                }
            } catch (FileSystemException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + String(e.what()));
            } catch (AssetException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + String(e.what()));
            }
        }

        Assets::EntityModel* GameImpl::loadBspModel(const String& name, const IO::MappedFile::Ptr& file, Logger& logger) const {
            const auto palette = loadTexturePalette();

            IO::Bsp29Parser parser(name, file->begin(), file->end(), palette);
            return parser.parseModel(logger);
        }

        Assets::EntityModel* GameImpl::loadMdlModel(const String& name, const IO::MappedFile::Ptr& file, Logger& logger) const {
            const auto palette = loadTexturePalette();

            IO::MdlParser parser(name, file->begin(), file->end(), palette);
            return parser.parseModel(logger);
        }

        Assets::EntityModel* GameImpl::loadMd2Model(const String& name, const IO::MappedFile::Ptr& file, Logger& logger) const {
            const auto palette = loadTexturePalette();

            IO::Md2Parser parser(name, file->begin(), file->end(), palette, m_fs);
            return parser.parseModel(logger);
        }

        Assets::EntityModel* GameImpl::loadMd3Model(const String& name, const IO::MappedFile::Ptr& file, Logger& logger) const {
            IO::Md3Parser parser(name, file->begin(), file->end(), m_fs);
            return parser.parseModel(logger);
        }

        Assets::EntityModel* GameImpl::loadDkmModel(const String& name, const IO::MappedFile::Ptr& file, Logger& logger) const {
            IO::DkmParser parser(name, file->begin(), file->end(), m_fs);
            return parser.parseModel(logger);
        }

        Assets::Palette GameImpl::loadTexturePalette() const {
            const auto& path = m_config.textureConfig().palette;
            return Assets::Palette::loadFile(m_fs, path);
        }

        StringList GameImpl::doAvailableMods() const {
            StringList result;
            if (m_gamePath.isEmpty() || !IO::Disk::directoryExists(m_gamePath)) {
                return result;
            }

            const auto& defaultMod = m_config.fileSystemConfig().searchPath.lastComponent().asString();
            const IO::DiskFileSystem fs(m_gamePath);
            const auto subDirs = fs.findItems(IO::Path(""), IO::FileTypeMatcher(false, true));
            for (size_t i = 0; i < subDirs.size(); ++i) {
                const String mod = subDirs[i].lastComponent().asString();
                if (!StringUtils::caseInsensitiveEqual(mod, defaultMod)) {
                    result.push_back(mod);
                }
            }
            return result;
        }

        StringList GameImpl::doExtractEnabledMods(const AttributableNode& node) const {
            StringList result;
            const auto& modStr = node.attribute(AttributeNames::Mods);
            if (modStr.empty()) {
                return result;
            }

            return StringUtils::splitAndTrim(modStr, ';');
        }

        String GameImpl::doDefaultMod() const {
            return m_config.fileSystemConfig().searchPath.asString();
        }

        const GameConfig::FlagsConfig& GameImpl::doSurfaceFlags() const {
            return m_config.faceAttribsConfig().surfaceFlags;
        }

        const GameConfig::FlagsConfig& GameImpl::doContentFlags() const {
            return m_config.faceAttribsConfig().contentFlags;
        }

        void GameImpl::writeLongAttribute(AttributableNode& node, const AttributeName& baseName, const AttributeValue& value, const size_t maxLength) const {
            node.removeNumberedAttribute(baseName);

            StringStream nameStr;
            for (size_t i = 0; i <= value.size() / maxLength; ++i) {
                nameStr.str("");
                nameStr << baseName << i+1;
                node.addOrUpdateAttribute(nameStr.str(), value.substr(i * maxLength, maxLength));
            }
        }

        String GameImpl::readLongAttribute(const AttributableNode& node, const AttributeName& baseName) const {
            size_t index = 1;
            StringStream nameStr;
            StringStream valueStr;
            nameStr << baseName << index;
            while (node.hasAttribute(nameStr.str())) {
                valueStr << node.attribute(nameStr.str());
                nameStr.str("");
                nameStr << baseName << ++index;
            }

            return valueStr.str();
        }
    }
}
