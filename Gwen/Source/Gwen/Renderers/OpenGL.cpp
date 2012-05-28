
#include "Gwen/Renderers/OpenGL.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"

#include <cmath>
#include <cassert>

#include "FreeImage/FreeImage.h"


namespace Gwen
{
	namespace Renderer
	{
        OpenGLCacheToTexture::OpenGLCacheToTexture() {
        }

        OpenGLCacheToTexture::~OpenGLCacheToTexture() {
        }
        
        void OpenGLCacheToTexture::Initialize() {
            glGenFramebuffers(1, &m_frameBufferId);
        }
        
        void OpenGLCacheToTexture::ShutDown() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &m_frameBufferId);
            m_frameBufferId = 0;

            for (RenderTextureCache::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
                it->second->Release(m_renderer);
            m_textures.clear();
        }
        
        void OpenGLCacheToTexture::SetupCacheTexture( Gwen::Controls::Base* control ) {
            m_renderer->Flush();
            
            const Gwen::Rect& bounds = control->GetBounds();
            
            Gwen::Texture* texture = NULL;
            if (m_textures.count(control) > 0) {
                texture = m_textures[control];
                if (texture->width != bounds.w || texture->height != bounds.h) {
                    texture->Release(m_renderer);
                    texture = NULL;
                }
            }
            
            if (texture == NULL) {
                texture = new Gwen::Texture();
                GLuint* textureId = new GLuint();
                *textureId = 0;
                texture->data = textureId;
                texture->width = bounds.w;
                texture->height = bounds.h;
            }

            glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
            GLuint* textureId = static_cast<GLuint*>(texture->data);
            if (*textureId == 0) {
                glGenTextures(1, textureId);
                glBindTexture(GL_TEXTURE_2D, *textureId);

                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bounds.w, bounds.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            }
            glBindTexture(GL_TEXTURE_2D, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *textureId, 0);

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, bounds.w, bounds.h, 0, -1, 1);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            
            glViewport(0, 0, bounds.w, bounds.h);
            glDisable(GL_SCISSOR_TEST);

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            
            m_viewportStack.push_back(m_renderer->GetViewport());
            m_renderer->SetViewport(bounds);
            
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            assert(status == GL_FRAMEBUFFER_COMPLETE);
            
            GLenum error = glGetError();
            assert(error == GL_NO_ERROR);
             
            m_textures[control] = texture;
        }
        
        void OpenGLCacheToTexture::FinishCacheTexture( Gwen::Controls::Base* control ) {
            m_renderer->Flush();
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            glPopAttrib();

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            
            m_renderer->SetViewport(m_viewportStack.back());
            m_viewportStack.pop_back();
        }
        
        void OpenGLCacheToTexture::DrawCachedControlTexture( Gwen::Controls::Base* control ) {
            assert(m_textures.count(control) > 0);
            Gwen::Texture* texture = m_textures[control];
            m_renderer->SetDrawColor(Gwen::Color(255, 255, 255, 255));
            
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
			glDisableClientState( GL_VERTEX_ARRAY );
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            
            m_renderer->DrawTexturedRect(texture, control->GetBounds(), 0, 1, 1, 0);

            double plane[4];
            glGetClipPlane(GL_CLIP_PLANE0, plane);
            
            glPopClientAttrib();
        }
        
        void OpenGLCacheToTexture::CreateControlCacheTexture( Gwen::Controls::Base* control ) {
        }
        
        void OpenGLCacheToTexture::UpdateControlCacheTexture( Gwen::Controls::Base* control ) {
        }
        
        void OpenGLCacheToTexture::SetRenderer( Gwen::Renderer::Base* renderer ) {
            m_renderer = renderer;
        }

		
        OpenGL::OpenGL()
		{
            m_cacheToTexture = NULL;
			m_iVertNum = 0;

			::FreeImage_Initialise();

			for ( int i=0; i<MaxVerts; i++ )
			{
				m_Vertices[ i ].z = 0.5f;
			}
		}

		OpenGL::~OpenGL()
		{
            if (m_cacheToTexture != NULL) {
                m_cacheToTexture->ShutDown();
                delete m_cacheToTexture;
            }
			::FreeImage_DeInitialise();
		}

		void OpenGL::Begin()
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glAlphaFunc( GL_GREATER, 1.0f );	
			glEnable ( GL_BLEND );
		}

		void OpenGL::End()
		{
			Flush();
		}

		void OpenGL::Flush()
		{
			if ( m_iVertNum == 0 ) return;

			glVertexPointer( 3, GL_FLOAT,  sizeof(Vertex), (void*) &m_Vertices[0].x );
			glEnableClientState( GL_VERTEX_ARRAY );

			glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(Vertex), (void*)&m_Vertices[0].r );
			glEnableClientState( GL_COLOR_ARRAY );

			glTexCoordPointer( 2, GL_FLOAT, sizeof(Vertex), (void*) &m_Vertices[0].u );
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );

			glDrawArrays( GL_TRIANGLES, 0, (GLsizei) m_iVertNum );

			m_iVertNum = 0;
			glFlush();
		}

        ICacheToTexture* OpenGL::GetCTT() {
            if (m_cacheToTexture == NULL) {
                m_cacheToTexture = new OpenGLCacheToTexture();
                m_cacheToTexture->SetRenderer(this);
                m_cacheToTexture->Initialize();
            }
            
            return m_cacheToTexture;
        }
        
		void OpenGL::AddVert( int x, int y, float u, float v )
		{
			if ( m_iVertNum >= MaxVerts-1 )
			{
				Flush();
			}

			m_Vertices[ m_iVertNum ].x = (float)x;
			m_Vertices[ m_iVertNum ].y = (float)y;
			m_Vertices[ m_iVertNum ].u = u;
			m_Vertices[ m_iVertNum ].v = v;

			m_Vertices[ m_iVertNum ].r = m_Color.r;
			m_Vertices[ m_iVertNum ].g = m_Color.g;
			m_Vertices[ m_iVertNum ].b = m_Color.b;
			m_Vertices[ m_iVertNum ].a = m_Color.a;

			m_iVertNum++;
		}

		void OpenGL::DrawFilledRect( Gwen::Rect rect )
		{
			GLboolean texturesOn;

			glGetBooleanv(GL_TEXTURE_2D, &texturesOn);
			if ( texturesOn )
			{
				Flush();
				glDisable(GL_TEXTURE_2D);
			}	

			Translate( rect );

			AddVert( rect.x, rect.y );
			AddVert( rect.x+rect.w, rect.y );
			AddVert( rect.x, rect.y + rect.h );

			AddVert( rect.x+rect.w, rect.y );
			AddVert( rect.x+rect.w, rect.y+rect.h );
			AddVert( rect.x, rect.y + rect.h );
		}

		void OpenGL::SetDrawColor(Gwen::Color color)
		{
			glColor4ubv( (GLubyte*)&color );
			m_Color = color;
		}

		void OpenGL::StartClip()
		{
			Flush();
			Gwen::Rect rect = ClipRegion();

			// OpenGL's coords are from the bottom left
			// so we need to translate them here.
			{
                rect.y = m_Viewport.h - (rect.y + rect.h);
                /*
				GLint view[4];
				glGetIntegerv( GL_VIEWPORT, &view[0] );
				rect.y = view[3] - (rect.y + rect.h);
                 */
			}

			glScissor( rect.x * Scale(), rect.y * Scale(), rect.w * Scale(), rect.h * Scale() );
			glEnable( GL_SCISSOR_TEST );
		};

		void OpenGL::EndClip()
		{
			Flush();
			glDisable( GL_SCISSOR_TEST );
		};

		void OpenGL::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
		{
			GLuint* tex = (GLuint*)pTexture->data;

			// Missing image, not loaded properly?
			if ( !tex )
			{
				return DrawMissingImage( rect );
			}

			Translate( rect );
			GLuint boundtex;

			GLboolean texturesOn;
			glGetBooleanv(GL_TEXTURE_2D, &texturesOn);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *)&boundtex);
			if ( !texturesOn || *tex != boundtex )
			{
				Flush();
				glBindTexture( GL_TEXTURE_2D, *tex );
				glEnable(GL_TEXTURE_2D);
			}		

			AddVert( rect.x, rect.y,			u1, v1 );
			AddVert( rect.x+rect.w, rect.y,		u2, v1 );
			AddVert( rect.x, rect.y + rect.h,	u1, v2 );

			AddVert( rect.x+rect.w, rect.y,		u2, v1 );
			AddVert( rect.x+rect.w, rect.y+rect.h, u2, v2 );
			AddVert( rect.x, rect.y + rect.h, u1, v2 );			
		}

		void OpenGL::LoadTexture( Gwen::Texture* pTexture )
		{
			Gwen::String textureName = pTexture->name.Get();
			const char *wFileName = textureName.c_str();

			FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType( wFileName );

			if ( imageFormat == FIF_UNKNOWN )
				imageFormat = FreeImage_GetFIFFromFilename( wFileName );

			// Image failed to load..
			if ( imageFormat == FIF_UNKNOWN )
			{
				pTexture->failed = true;
				return;
			}

			// Try to load the image..
			FIBITMAP* bits = FreeImage_Load( imageFormat, wFileName );
			if ( !bits )
			{
				pTexture->failed = true;
				return;
			}

			// Convert to 32bit
			FIBITMAP * bits32 = FreeImage_ConvertTo32Bits( bits );
			FreeImage_Unload( bits );
			if ( !bits32 )
			{
				pTexture->failed = true;
				return;
			}

			// Flip
			::FreeImage_FlipVertical( bits32 );


			// Create a little texture pointer..
			GLuint* pglTexture = new GLuint;

			// Sort out our GWEN texture
			pTexture->data = pglTexture;
			pTexture->width = FreeImage_GetWidth( bits32 );
			pTexture->height = FreeImage_GetHeight( bits32 );

			// Create the opengl texture
			glGenTextures( 1, pglTexture );
			glBindTexture( GL_TEXTURE_2D, *pglTexture );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			#ifdef FREEIMAGE_BIGENDIAN
			GLenum format = GL_RGBA;
			#else
			GLenum format = GL_BGRA;
			#endif

			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, pTexture->width, pTexture->height, 0, format, GL_UNSIGNED_BYTE, (const GLvoid*)FreeImage_GetBits( bits32 ) );

			FreeImage_Unload( bits32 );

		}

		void OpenGL::FreeTexture( Gwen::Texture* pTexture )
		{
			GLuint* tex = (GLuint*)pTexture->data;
			if ( !tex ) return;

			glDeleteTextures( 1, tex );
			delete tex;
			pTexture->data = NULL;
		}

		Gwen::Color OpenGL::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default )
		{
			GLuint* tex = (GLuint*)pTexture->data;
			if ( !tex ) return col_default;

			unsigned int iPixelSize = sizeof(unsigned char) * 4;

			glBindTexture( GL_TEXTURE_2D, *tex );

			unsigned char* data = (unsigned char*) malloc( iPixelSize * pTexture->width * pTexture->height );

				glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				unsigned int iOffset = (y * pTexture->width + x) * 4;

				Gwen::Color c;
				c.r = data[0 + iOffset];
				c.g = data[1 + iOffset];
				c.b = data[2 + iOffset];
				c.a = data[3 + iOffset];

			//
			// Retrieving the entire texture for a single pixel read
			// is kind of a waste - maybe cache this pointer in the texture
			// data and then release later on? It's never called during runtime
			// - only during initialization.
			//
			free( data );

			return c;
		}

	}
}