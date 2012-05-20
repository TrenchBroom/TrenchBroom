/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#ifndef GWEN_RENDERERS_OPENGL_FTGL_H
#define GWEN_RENDERERS_OPENGL_FTGL_H

#include "Gwen/Gwen.h"
#include "Gwen/Renderers/OpenGL.h"
#include "Utilities/SharedPointer.h"
#include "FTGL/ftgl.h"
#include <map>

namespace Gwen 
{
	namespace Renderer 
	{
        class FontDescriptor {
        public:
            UnicodeString name;
            float size;
            FontDescriptor() {}
            FontDescriptor(const UnicodeString& name, float size) : name(name), size(size) {}
        };
        
        bool operator<(FontDescriptor const& left, FontDescriptor const& right);

        typedef std::tr1::shared_ptr<FTFont> FontPtr;

		class OpenGL_FTGL : public Gwen::Renderer::OpenGL
		{
        protected:
            typedef std::map<FontDescriptor, FontPtr> FontCache;
            FontCache m_fontCache;
            FontPtr loadFont(Gwen::Font* pFont);
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
