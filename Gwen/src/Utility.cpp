/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#include "Gwen/ToolTip.h"
#include "Gwen/Utility.h"
#include "Gwen/TextObject.h"

using namespace Gwen;

#ifdef _MSC_VER
	#pragma warning(disable:4267)// conversion from 'size_t' to 'int', possible loss of data
#endif

UnicodeString Gwen::Utility::Format( const wchar_t* fmt, ... )
{
	wchar_t strOut[ 4096 ];

	va_list s;
	va_start( s, fmt ); 
	vswprintf( strOut, sizeof(strOut), fmt, s );
	va_end(s);

	UnicodeString str = strOut;
	return str;
}



void Gwen::Utility::Strings::Split( const Gwen::String& str, const Gwen::String& seperator, Strings::List& outbits, bool bLeave )
{
	int iOffset = 0;
	int iLength = str.length();
	int iSepLen = seperator.length();

	size_t i = str.find( seperator, 0 );
	while ( i != std::string::npos )
	{
		outbits.push_back( str.substr( iOffset, i-iOffset ) );
		iOffset = i + iSepLen;

		i = str.find( seperator, iOffset );
		if ( bLeave ) iOffset -= iSepLen;
	}

	outbits.push_back( str.substr( iOffset, iLength-iOffset ) );
}

void Gwen::Utility::Strings::Split( const Gwen::UnicodeString& str, const Gwen::UnicodeString& seperator, Strings::UnicodeList& outbits, bool bLeave )
{
	int iOffset = 0;
	int iLength = str.length();
	int iSepLen = seperator.length();

	size_t i = str.find( seperator, 0 );
	while ( i != std::wstring::npos )
	{
		outbits.push_back( str.substr( iOffset, i-iOffset ) );
		iOffset = i + iSepLen;

		i = str.find( seperator, iOffset );
		if ( bLeave ) iOffset -= iSepLen;
	}

	outbits.push_back( str.substr( iOffset, iLength-iOffset ) );
}

int Gwen::Utility::Strings::To::Int( const Gwen::String& str )
{
	if ( str == "" ) return 0;
	return atoi( str.c_str() );
}

float Gwen::Utility::Strings::To::Float( const Gwen::String& str )
{
	if ( str == "" ) return 0.0f;
	return (float)atof( str.c_str() );
}

bool Gwen::Utility::Strings::To::Bool( const Gwen::String& str )
{
	if ( str.size() == 0 ) return false;
	if ( str[0] == 'T' || str[0] == 't' || str[0] == 'y' || str[0] == 'Y' ) return true;
	if ( str[0] == 'F' || str[0] == 'f' || str[0] == 'n' || str[0] == 'N' ) return false;
	if ( str[0] == '0' ) return false;
	return true;
}

bool Gwen::Utility::Strings::To::Floats( const Gwen::String& str, float* f, size_t iCount )
{
	Strings::List lst;
	Strings::Split( str, " ", lst );
	if ( lst.size() != iCount ) return false;

	for ( size_t i=0; i<iCount; i++ )
	{
		f[i] = Strings::To::Float( lst[i] );
	}

	return true;
}


bool Gwen::Utility::Strings::Wildcard( const TextObject& strWildcard, const TextObject& strHaystack )
{
	const UnicodeString& W = strWildcard.GetUnicode();
	const UnicodeString& H = strHaystack.GetUnicode();

	if ( strWildcard == "*" ) return true;

	int iPos = W.find( L"*", 0 );
	if ( iPos == UnicodeString::npos ) return strWildcard == strHaystack;

	// First half matches
	if ( iPos > 0 && W.substr( 0, iPos ) != H.substr( 0, iPos ) )
		return false;

	// Second half matches
	if ( iPos != W.length()-1 )
	{
		UnicodeString strEnd = W.substr( iPos+1, W.length() );

		if ( strEnd != H.substr( H.length() - strEnd.length(), H.length() ) )
			return false;
	}

	return true;
}



void Gwen::Utility::Strings::ToUpper( Gwen::UnicodeString& str )
{
	transform( str.begin(), str.end(), str.begin(), towupper );
}

void Gwen::Utility::Strings::Strip( Gwen::UnicodeString& str, const Gwen::UnicodeString& chars )
{
	Gwen::UnicodeString Source = str;
	str = L"";

	for ( int i =0; i<Source.length(); i++ )
	{
		if ( chars.find( Source[i] ) != Gwen::UnicodeString::npos )
			continue;

		str += Source[i];
	}
}



