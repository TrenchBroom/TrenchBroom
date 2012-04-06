#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Controls/CollapsibleList.h"

using namespace Gwen;

class CollapsibleList : public GUnit
{
	public:

		GWEN_CONTROL_INLINE( CollapsibleList, GUnit )
		{
			Gwen::Controls::CollapsibleList* pControl = new Gwen::Controls::CollapsibleList( this );
			pControl->SetSize( 100, 200 );
			pControl->SetPos( 10, 10 );

			{
				Gwen::Controls::CollapsibleCategory* cat = pControl->Add( "Category One" );
				cat->Add( "Hello" );
				cat->Add( "Two" );
				cat->Add( "Three" );
				cat->Add( "Four" );
			}

			{
				Gwen::Controls::CollapsibleCategory* cat = pControl->Add( "Shopping" );
				cat->Add( "Special" );
				cat->Add( "Two Noses" );
				cat->Add( "Orange ears" );
				cat->Add( "Beer" );
				cat->Add( "Three Eyes" );
				cat->Add( "Special" );
				cat->Add( "Two Noses" );
				cat->Add( "Orange ears" );
				cat->Add( "Beer" );
				cat->Add( "Three Eyes" );
				cat->Add( "Special" );
				cat->Add( "Two Noses" );
				cat->Add( "Orange ears" );
				cat->Add( "Beer" );
				cat->Add( "Three Eyes" );
			}

			{
				Gwen::Controls::CollapsibleCategory* cat = pControl->Add( "Category One" );
				cat->Add( "Hello" );
				cat->Add( "Two" );
				cat->Add( "Three" );
				cat->Add( "Four" );
			}
		}
};



DEFINE_UNIT_TEST( CollapsibleList, L"CollapsibleList" );