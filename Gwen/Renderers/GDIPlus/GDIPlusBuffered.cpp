
#include <windows.h>
#include <gdiplus.h>

#include "Gwen/Renderers/GDIPlus.h"

#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"

namespace Gwen
{
	namespace Renderer
	{
		GDIPlusBuffered::GDIPlusBuffered( HWND pHWND ) : GDIPlus( pHWND )
		{
			m_Bitmap = NULL;
		}

		GDIPlusBuffered::~GDIPlusBuffered()
		{
			DestroyBackbuffer();
		}

		void GDIPlusBuffered::CreateBackbuffer()
		{
			// Check the Client Gwen::Rect size (of supplied HWND)
			// If it has changed, we need to recreate the backbuffer
			{
				RECT rect;
				GetClientRect( m_HWND, &rect );

				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;

				if ( m_iWidth != width ) DestroyBackbuffer();
				if ( m_iHeight != height ) DestroyBackbuffer();

				m_iWidth = width;
				m_iHeight = height;
			}

			if ( m_Bitmap ) return;

			Gdiplus::Graphics gfx( m_hDC );
			m_Bitmap = new Gdiplus::Bitmap( m_iWidth, m_iHeight, &gfx );
			graphics = Gdiplus::Graphics::FromImage( m_Bitmap );
		}

		void GDIPlusBuffered::DestroyBackbuffer()
		{
			if ( m_Bitmap )
			{ 
				delete m_Bitmap; 
				m_Bitmap = NULL; 
			}

			if ( graphics )
			{
				delete graphics;
				graphics = NULL;
			}
		}

		void GDIPlusBuffered::Begin()
		{
			m_hDC = GetDC( m_HWND );
			CreateBackbuffer();
		}

		void GDIPlusBuffered::End()
		{
			Gdiplus::Graphics gfx( m_hDC );
			gfx.DrawImage( m_Bitmap, 0, 0 );

			ReleaseDC( m_HWND, m_hDC );
			m_hDC = NULL;
		}
	}
}