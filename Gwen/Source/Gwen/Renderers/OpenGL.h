/*
 GWEN
 Copyright (c) 2011 Facepunch Studios
 See license in Gwen.h
 */

#ifndef GWEN_RENDERERS_OPENGL_H
#define GWEN_RENDERERS_OPENGL_H

#include "Gwen/Gwen.h"
#include "Gwen/Texture.h"
#include "Gwen/BaseRender.h"
#include "GL/GLee.h"
#include <map>

namespace Gwen 
{
	namespace Renderer 
	{
        
        class OpenGLCacheToTexture : public ICacheToTexture {
        protected:
            typedef std::map<Gwen::Controls::Base*, Gwen::Texture*> RenderTextureCache;
            typedef std::vector<Gwen::Rect> ViewportStack;
            
            Gwen::Renderer::Base* m_renderer;
            GLuint m_frameBufferId;
            GLuint m_renderBufferId;
            RenderTextureCache m_textures;
            ViewportStack m_viewportStack;
        public:
            OpenGLCacheToTexture();
            virtual ~OpenGLCacheToTexture();
			virtual void Initialize();
			virtual void ShutDown();
			virtual void SetupCacheTexture( Gwen::Controls::Base* control, const Gwen::Point& offset );
			virtual void FinishCacheTexture( Gwen::Controls::Base* control );
			virtual void DrawCachedControlTexture( Gwen::Controls::Base* control );
			virtual void CreateControlCacheTexture( Gwen::Controls::Base* control );
			virtual void UpdateControlCacheTexture( Gwen::Controls::Base* control );
			virtual void SetRenderer( Gwen::Renderer::Base* renderer );
        };
        
		class OpenGL : public Gwen::Renderer::Base
		{
        public:
            
            struct Vertex
            {
                float x, y, z;
                float u, v;
                unsigned char r, g, b, a;
            };
            
            OpenGL();
            ~OpenGL();
            
            virtual void Begin();
            virtual void End();
            
            virtual void SetDrawColor( Gwen::Color color );
            virtual void DrawFilledRect( Gwen::Rect rect );
            
            void StartClip();
            void EndClip();
            
            void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f );
            void LoadTexture( Gwen::Texture* pTexture );
            void FreeTexture( Gwen::Texture* pTexture );
            Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default );
            virtual void Flush();
            
            virtual ICacheToTexture* GetCTT();
        protected:
            
            static const int	MaxVerts = 1024;
            
            
            void AddVert( int x, int y, float u = 0.0f , float v = 0.0f );
            
            Gwen::Color			m_Color;
            int					m_iVertNum;
            Vertex				m_Vertices[ MaxVerts ];
            
            ICacheToTexture*    m_cacheToTexture;
		};
        
	}
}
#endif
