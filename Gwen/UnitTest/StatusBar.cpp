#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Controls/StatusBar.h"
#include "Gwen/Controls/Label.h"

using namespace Gwen;

class StatusBar : public GUnit
{
	public:

	GWEN_CONTROL_INLINE( StatusBar, GUnit )
	{
		Gwen::Controls::StatusBar* pStatus = new Gwen::Controls::StatusBar( this );

		Gwen::Controls::Label* pRight = new Gwen::Controls::Label( pStatus );
		pRight->SetText( L"Label Added to Right" );
		pStatus->AddControl( pRight, true );
	}

	void Layout( Gwen::Skin::Base* skin ){}
};



DEFINE_UNIT_TEST( StatusBar, L"StatusBar" );