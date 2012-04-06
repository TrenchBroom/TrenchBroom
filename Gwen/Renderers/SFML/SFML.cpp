#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/Renderers/SFML.h"
#include <GL/gl.h>

namespace Gwen 
{
	namespace Renderer 
	{

		SFML::SFML( sf::RenderTarget& target ) : m_Target(target)
		{

		}

		SFML::~SFML()
		{

		}

		void SFML::SetDrawColor( Gwen::Color color )
		{
			m_Color.r = color.r;
			m_Color.g = color.g;
			m_Color.b = color.b;
			m_Color.a = color.a;
		}

		void SFML::DrawFilledRect( Gwen::Rect rect )
		{
			Translate( rect );

			#if SFML_VERSION_MAJOR == 2
			m_Target.Draw( sf::Shape::Rectangle( rect.x, rect.y, rect.w, rect.h, m_Color ) );
			#else
			m_Target.Draw( sf::Shape::Rectangle( rect.x, rect.y, rect.x + rect.w, rect.y + rect.h, m_Color ) );
			#endif
		}

		void SFML::LoadFont( Gwen::Font* font )
		{
			font->realsize = font->size * Scale();

			sf::Font* pFont = new sf::Font();

#if SFML_VERSION_MAJOR == 2
			if ( !pFont->LoadFromFile( Utility::UnicodeToString( font->facename ) ) )
#else
			if ( !pFont->LoadFromFile( Utility::UnicodeToString( font->facename ), font->realsize  ) )
#endif
			{
				// Ideally here we should be setting the font to a system default font here.
				delete pFont;

				pFont = const_cast<sf::Font*>( &sf::Font::GetDefaultFont() );
			}
			
			font->data = pFont;
		}

		void SFML::FreeFont( Gwen::Font* pFont )
		{
			if ( !pFont->data ) return;

			sf::Font* font = ((sf::Font*)pFont->data);

			// If this is the default font then don't delete it!
			if ( font != &sf::Font::GetDefaultFont() )
			{
				delete font;
			}

			pFont->data = NULL;
		}

		void SFML::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
		{
			Translate( pos.x, pos.y );

			const sf::Font* pSFFont = (sf::Font*)(pFont->data);

			// If the font doesn't exist, or the font size should be changed
			if ( !pSFFont || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			if  ( !pSFFont )
			{
				pSFFont = &(sf::Font::GetDefaultFont());
			}

			#if SFML_VERSION_MAJOR == 2
				m_Target.SaveGLStates();
					sf::Text sfStr( text );
					sfStr.SetFont( *pSFFont );
					sfStr.Move( pos.x, pos.y );
					sfStr.SetCharacterSize( pFont->realsize );
					sfStr.SetColor( m_Color );
					m_Target.Draw( sfStr );
				m_Target.RestoreGLStates();
			#else
				sf::String sfStr( text );
				sfStr.SetFont( *pSFFont );
				sfStr.Move( pos.x, pos.y );
				sfStr.SetSize( pFont->realsize );
				sfStr.SetColor( m_Color );
				m_Target.Draw( sfStr );
			#endif

			
		}

		Gwen::Point SFML::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
		{
			const sf::Font* pSFFont = (sf::Font*)(pFont->data);

			// If the font doesn't exist, or the font size should be changed
			if ( !pSFFont || fabs( pFont->realsize - pFont->size * Scale() ) > 2 )
			{
				FreeFont( pFont );
				LoadFont( pFont );
			}

			if  ( !pSFFont )
			{
				pSFFont = &(sf::Font::GetDefaultFont());
			}

			#if SFML_VERSION_MAJOR == 2
				sf::Text sfStr( text );
				sfStr.SetFont( *pSFFont );
				sfStr.SetCharacterSize( pFont->realsize );
				sf::FloatRect sz = sfStr.GetRect();
				return Gwen::Point( sz.Width, sz.Height );

			#else
				sf::String sfStr( text );
				sfStr.SetFont( *pSFFont );
				sfStr.SetSize( pFont->realsize );
				sf::FloatRect sz = sfStr.GetRect();
				return Gwen::Point( sz.GetWidth(), sz.GetHeight() );
			#endif
			
		}

		void SFML::StartClip()
		{
			Gwen::Rect rect = ClipRegion();

			// OpenGL's coords are from the bottom left
			// so we need to translate them here.
			{
				GLint view[4];
				glGetIntegerv( GL_VIEWPORT, &view[0] );
				rect.y = view[3] - (rect.y + rect.h);
			}

			glScissor( rect.x * Scale(), rect.y * Scale(), rect.w * Scale(), rect.h * Scale() );
			glEnable( GL_SCISSOR_TEST );
		};


		void SFML::EndClip()
		{
			glDisable( GL_SCISSOR_TEST );
		};

		void SFML::LoadTexture( Gwen::Texture* pTexture )
		{
			if ( !pTexture ) return;
			if ( pTexture->data ) FreeTexture( pTexture );

#if SFML_VERSION_MAJOR == 2
			sf::Texture* tex = new sf::Texture();
#else
			sf::Image* tex = new sf::Image();
#endif
			tex->SetSmooth( true );

			if ( !tex->LoadFromFile( pTexture->name.Get() ) )
			{
				delete( tex );
				pTexture->failed = true;
				return;
			}

			pTexture->height = tex->GetHeight();
			pTexture->width = tex->GetWidth();
			pTexture->data = tex;

		};

		void SFML::FreeTexture( Gwen::Texture* pTexture )
		{
#if SFML_VERSION_MAJOR == 2
			sf::Texture* tex = static_cast<sf::Texture*>( pTexture->data );
#else 
			sf::Image* tex = static_cast<sf::Image*>( pTexture->data );
#endif 

			if ( tex )
			{
				delete tex;
			}

			pTexture->data = NULL;
		}

		void SFML::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
		{
#if SFML_VERSION_MAJOR == 2
			const sf::Texture* tex = static_cast<sf::Texture*>( pTexture->data );
#else 
			const sf::Image* tex = static_cast<sf::Image*>( pTexture->data );
#endif 

			if ( !tex ) 
				return DrawMissingImage( rect );

			Translate( rect );
		
			tex->Bind();

			glColor4f(1, 1, 1, 1 );

			glBegin( GL_QUADS );
				glTexCoord2f( u1, v1 );		glVertex2f(rect.x,     rect.y);
				glTexCoord2f( u1, v2 );		glVertex2f(rect.x,     rect.y + rect.h);
				glTexCoord2f( u2, v2 );		glVertex2f(rect.x + rect.w, rect.y + rect.h);
				glTexCoord2f( u2, v1 );		glVertex2f(rect.x + rect.w, rect.y) ;
			glEnd();

			glBindTexture( GL_TEXTURE_2D, 0);
		}

		Gwen::Color SFML::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default )
		{
			#if SFML_VERSION_MAJOR == 2

				const sf::Texture* tex = static_cast<sf::Texture*>( pTexture->data );
				if ( !tex ) return col_default;

				sf::Image img = tex->CopyToImage();
				sf::Color col = img.GetPixel( x, y );
				return Gwen::Color( col.r, col.g, col.b, col.a );

			#else 

				const sf::Image* tex = static_cast<sf::Image*>( pTexture->data );
				if ( !tex ) return col_default;

				sf::Color col = tex->GetPixel( x, y );
				return Gwen::Color( col.r, col.g, col.b, col.a );

			#endif 

		}

	
	}
}
