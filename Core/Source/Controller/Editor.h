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

#ifndef TrenchBroom_Editor_h
#define TrenchBroom_Editor_h

#include "Utilities/VecMath.h"

#include <string>

namespace TrenchBroom {
    class Filter;
    
    namespace Model {
        namespace Assets {
            class Palette;
            class TextureManager;
        }
        class Map;
        class UndoGroup;
    }
    
    namespace Renderer {
        class MapRenderer;
    }
    
    namespace Controller {
        class Autosaver;
        class Camera;
        class Grid;
        class InputController;
        class TransientOptions;
        class ProgressIndicator;

        class Editor {
        public:
            typedef enum {
                LEFT,
                UP,
                RIGHT,
                DOWN,
                FORWARD,
                BACKWARD
            } EMoveDirection;
            
            typedef enum {
                ROLL,
                PITCH,
                YAW
            } ERotationAxis;
        private:
            std::string m_mapPath;
            Model::Map* m_map;
            Camera* m_camera;
            Grid* m_grid;
            InputController* m_inputController;
            TransientOptions* m_options;
            Filter* m_filter;
            Autosaver* m_autosaver;
            Model::Assets::TextureManager* m_textureManager;
            Model::Assets::Palette* m_palette;
            Renderer::MapRenderer* m_renderer;
            std::string m_entityDefinitionFilePath;

            void updateFaceTextures();
            void textureManagerDidChange(Model::Assets::TextureManager& textureManager);
            void preferencesDidChange(const std::string& key);
            void undoGroupCreated(const Model::UndoGroup& group);
        public:
            Editor(const std::string& entityDefinitionFilePath, const std::string& palettePath);
            ~Editor();
            
            void loadMap(const std::string& path, ProgressIndicator* indicator);
            void saveMap(const std::string& path);
            
            void loadTextureWad(const std::string& path);
            
            const std::string& mapPath() const;
            Model::Map& map() const;
            Camera& camera() const;
            Grid& grid() const;
            InputController& inputController() const;
            TransientOptions& options() const;
            Filter& filter() const;
            Autosaver& autosaver() const;
            Model::Assets::Palette& palette() const;
            Model::Assets::TextureManager& textureManager() const;
            
            void setRenderer(Renderer::MapRenderer* renderer);
            Renderer::MapRenderer* renderer() const;
            
            void undo();
            void redo();
            
            void selectAll();
            void selectEntities();
            void selectTouching(bool deleteBrush = true);
            void selectNone();
            
            void moveTextures(EMoveDirection direction, bool disableSnapToGrid);
            void rotateTextures(bool clockwise, bool disableSnapToGrid);
            
            void moveObjects(EMoveDirection direction, bool disableSnapToGrid);
            void rotateObjects(ERotationAxis axis, bool clockwise);
            void flipObjects(bool horizontally);
            void duplicateObjects();
            void enlargeBrushes();
            
            void toggleGrid();
            void toggleSnapToGrid();
            void setGridSize(int size);
            
            void moveCamera(EMoveDirection direction, bool disableSnapToGrid);
        };
    }
}

#endif
