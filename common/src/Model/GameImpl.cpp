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
#include "IO/DkPakFileSystem.h"
#include "IO/DiskFileSystem.h"
#include "IO/FgdParser.h"
#include "IO/FileMatcher.h"
#include "IO/FileSystem.h"
#include "IO/IdPakFileSystem.h"
#include "IO/IOUtils.h"
#include "IO/MapParser.h"
#include "IO/MdlParser.h"
#include "IO/Md2Parser.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/ObjSerializer.h"
#include "IO/WorldReader.h"
#include "IO/SimpleParserStatus.h"
#include "IO/SystemPaths.h"
#include "IO/TextureLoader.h"
#include "Model/AttributableNodeVariableStore.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/EntityAttributes.h"
#include "Model/GameConfig.h"
#include "Model/Layer.h"
#include "Model/Tutorial.h"
#include "Model/World.h"

#include "Exceptions.h"

#include <cstdio>

namespace TrenchBroom {
    namespace Model {
        GameImpl::GameImpl(GameConfig& config, const IO::Path& gamePath, Logger* logger) :
        m_config(config),
        m_gamePath(gamePath) {
            initializeFileSystem(logger);
        }
        
        void GameImpl::initializeFileSystem(Logger* logger) {
            const GameConfig::FileSystemConfig& fileSystemConfig = m_config.fileSystemConfig();
            if (!m_gamePath.isEmpty() && IO::Disk::directoryExists(m_gamePath)) {
                addSearchPath(fileSystemConfig.searchPath, logger);
                addPackages(m_gamePath + fileSystemConfig.searchPath);

                for (const IO::Path& searchPath : m_additionalSearchPaths) {
                    addSearchPath(searchPath, logger);
                    addPackages(m_gamePath + searchPath);
            }
        }
        }

        void GameImpl::addSearchPath(const IO::Path& searchPath, Logger* logger) {
            try {
                m_gameFS.pushFileSystem(new IO::DiskFileSystem(m_gamePath + searchPath));
            } catch (const FileSystemException& e) {
                logger->error("Could not add file system search path '" + searchPath.asString() + "': " + String(e.what()));
            }
        }
        
        void GameImpl::addPackages(const IO::Path& searchPath) {
            const GameConfig::FileSystemConfig& fileSystemConfig = m_config.fileSystemConfig();
            const GameConfig::PackageFormatConfig& packageFormatConfig = fileSystemConfig.packageFormat;

            const StringList& packageExtensions = packageFormatConfig.extensions;
            const String& packageFormat = packageFormatConfig.format;

            if (IO::Disk::directoryExists(searchPath)) {
                const IO::DiskFileSystem diskFS(searchPath);
                const IO::Path::List packages = diskFS.findItems(IO::Path(""), IO::FileExtensionMatcher(packageExtensions));
                for (const IO::Path& packagePath : packages) {
                    IO::MappedFile::Ptr packageFile = diskFS.openFile(packagePath);
                    ensure(packageFile.get() != nullptr, "packageFile is null");

                    if (StringUtils::caseInsensitiveEqual(packageFormat, "idpak")) {
                        m_gameFS.pushFileSystem(new IO::IdPakFileSystem(packagePath, packageFile));
                    } else if (StringUtils::caseInsensitiveEqual(packageFormat, "dkpak")) {
                        m_gameFS.pushFileSystem(new IO::DkPakFileSystem(packagePath, packageFile));
                    }
                }
            }
        }

        const String& GameImpl::doGameName() const {
            return m_config.name();
        }

        IO::Path GameImpl::doGamePath() const {
            return m_gamePath;
        }

        void GameImpl::doSetGamePath(const IO::Path& gamePath, Logger* logger) {
            m_gamePath = gamePath;
            m_gameFS.clear();
            initializeFileSystem(logger);
        }

        void GameImpl::doSetAdditionalSearchPaths(const IO::Path::List& searchPaths, Logger* logger) {
            m_additionalSearchPaths = searchPaths;
            m_gameFS.clear();
            initializeFileSystem(logger);
        }

        Game::PathErrors GameImpl::doCheckAdditionalSearchPaths(const IO::Path::List& searchPaths) const {
            PathErrors result;
            for (const IO::Path& searchPath : searchPaths) {
                try {
                    IO::DiskFileSystem(m_gamePath + searchPath);
                } catch (const Exception& e) {
                    result.insert(std::make_pair(searchPath, e.what()));
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

        World* GameImpl::doNewMap(const MapFormat::Type format, const vm::bbox3& worldBounds, Logger* logger) const {
            const auto initialMapFilePath = m_config.findInitialMap(formatName(format));
            if (!initialMapFilePath.isEmpty() && IO::Disk::fileExists(initialMapFilePath)) {
                return doLoadMap(format, worldBounds, initialMapFilePath, logger);
            } else {
                auto* world = new World(format, brushContentTypeBuilder(), worldBounds);

                const Model::BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCuboid(vm::vec3(128.0, 128.0, 32.0), Model::BrushFace::NoTextureName);
                world->defaultLayer()->addChild(brush);

                if (format == MapFormat::Valve) {
                    world->addOrUpdateAttribute(AttributeNames::ValveVersion, "220");
                    world->addOrUpdateAttribute(AttributeNames::ValveMaxRange, StringUtils::toString(int(worldBounds.max[0])));
                }

                return world;
            }
        }

        World* GameImpl::doLoadMap(const MapFormat::Type format, const vm::bbox3& worldBounds, const IO::Path& path, Logger* logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::WorldReader reader(file->begin(), file->end(), brushContentTypeBuilder());
            return reader.read(format, worldBounds, parserStatus);
        }

        void GameImpl::doWriteMap(World* world, const IO::Path& path) const {
            const String mapFormatName = formatName(world->format());

            IO::OpenFile open(path, true);
            IO::writeGameComment(open.file, gameName(), mapFormatName);

            IO::NodeWriter writer(world, open.file);
            writer.writeMap();
        }

        void GameImpl::doExportMap(World* world, const Model::ExportFormat format, const IO::Path& path) const {
            IO::OpenFile open(path, true);

            switch (format) {
                case Model::WavefrontObj:
                    IO::NodeWriter(world, new IO::ObjFileSerializer(open.file)).writeMap();
                    break;
            }
        }

        NodeList GameImpl::doParseNodes(const String& str, World* world, const vm::bbox3& worldBounds, Logger* logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::NodeReader reader(str, world);
            return reader.read(worldBounds, parserStatus);
        }

        BrushFaceList GameImpl::doParseBrushFaces(const String& str, World* world, const vm::bbox3& worldBounds, Logger* logger) const {
            IO::SimpleParserStatus parserStatus(logger);
            IO::BrushFaceReader reader(str, world);
            return reader.read(worldBounds, parserStatus);
        }

        void GameImpl::doWriteNodesToStream(World* world, const Model::NodeList& nodes, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeNodes(nodes);
        }

        void GameImpl::doWriteBrushFacesToStream(World* world, const BrushFaceList& faces, std::ostream& stream) const {
            IO::NodeWriter writer(world, stream);
            writer.writeBrushFaces(faces);
        }

        Game::TexturePackageType GameImpl::doTexturePackageType() const {
            using Model::GameConfig;
            switch (m_config.textureConfig().package.type) {
                case GameConfig::TexturePackageConfig::PT_File:
                    return TP_File;
                case GameConfig::TexturePackageConfig::PT_Directory:
                    return TP_Directory;
                case GameConfig::TexturePackageConfig::PT_Unset:
                    throw GameException("Texture package type is not set in game configuration");
                switchDefault()
            }
        }

        void GameImpl::doLoadTextureCollections(AttributableNode* node, const IO::Path& documentPath, Assets::TextureManager& textureManager, Logger* logger) const {
            const AttributableNodeVariableStore variables(node);
            const IO::Path::List paths = extractTextureCollections(node);

            const IO::Path::List fileSearchPaths = textureCollectionSearchPaths(documentPath);
            IO::TextureLoader textureLoader(variables, m_gameFS, fileSearchPaths, m_config.textureConfig(), logger);
            textureLoader.loadTextures(paths, textureManager);
        }

        IO::Path::List GameImpl::textureCollectionSearchPaths(const IO::Path& documentPath) const {
            IO::Path::List result;
            result.push_back(documentPath);
            result.push_back(m_gamePath);
            result.push_back(IO::SystemPaths::appDirectory());
            return result;
        }

        bool GameImpl::doIsTextureCollection(const IO::Path& path) const {
            const GameConfig::TexturePackageConfig packageConfig = m_config.textureConfig().package;
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
                const IO::Path& searchPath = m_config.textureConfig().package.rootDirectory;
                if (!searchPath.isEmpty() && m_gameFS.directoryExists(searchPath))
                    return m_gameFS.findItems(searchPath, IO::FileTypeMatcher(false, true));
                return IO::Path::List();
            } catch (FileSystemException& e) {
                throw GameException("Could not find texture collections: " + String(e.what()));
            }
        }

        IO::Path::List GameImpl::doExtractTextureCollections(const AttributableNode* node) const {
            const String& property = m_config.textureConfig().attribute;
            if (property.empty())
                return IO::Path::List(0);

            const AttributeValue& pathsValue = node->attribute(property);
            if (pathsValue.empty())
                return IO::Path::List(0);

            return IO::Path::asPaths(StringUtils::splitAndTrim(pathsValue, ';'));
        }

        void GameImpl::doUpdateTextureCollections(AttributableNode* node, const IO::Path::List& paths) const {
            const String& attribute = m_config.textureConfig().attribute;
            if (attribute.empty())
                return;

            const String value = StringUtils::join(IO::Path::asStrings(paths, '/'), ';');
            node->addOrUpdateAttribute(attribute, value);
        }

        bool GameImpl::doIsEntityDefinitionFile(const IO::Path& path) const {
            const String extension = path.extension();
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                return true;
            } else if (StringUtils::caseInsensitiveEqual("def", extension)) {
                return true;
            } else {
                return false;
            }
        }

        Assets::EntityDefinitionList GameImpl::doLoadEntityDefinitions(IO::ParserStatus& status, const IO::Path& path) const {
            const String extension = path.extension();
            const Color& defaultColor = m_config.entityConfig().defaultColor;

            Assets::EntityDefinitionList definitions;
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::FgdParser parser(file->begin(), file->end(), defaultColor, file->path());
                definitions = parser.parseDefinitions(status);
            } else if (StringUtils::caseInsensitiveEqual("def", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::DefParser parser(file->begin(), file->end(), defaultColor);
                definitions = parser.parseDefinitions(status);
            } else {
                throw GameException("Unknown entity definition format: '" + path.asString() + "'");
            }

            definitions.push_back(Tutorial::createTutorialEntityDefinition());
            return definitions;
        }

        Assets::EntityDefinitionFileSpec::List GameImpl::doAllEntityDefinitionFiles() const {
            const IO::Path::List paths = m_config.entityConfig().defFilePaths;
            const size_t count = paths.size();

            Assets::EntityDefinitionFileSpec::List result(count);
            for (size_t i = 0; i < count; ++i)
                result[i] = Assets::EntityDefinitionFileSpec::builtin(paths[i]);

            return result;
        }

        Assets::EntityDefinitionFileSpec GameImpl::doExtractEntityDefinitionFile(const AttributableNode* node) const {
            const AttributeValue& defValue = node->attribute(AttributeNames::EntityDefinitions);
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
                const IO::MappedFile::Ptr file = m_gameFS.openFile(path);
                ensure(file.get() != nullptr, "file is null");

                const String modelName = path.lastComponent().asString();
                const String extension = StringUtils::toLower(path.extension());
                const StringSet supported = m_config.entityConfig().modelFormats;

                if (extension == "mdl" && supported.count("mdl") > 0) {
                    return loadMdlModel(modelName, file);
                } else if (extension == "md2" && supported.count("md2") > 0) {
                    return loadMd2Model(modelName, file);
                } else if (extension == "bsp" && supported.count("bsp") > 0) {
                    return loadBspModel(modelName, file);
                } else if (extension == "dkm" && supported.count("dkm") > 0) {
                    return loadDkmModel(modelName, file);
                } else {
                    throw GameException("Unsupported model format '" + path.asString() + "'");
                }
            } catch (FileSystemException& e) {
                throw GameException("Could not load model " + path.asString() + ": " + String(e.what()));
            }
        }

        Assets::EntityModel* GameImpl::loadBspModel(const String& name, const IO::MappedFile::Ptr& file) const {
            const Assets::Palette palette = loadTexturePalette();

            IO::Bsp29Parser parser(name, file->begin(), file->end(), palette);
            return parser.parseModel();
        }

        Assets::EntityModel* GameImpl::loadMdlModel(const String& name, const IO::MappedFile::Ptr& file) const {
            const Assets::Palette palette = loadTexturePalette();

            IO::MdlParser parser(name, file->begin(), file->end(), palette);
            return parser.parseModel();
        }

        Assets::EntityModel* GameImpl::loadMd2Model(const String& name, const IO::MappedFile::Ptr& file) const {
            const Assets::Palette palette = loadTexturePalette();

            IO::Md2Parser parser(name, file->begin(), file->end(), palette, m_gameFS);
            return parser.parseModel();
        }

        Assets::EntityModel* GameImpl::loadDkmModel(const String& name, const IO::MappedFile::Ptr& file) const {
            IO::DkmParser parser(name, file->begin(), file->end(), m_gameFS);
            return parser.parseModel();
        }

        Assets::Palette GameImpl::loadTexturePalette() const {
            // Be aware that this function does not work when the palette path contains variables.
            // However, since so far the only game that uses such variables is Daikatana, and the
            // Daikatana models do not refer to the global palette, we can ignore this here.
            const IO::Path& path = m_config.textureConfig().palette;
            return Assets::Palette::loadFile(m_gameFS, path);
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
            const IO::Path::List subDirs = fs.findItems(IO::Path(""), IO::FileTypeMatcher(false, true));
            for (size_t i = 0; i < subDirs.size(); ++i) {
                const String mod = subDirs[i].lastComponent().asString();
                if (!StringUtils::caseInsensitiveEqual(mod, defaultMod))
                    result.push_back(mod);
            }
            return result;
        }

        StringList GameImpl::doExtractEnabledMods(const AttributableNode* node) const {
            StringList result;
            const AttributeValue& modStr = node->attribute(AttributeNames::Mods);
            if (modStr.empty())
                return result;

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

        void GameImpl::writeLongAttribute(AttributableNode* node, const AttributeName& baseName, const AttributeValue& value, const size_t maxLength) const {

            node->removeNumberedAttribute(baseName);

            StringStream nameStr;
            for (size_t i = 0; i <= value.size() / maxLength; ++i) {
                nameStr.str("");
                nameStr << baseName << i+1;
                node->addOrUpdateAttribute(nameStr.str(), value.substr(i * maxLength, maxLength));
            }
        }

        String GameImpl::readLongAttribute(const AttributableNode* node, const AttributeName& baseName) const {
            ensure(node != nullptr, "node is null");

            size_t index = 1;
            StringStream nameStr;
            StringStream valueStr;
            nameStr << baseName << index;
            while (node->hasAttribute(nameStr.str())) {
                valueStr << node->attribute(nameStr.str());
                nameStr.str("");
                nameStr << baseName << ++index;
            }

            return valueStr.str();
        }
    }
}
