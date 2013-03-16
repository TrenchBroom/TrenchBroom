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

#include "MapDocument.h"

#include "Controller/Autosaver.h"
#include "Controller/Command.h"
#include "IO/FileManager.h"
#include "IO/IOException.h"
#include "IO/MapParser.h"
#include "IO/MapWriter.h"
#include "IO/Wad.h"
#include "IO/mmapped_fstream.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinitionManager.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "Model/Octree.h"
#include "Model/Picker.h"
#include "Model/PointFile.h"
#include "Model/TextureManager.h"
#include "Renderer/SharedResources.h"
#include "Renderer/TextureRendererManager.h"
#include "Utility/Console.h"
#include "Utility/Grid.h"
#include "Utility/List.h"
#include "Utility/Preferences.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"
#include "View/EditorView.h"
#include "View/FaceInspector.h"
#include "View/Inspector.h"
#include "View/ProgressIndicatorDialog.h"

#include <cassert>

#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/stopwatch.h>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        BEGIN_EVENT_TABLE(MapDocument, wxDocument)
        EVT_TIMER(wxID_ANY, MapDocument::OnAutosaveTimer)
        END_EVENT_TABLE()

        IMPLEMENT_DYNAMIC_CLASS(MapDocument, wxDocument)

        bool MapDocument::DoOpenDocument(const wxString& file) {
            const String path = file.ToStdString();
            IO::FileManager fileManager;
            IO::MappedFile::Ptr mappedFile = fileManager.mapFile(path);
            if (mappedFile.get() != NULL) {
                console().info("Unloading existing map file and textures...");
                clear();
                
                console().info("Loading file %s", file.mbc_str().data());
                
                View::ProgressIndicatorDialog progressIndicator;
                loadMap(mappedFile->begin(), mappedFile->end(), progressIndicator);
                loadTextures(progressIndicator);
                updateAfterTextureManagerChanged();
                
                String definitionFile = "";
                Entity* worldspawnEntity = worldspawn(false);
                if (worldspawnEntity != NULL) {
                    const PropertyValue* defValue = worldspawnEntity->propertyForKey(Entity::DefKey);
                    if (defValue != NULL)
                        definitionFile = *defValue;
                }
                
                setEntityDefinitionFile(definitionFile);

                String title = fileManager.pathComponents(path).back();
                SetTitle(title);
                return true;
            }

            console().info("Could not open file %s", path.c_str());

            return true;
        }

        bool MapDocument::DoSaveDocument(const wxString& file) {
            wxStopWatch watch;
            IO::MapWriter mapWriter;
            mapWriter.writeToFileAtPath(*m_map, file.ToStdString(), true);
            console().info("Saved map file to %s in %f seconds", file.ToStdString().c_str(), watch.Time() / 1000.0f);
            
            return true;
        }

        void MapDocument::clear() {
            m_sharedResources->textureRendererManager().invalidate();
            m_editStateManager->clear();
            m_map->clear();
            m_octree->clear();
            m_textureManager->clear();
            m_definitionManager->clear();
            unloadPointFile();
            invalidateSearchPaths();

            Controller::Command clearCommand(Controller::Command::ClearMap);
            UpdateAllViews(NULL, &clearCommand);
        }

        void MapDocument::loadPalette() {
            IO::FileManager fileManager;
            String resourcePath = fileManager.resourceDirectory();
            String palettePath = fileManager.appendPath(resourcePath, "QuakePalette.lmp");
            m_sharedResources->loadPalette(palettePath);
        }

        void MapDocument::loadMap(char* begin, char* end, Utility::ProgressIndicator& progressIndicator) {
            progressIndicator.setText("Loading map file...");
            
            wxStopWatch watch;
            IO::MapParser parser(begin, end, console());
            parser.parseMap(*m_map, &progressIndicator);
            
            console().info("Loaded map file in %f seconds", watch.Time() / 1000.0f);
        }

        void MapDocument::loadTextures(Utility::ProgressIndicator& progressIndicator) {
            progressIndicator.setText("Loading textures...");

            const String* wads = worldspawn(true)->propertyForKey(Entity::WadKey);
            if (wads != NULL) {
                StringList wadPaths = Utility::split(*wads, ';');
                for (unsigned int i = 0; i < wadPaths.size(); i++) {
                    String wadPath = Utility::trim(wadPaths[i]);
                    loadTextureWad(wadPath);
                }
            }
        }
        
        MapDocument::MapDocument() :
        m_autosaver(NULL),
        m_console(NULL),
        m_sharedResources(NULL),
        m_map(NULL),
        m_editStateManager(NULL),
        m_octree(NULL),
        m_picker(NULL),
        m_textureManager(NULL),
        m_definitionManager(NULL),
        m_grid(new Utility::Grid(4)),
        m_mruTexture(NULL),
        m_mruTextureName(""),
        m_textureLock(true),
        m_modificationCount(0),
        m_searchPathsValid(false),
        m_pointFile(NULL) {}

        MapDocument::~MapDocument() {
            delete m_autosaveTimer;
            m_autosaveTimer = NULL;
            delete m_autosaver;
            m_autosaver = NULL;
            delete m_picker;
            m_picker = NULL;
            delete m_octree;
            m_octree = NULL;
            delete m_editStateManager;
            m_editStateManager = NULL;
            delete m_map;
            m_map = NULL;
            delete m_definitionManager;
            m_definitionManager = NULL;
            delete m_textureManager;
            m_textureManager = NULL;
            delete m_grid;
            m_grid = NULL;
            m_sharedResources->Destroy(); // makes sure that the resources are deleted after the last frame
            m_sharedResources = NULL;
            delete m_console;
            m_console = NULL;
        }


        std::istream& MapDocument::LoadObject(std::istream& stream) {
            return stream;
        }

        std::ostream& MapDocument::SaveObject(std::ostream& stream) {
            /* //we're doing this directly in OnSaveDocument so that we can avoid using stream io
            IO::MapWriter mapWriter;
            mapWriter.writeToStream(*m_map, stream);
            m_modificationCount = 0;
             */
            return stream;
        }

        Entity* MapDocument::worldspawn(bool create) {
            Entity* worldspawn = m_map->worldspawn();
            if (worldspawn == NULL && create) {
                worldspawn = new Entity(m_map->worldBounds());
                worldspawn->setProperty(Entity::ClassnameKey, Entity::WorldspawnClassname);
                EntityDefinition* definition = m_definitionManager->definition(Entity::WorldspawnClassname);
                worldspawn->setDefinition(definition);
                m_map->addEntity(*worldspawn);
            }

            return worldspawn;
        }

        void MapDocument::Modify(bool modify) {
            wxDocument::Modify(modify);
            if (modify)
                m_autosaver->updateLastModificationTime();
            else
                m_autosaver->clearDirtyFlag();

            wxList& views = GetViews();
            wxList::iterator it, end;
            for (it = views.begin(), end = views.end(); it != end; ++it) {
                View::EditorView* view = wxDynamicCast(*it, View::EditorView);
                if (view != NULL)
                    view->setModified(IsModified());
            }
        }

        void MapDocument::addEntity(Entity& entity) {
            const String* classname = entity.classname();
            if (classname != NULL) {
                Model::EntityDefinition* definition = m_definitionManager->definition(*classname);
                if (definition != NULL)
                    entity.setDefinition(definition);
            }
            m_map->addEntity(entity);
            m_octree->addObject(entity);

            const Model::BrushList& brushes = entity.brushes();
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                m_octree->addObject(brush);

                const FaceList& faces = brush.faces();
                FaceList::const_iterator faceIt, faceEnd;
                for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                    Face& face = **faceIt;
                    face.setTexture(m_textureManager->texture(face.textureName()));
                }
            }
        }

        void MapDocument::entityWillChange(Entity& entity) {
            m_octree->removeObject(entity);
        }

        void MapDocument::entityDidChange(Entity& entity) {
            m_octree->addObject(entity);
        }

        void MapDocument::entitiesWillChange(const EntityList& entities) {
            MapObjectList objects;
            objects.insert(objects.begin(), entities.begin(), entities.end());
            m_octree->removeObjects(objects);
        }

        void MapDocument::entitiesDidChange(const EntityList& entities) {
            MapObjectList objects;
            objects.insert(objects.begin(), entities.begin(), entities.end());
            m_octree->addObjects(objects);
        }

        void MapDocument::removeEntity(Entity& entity) {
            const Model::BrushList& brushes = entity.brushes();
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                m_octree->removeObject(brush);
            }

            m_octree->removeObject(entity);
            m_map->removeEntity(entity);
            entity.setDefinition(NULL);
        }

        void MapDocument::addBrush(Entity& entity, Brush& brush) {
            if (!entity.worldspawn())
                m_octree->removeObject(entity);
            entity.addBrush(brush);
            m_octree->addObject(brush);
            if (!entity.worldspawn())
                m_octree->addObject(entity);

            const FaceList& faces = brush.faces();
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.setTexture(m_textureManager->texture(face.textureName()));
            }
        }

        void MapDocument::brushWillChange(Brush& brush) {
            Entity* entity = brush.entity();
            if (entity != NULL && !entity->worldspawn())
                m_octree->removeObject(*entity);
            m_octree->removeObject(brush);
        }

        void MapDocument::brushDidChange(Brush& brush) {
            Entity* entity = brush.entity();
            m_octree->addObject(brush);
            if (entity != NULL && !entity->worldspawn())
                m_octree->addObject(*entity);
        }

        void MapDocument::brushesWillChange(const BrushList& brushes) {
            MapObjectSet objects;
            objects.insert(brushes.begin(), brushes.end());
            
            BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Brush* brush = *it;
                Entity* entity = brush->entity();
                if (entity != NULL && !entity->worldspawn())
                    objects.insert(entity);
            }
            
            m_octree->removeObjects(makeList(objects));
        }

        void MapDocument::brushesDidChange(const BrushList& brushes) {
            MapObjectList objects;
            objects.insert(objects.begin(), brushes.begin(), brushes.end());

            BrushList::const_iterator it, end;
            for (it = brushes.begin(), end = brushes.end(); it != end; ++it) {
                Brush* brush = *it;
                Entity* entity = brush->entity();
                if (entity != NULL && !entity->worldspawn())
                    objects.push_back(entity);
            }

            m_octree->addObjects(objects);
        }

        void MapDocument::removeBrush(Brush& brush) {
            m_octree->removeObject(brush);
            Entity* entity = brush.entity();
            if (entity != NULL) {
                if (!entity->worldspawn())
                    m_octree->removeObject(*entity);
                entity->removeBrush(brush);
                if (!entity->worldspawn())
                    m_octree->addObject(*entity);
            }
            
            const FaceList& faces = brush.faces();
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                Face& face = **faceIt;
                face.setTexture(NULL);
            }
        }

        Utility::Console& MapDocument::console() const {
            return *m_console;
        }

        Renderer::SharedResources& MapDocument::sharedResources() const {
            return *m_sharedResources;
        }

        Map& MapDocument::map() const {
            return *m_map;
        }

        EntityDefinitionManager& MapDocument::definitionManager() const {
            return *m_definitionManager;
        }

        EditStateManager& MapDocument::editStateManager() const {
            return *m_editStateManager;
        }

        TextureManager& MapDocument::textureManager() const {
            return *m_textureManager;
        }

        Picker& MapDocument::picker() const {
            return *m_picker;
        }

        Utility::Grid& MapDocument::grid() const {
            return *m_grid;
        }

        const StringList& MapDocument::searchPaths() const {
            if (!m_searchPathsValid) {
                m_searchPaths.clear();
                m_searchPaths.push_back("id1");
                
                Entity* worldspawnEntity = m_map->worldspawn();
                if (worldspawnEntity != NULL) {
                    const PropertyValue* modValue = worldspawnEntity->propertyForKey(Entity::ModKey);
                    if (modValue != NULL && !Utility::equalsString(*modValue, "id1", false))
                        m_searchPaths.push_back(*modValue);
                }
                
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
                const String& quakePath = prefs.getString(Preferences::QuakePath);

                IO::FileManager fileManager;
                m_searchPaths = fileManager.resolveSearchpaths(quakePath, m_searchPaths);
                m_searchPathsValid = true;
            }

            return m_searchPaths;
        }
        
        void MapDocument::invalidateSearchPaths() {
            m_searchPathsValid = false;
        }

        bool MapDocument::pointFileExists() {
            return PointFile::exists(GetFilename().ToStdString());
        }
        
        void MapDocument::loadPointFile() {
            assert(pointFileExists());

            if (m_pointFile != NULL)
                unloadPointFile();
            m_pointFile = new PointFile(GetFilename().ToStdString());
        }
        
        void MapDocument::unloadPointFile() {
            assert(m_pointFile == NULL);
            delete m_pointFile;
            m_pointFile = NULL;
        }
        
        bool MapDocument::pointFileLoaded() {
            return m_pointFile != NULL;
        }
        
        PointFile& MapDocument::pointFile() {
            assert(pointFileLoaded());
            return *m_pointFile;
        }

        Model::Texture* MapDocument::mruTexture() const {
            return m_mruTexture;
        }

        void MapDocument::setMruTexture(Model::Texture* texture) {
            if (texture == NULL)
                m_mruTextureName = "";
            else
                m_mruTextureName = texture->name();
            m_mruTexture = texture;
        }

        bool MapDocument::textureLock() const {
            return m_textureLock;
        }

        void MapDocument::setTextureLock(bool textureLock) {
            m_textureLock = textureLock;
        }

        void MapDocument::updateAfterTextureManagerChanged() {
            Model::FaceList changedFaces;
            Model::TextureList newTextures;

            const Model::EntityList& entities = m_map->entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                const Model::BrushList& brushes = entities[i]->brushes();
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    const Model::FaceList& faces = brushes[j]->faces();
                    for (unsigned int k = 0; k < faces.size(); k++) {
                        const String& textureName = faces[k]->textureName();
                        Model::Texture* oldTexture = faces[k]->texture();
                        Model::Texture* newTexture = m_textureManager->texture(textureName);
                        if (oldTexture != newTexture) {
                            changedFaces.push_back(faces[k]);
                            newTextures.push_back(newTexture);
                        }
                    }
                }
            }

            if (!changedFaces.empty()) {
                for (unsigned int i = 0; i < changedFaces.size(); i++)
                    changedFaces[i]->setTexture(newTextures[i]);
            }

            if (m_mruTexture != NULL && m_mruTexture != m_textureManager->texture(m_mruTextureName))
                setMruTexture(NULL);
        }

        void MapDocument::loadTextureWad(const String& path) {
            loadTextureWad(path, m_textureManager->collections().size());
        }

        void MapDocument::loadTextureWad(const String& path, size_t index) {
            IO::FileManager fileManager;

            String wadPath = path;
            if (!fileManager.isAbsolutePath(wadPath)) {
                Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();

                StringList rootPaths;
                rootPaths.push_back(GetFilename().ToStdString());
                rootPaths.push_back(wxStandardPaths::Get().GetExecutablePath().ToStdString());
                rootPaths.push_back(prefs.getString(Preferences::QuakePath));
                
                if (!fileManager.resolveRelativePath(path, rootPaths, wadPath)) {
                    console().error("Could not open texture wad %s (tried relative to current map file, TrenchBroom executable, and Quake path)", path.c_str());
                    return;
                }
            }

            if (fileManager.exists(wadPath)) {
                Model::TextureCollection* collection = NULL;
                wxStopWatch watch;
                try {
                    collection = new Model::TextureCollection(path, wadPath);
                    m_textureManager->addCollection(collection, index);
                    console().info("Loaded %s in %f seconds", wadPath.c_str(), watch.Time() / 1000.0f);
                } catch (IO::IOException e) {
                    delete collection;
                    collection = NULL;
                    console().error("Could not open texture wad %s: %s", wadPath.c_str(), e.what());
                }
            } else {
                console().error("Could not open texture wad %s", wadPath.c_str());
            }
        }

        void MapDocument::setEntityDefinitionFile(const String& definitionFile) {
            IO::FileManager fileManager;
            const String resourcePath = fileManager.resourceDirectory();
            const String defsPath = fileManager.appendPathComponent(resourcePath, "Defs");
            String definitionPath;
            
            if (Utility::startsWith(definitionFile, "external:")) {
                definitionPath = definitionFile.substr(9);
            } else if (Utility::startsWith(definitionFile, "builtin:")) {
                definitionPath = fileManager.appendPath(defsPath, definitionFile.substr(8));
            } else if (definitionFile == "") {
                definitionPath = fileManager.appendPath(defsPath, Entity::DefaultDefinition);
            } else {
                console().error("Unable to load entity definition file %s", definitionFile.c_str());
                return;
            }

            const EntityList& entities = m_map->entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity& entity = *entities[i];
                entity.setDefinition(NULL);
            }

            m_octree->clear();
            
            Entity* worldspawnEntity = worldspawn(true);
            worldspawnEntity->setProperty(Entity::DefKey, definitionFile);

            console().info("Loading entity definition file %s", definitionPath.c_str());
            m_definitionManager->clear();
            m_definitionManager->load(definitionPath);

            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity& entity = *entities[i];
                const PropertyValue* classname = entity.classname();
                if (classname != NULL) {
                    EntityDefinition* definition = m_definitionManager->definition(*classname);
                    entity.setDefinition(definition);
                }
            }
            
            m_octree->loadMap();
        }

        void MapDocument::incModificationCount() {
            m_modificationCount++;
            Modify(m_modificationCount != 0);
        }

        void MapDocument::decModificationCount() {
            m_modificationCount--;
            Modify(m_modificationCount != 0);
        }

        bool MapDocument::OnCreate(const wxString& path, long flags) {
            BBox worldBounds(Vec3f(-16384, -16384, -16384), Vec3f(16384, 16384, 16384));

            m_console = new Utility::Console();
            m_textureManager = new TextureManager();
            m_sharedResources = new Renderer::SharedResources(*m_textureManager, *m_console);
            m_map = new Model::Map(worldBounds);
            m_editStateManager = new Model::EditStateManager();
            m_octree = new Octree(*m_map);
            m_picker = new Model::Picker(*m_octree);
            m_definitionManager = new EntityDefinitionManager(*m_console);
            m_modificationCount = 0;
            m_autosaver = new Controller::Autosaver(*this);
            m_autosaveTimer = new wxTimer(this);
            m_autosaveTimer->Start(1000);

            loadPalette();

            return wxDocument::OnCreate(path, flags);
        }

		bool MapDocument::OnNewDocument() {
			if (wxDocument::OnNewDocument()) {
				// prompt for initial stuff like world bounds, mods, palette, def here
                clear();
                setEntityDefinitionFile("");

                // place 1 new brush at origin
                BBox brushBounds(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(64.0f, 64.0f, 16.0f));
                Model::Brush* brush = new Model::Brush(m_map->worldBounds(), brushBounds, NULL);
                addBrush(*worldspawn(true), *brush);
                
                Controller::Command loadCommand(Controller::Command::LoadMap);
                UpdateAllViews(NULL, &loadCommand);
                
                m_modificationCount = 0;
                m_autosaver->clearDirtyFlag();
				return true;
			}

			return false;
		}

        bool MapDocument::OnOpenDocument(const wxString& path) {
            if (wxDocument::OnOpenDocument(path)) {
                Controller::Command loadCommand(Controller::Command::LoadMap);
                UpdateAllViews(NULL, &loadCommand);
                m_modificationCount = 0;
                m_autosaver->clearDirtyFlag();
				return true;
            }

            return false;
        }

        void MapDocument::OnAutosaveTimer(wxTimerEvent& event) {
            m_autosaver->triggerAutosave();
        }
	}
}
