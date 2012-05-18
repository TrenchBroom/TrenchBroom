/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#ifndef GWEN_RENDERERS_OPENGL_FTGL_H
#define GWEN_RENDERERS_OPENGL_FTGL_H

#include "Gwen/Gwen.h"
#include "Gwen/Renderers/OpenGL.h"
#include "FTGL/ftgl.h"
#include <map>

namespace Gwen 
{
	namespace Renderer 
	{
        
		class OpenGL_FTGL : public Gwen::Renderer::OpenGL
		{
        protected:
            String resolveFontPath(Gwen::Font* pFont);
            FTGL::FTGLfont* loadFont(Gwen::Font* pFont);
        public:
            
            OpenGL_FTGL();
            ~OpenGL_FTGL();
            
            void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
            Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );
            
        protected:
            
		};
        
	}
}
#endif
