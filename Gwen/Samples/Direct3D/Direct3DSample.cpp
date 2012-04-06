#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <d3d9.h>
#include <D3dx9core.h>
#include <D3dx9math.h>
#include <DxErr.h>

// C RunTime Header Files
#include <stdlib.h>
#include <memory.h>
#include <tchar.h>

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"
#include "Gwen/Renderers/DirectX9.h"

#pragma comment( lib, "d3dxof.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "dxerr.lib" )
#pragma comment( lib, "d3dx9.lib" )

HWND					g_pHWND = NULL;
LPDIRECT3D9				g_pD3D = NULL;
IDirect3DDevice9*		g_pD3DDevice = NULL;
D3DPRESENT_PARAMETERS	g_D3DParams;

//
// Windows bullshit to create a Window to render to.
//
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

	HWND hWindow = CreateWindowEx( (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) , wc.lpszClassName, L"GWEN - Direct 3D Sample", (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME), -1, -1, 1004, 650, NULL, NULL, GetModuleHandle(NULL), NULL );

	ShowWindow( hWindow, SW_SHOW );
	SetForegroundWindow( hWindow );
	SetFocus( hWindow );

	return hWindow;
}


//
// Typical DirectX stuff to create a D3D device
//
void ResetD3DDevice()
{
	g_pD3DDevice->Reset( &g_D3DParams );
}

void CreateD3DDevice()
{
	ZeroMemory( &g_D3DParams, sizeof( g_D3DParams ) );

	RECT ClientRect;
	GetClientRect( g_pHWND, &ClientRect );

	g_D3DParams.Windowed = TRUE;
	g_D3DParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_D3DParams.BackBufferWidth = ClientRect.right;
	g_D3DParams.BackBufferHeight = ClientRect.bottom;
	g_D3DParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	g_D3DParams.BackBufferFormat = D3DFMT_X8R8G8B8;
	//g_D3DParams.EnableAutoDepthStencil = TRUE;
	//g_D3DParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	g_D3DParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_pHWND, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_D3DParams, &g_pD3DDevice );
	if ( FAILED(hr) )
	{
		OutputDebugString( DXGetErrorDescription( hr ) );
		return;
	}
}

//
// Program starts here
//
int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//
	// Create a window and attach directx to it
	//
	g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	g_pHWND = CreateGameWindow();
	CreateD3DDevice();	

	//
	// Create a GWEN DirectX renderer
	//
	Gwen::Renderer::DirectX9* pRenderer = new Gwen::Renderer::DirectX9( g_pD3DDevice );

	//
	// Create a GWEN skin
	//
	Gwen::Skin::TexturedBase skin;
	skin.SetRender( pRenderer );
	skin.Init( "DefaultSkin.png" );

	//
	// Create a Canvas (it's root, on which all other GWEN panels are created)
	//
	Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( &skin );
	pCanvas->SetSize( 1000, 622 );
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

			// Handle the regular window stuff..
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Normal DirectX rendering loop
			g_pD3DDevice->BeginScene();

				g_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 0 ), 1, 0 );

				// This is how easy it is to render GWEN!
				pCanvas->RenderCanvas();

			g_pD3DDevice->EndScene();
			g_pD3DDevice->Present( NULL, NULL, NULL, NULL );

		}
	}

	if ( g_pD3DDevice )
	{
		g_pD3DDevice->Release();
		g_pD3DDevice = NULL;
	}

	if ( g_pD3D )
	{
		g_pD3D->Release();
		g_pD3D = NULL;
	}
}