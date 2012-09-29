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

#ifndef __TrenchBroom__TextureBrowserCanvas__
#define __TrenchBroom__TextureBrowserCanvas__

#include "Model/TextureManager.h"
#include "Renderer/RenderTypes.h"
#include "Renderer/Text/StringManager.h"
#include "View/CellLayoutGLCanvas.h"

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(EVT_TEXTURE_SELECTED_EVENT, 7777)
END_DECLARE_EVENT_TYPES()

namespace TrenchBroom {
    namespace Model {
        class Texture;
        class TextureCollection;
        class TextureManager;
    }
    
    namespace Renderer {
        namespace Text {
            class FontDescriptor;
        }
        
        class Shader;
        class ShaderProgram;
        class TextureRenderer;
        class Vbo;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class DocumentViewHolder;
        
        class TextureSelectedCommand : public wxCommandEvent {
        protected:
            Model::Texture* m_texture;
        public:
            TextureSelectedCommand(Model::Texture* texture) :
            wxCommandEvent(EVT_TEXTURE_SELECTED_EVENT, wxID_ANY),
            m_texture(texture) {}

            inline Model::Texture* texture() const {
                return m_texture;
            }
            
            inline wxEvent* Clone() const {
                return new TextureSelectedCommand(*this);
            }
        };

        class TextureGroupData {
        public:
            Model::TextureCollection* textureCollection;
            Renderer::Text::StringRendererPtr stringRenderer;
            
            TextureGroupData(Model::TextureCollection* textureCollection, Renderer::Text::StringRendererPtr stringRenderer) :
            textureCollection(textureCollection),
            stringRenderer(stringRenderer) {}
            
            TextureGroupData() :
            textureCollection(NULL),
            stringRenderer(NULL) {}
        };
        
        class TextureCellData {
        public:
            Model::Texture* texture;
            Renderer::TextureRenderer* textureRenderer;
            Renderer::Text::StringRendererPtr stringRenderer;
            
            TextureCellData(Model::Texture* texture, Renderer::TextureRenderer* textureRenderer, Renderer::Text::StringRendererPtr stringRenderer) :
            texture(texture),
            textureRenderer(textureRenderer),
            stringRenderer(stringRenderer) {}
        };
        
        class TextureBrowserCanvas : public CellLayoutGLCanvas<TextureCellData, TextureGroupData> {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            Model::Texture* m_selectedTexture;
            Renderer::Text::StringManager m_stringManager;
            
            typedef std::map<Model::Texture*, Renderer::Text::StringRendererPtr> StringRendererCache;
            StringRendererCache m_stringRendererCache;
            
            Renderer::ShaderPtr m_textureBorderVertexShader;
            Renderer::ShaderPtr m_textureBorderFragmentShader;
            Renderer::ShaderPtr m_textureVertexShader;
            Renderer::ShaderPtr m_textureFragmentShader;
            Renderer::ShaderPtr m_textVertexShader;
            Renderer::ShaderPtr m_textFragmentShader;
            Renderer::ShaderProgramPtr m_textureBorderShaderProgram;
            Renderer::ShaderProgramPtr m_textureShaderProgram;
            Renderer::ShaderProgramPtr m_textShaderProgram;
            bool m_shadersCreated;
            
            bool m_group;
            bool m_hideUnused;
            Model::TextureSortOrder::Type m_sortOrder;
            String m_filterText;
            
            void createShaders();
            
            void addTextureToLayout(Layout& layout, Model::Texture* texture, const Renderer::Text::FontDescriptor& font);
            virtual void doInitLayout(Layout& layout);
            virtual void doReloadLayout(Layout& layout);
            virtual void doRender(Layout& layout, Renderer::Transformation& transformation, float y, float height);
            virtual void handleLeftClick(Layout& layout, float x, float y);
        public:
            TextureBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder);
            ~TextureBrowserCanvas();
            
            inline void setSortOrder(Model::TextureSortOrder::Type sortOrder) {
                if (sortOrder == m_sortOrder)
                    return;
                m_sortOrder = sortOrder;
                reload();
                Refresh();
            }
        
            inline void setGroup(bool group) {
                if (group == m_group)
                    return;
                m_group = group;
                reload();
                Refresh();
            }
        
            inline void setHideUnused(bool hideUnused) {
                if (hideUnused == m_hideUnused)
                    return;
                m_hideUnused = hideUnused;
                reload();
                Refresh();
            }
        
            inline void setFilterText(const String& filterText) {
                if (filterText == m_filterText)
                    return;
                m_filterText = filterText;
                reload();
                Refresh();
            }
            
            inline Model::Texture* selectedTexture() const {
                return m_selectedTexture;
            }
            
            inline void setSelectedTexture(Model::Texture* texture) {
                if (texture == m_selectedTexture)
                    return;
                m_selectedTexture = texture;
                Refresh();
            }
        };
    }
}

typedef void (wxEvtHandler::*textureSelectedEventFunction)(TrenchBroom::View::TextureSelectedCommand&);

#define EVT_TEXTURE_SELECTED_HANDLER(fn) \
(wxObjectEventFunction)(wxEventFunction) (wxCommandEventFunction) \
wxStaticCastEvent(textureSelectedEventFunction, &fn)

#define EVT_TEXTURE_SELECTED(id,fn) \
DECLARE_EVENT_TABLE_ENTRY(EVT_TEXTURE_SELECTED_EVENT, id, wxID_ANY, \
(wxObjectEventFunction)(wxEventFunction) (wxCommandEventFunction) \
wxStaticCastEvent(textureSelectedEventFunction, &fn), (wxObject*) NULL ),

#endif /* defined(__TrenchBroom__TextureBrowserCanvas__) */
