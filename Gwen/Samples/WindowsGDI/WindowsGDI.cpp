
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <GdiPlus.h>

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Renderers/GDIPlus.h"

#pragma comment( lib, "gdiplus.lib" )



HWND CreateGameWindow( void )
{
	WNDCLASS	wc;
	ZeroMemory( &wc, sizeof( wc ) );

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= DefWindowProc;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.lpszClassName	= L"GWENWindow";
	wc.hCursor			= LoadCursor( NULL, IDC_ARROW );

	RegisterClass( &wc );

	HWND hWindow = CreateWindowEx( (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) , wc.lpszClassName, L"GWEN - GDI+ Sample", (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME), -1, -1, 1004, 650, NULL, NULL, GetModuleHandle(NULL), NULL );

	ShowWindow( hWindow, SW_SHOW );
	SetForegroundWindow( hWindow );
	SetFocus( hWindow );

	return hWindow;
}

HWND						g_pHWND = NULL;

int main()
{

	//
	// Create a new window
	//
	g_pHWND = CreateGameWindow();

	//
	// Create a GWEN GDI+ Renderer
	// Note: we're using the buffered version.
	// This version draws to a texture and then draws that texture to the window
	// This prevents all the crazy flickering (test with Gwen::Renderer::GDIPlus to see)
	//
	Gwen::Renderer::GDIPlusBuffered* pRenderer = new Gwen::Renderer::GDIPlusBuffered( g_pHWND );

	//
	// Create a GWEN skin
	//
	//Gwen::Skin::Simple skin;
	Gwen::Skin::TexturedBase skin;
	skin.SetRender( pRenderer );
	skin.Init( "DefaultSkin.png" );

	//
	// Create a Canvas (it's root, on which all other GWEN panels are created)
	//
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( &skin );
	pCanvas->SetSize( 998, 650 - 24 );
	pCanvas->SetDrawBackground( true );
	pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );

	//
	// Create our unittest control (which is a Window with controls in it)
	//
	UnitTest* pUnit = new UnitTest( pCanvas );
	pUnit->SetPos( 10, 10 );

	//
	// Create a Windows Control helper 
	// (Processes Windows MSG's and fires input at GWEN)
	//
	Gwen::Input::Windows GwenInput;
	GwenInput.Initialize( pCanvas );

	//
	// Begin the main game loop
	//
	MSG msg;
	while( true )
	{
		// Skip out if the window is closed
		if ( !IsWindowVisible( g_pHWND ) )
			break;

		// If we have a message from windows..
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			// .. give it to the input handler to process
			GwenInput.ProcessMessage( msg );

			// if it's QUIT then quit..
			if ( msg.message == WM_QUIT )
				break;

			if ( msg.message == WM_PAINT )
			{
				// This doesn't actually draw it, it just marks it
				// so it will redraw when next checked (NeedsRedraw)
				pCanvas->Redraw();
			}

			// Handle the regular window stuff..
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}

		// If GWEN's Canvas needs a redraw then redraw it
		//
		// In your game you would probably just draw it 
		//  every frame. But we have the option of only
		//  drawing it when it needs it too.
		//
		if ( pCanvas->NeedsRedraw() )
		{
			pCanvas->RenderCanvas();
		}
	}

}