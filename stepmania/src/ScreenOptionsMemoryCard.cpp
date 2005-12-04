#include "global.h"
#include "ScreenOptionsMemoryCard.h"
#include "RageLog.h"
#include "arch/MemoryCard/MemoryCardDriver.h"
#include "MemoryCardManager.h"
#include "GameState.h"
#include "ScreenManager.h"
#include "ScreenPrompt.h"


REGISTER_SCREEN_CLASS( ScreenOptionsMemoryCard );
ScreenOptionsMemoryCard::ScreenOptionsMemoryCard( CString sClassName ) : ScreenOptions( sClassName )
{
	LOG->Trace( "ScreenOptionsMemoryCard::ScreenOptionsMemoryCard()" );
}

void ScreenOptionsMemoryCard::Init()
{
	ScreenOptions::Init();

	FOREACH_CONST( UsbStorageDevice, MEMCARDMAN->GetStorageDevices(), iter )
	{
		// TODO: Make these string themable

		vector<CString> vs;
		if( iter->sOsMountDir.empty() )
			vs.push_back( "(no mount dir)" );
		else
			vs.push_back( iter->sOsMountDir );
		if( iter->sVolumeLabel.empty() )
			vs.push_back( "(no label)" );
		else
			vs.push_back( iter->sVolumeLabel );
		if( iter->iVolumeSizeMB == 0 )
			vs.push_back( "size ???" );
		else
			vs.push_back( ssprintf("%dMB",iter->iVolumeSizeMB) );

		CString sDescription = join(", ", vs);
		OptionRowDefinition def( sDescription, true, "" );
		def.m_sExplanationName = "Memory Card";
		def.m_bAllowThemeTitle = false;
		m_vDefs.push_back( def );	
	}

	if( MEMCARDMAN->GetStorageDevices().empty() )
	{
		OptionRowDefinition def( "No memory cards detected", true, "" );
		m_vDefs.push_back( def );	
	}
	
	vector<OptionRowHandler*> vHands( m_vDefs.size(), NULL );

	InitMenu( m_vDefs, vHands );
}

void ScreenOptionsMemoryCard::BeginScreen()
{
	ScreenOptions::BeginScreen();

	// select the last chosen memory card (if present)
	if( !MEMCARDMAN->m_sEditorMemoryCardOsMountPoint.Get().empty() )
	{
		const vector<UsbStorageDevice> &v = MEMCARDMAN->GetStorageDevices();
		for( unsigned i=0; i<v.size(); i++ )	
		{
			if( v[i].sOsMountDir == MEMCARDMAN->m_sEditorMemoryCardOsMountPoint.Get() )
			{
				this->MoveRowAbsolute( PLAYER_1, i, false );
				break;
			}
		}
	}

	this->SubscribeToMessage( Message_StorageDevicesChanged );
}

void ScreenOptionsMemoryCard::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenOptions::HandleScreenMessage( SM );
}

void ScreenOptionsMemoryCard::HandleMessage( const CString& sMessage )
{
	if( sMessage == MessageToString(Message_StorageDevicesChanged) )
	{
		if( !m_Out.IsTransitioning() )
			SCREENMAN->SetNewScreen( this->m_sName );	// reload
	}
}

void ScreenOptionsMemoryCard::MenuStart( const InputEventPlus &input )
{
	ScreenOptions::MenuStart( input );
}

void ScreenOptionsMemoryCard::ImportOptions( int iRow, const vector<PlayerNumber> &vpns )
{

}

void ScreenOptionsMemoryCard::ExportOptions( int iRow, const vector<PlayerNumber> &vpns )
{
	OptionRow &row = *m_pRows[iRow];
	if( row.GetRowType() == OptionRow::ROW_EXIT )
		return;

	PlayerNumber pn = GAMESTATE->m_MasterPlayerNumber;
	if( m_iCurrentRow[pn] == iRow )
	{
		const vector<UsbStorageDevice> &v = MEMCARDMAN->GetStorageDevices();
		if( iRow < v.size() )
		{
			const UsbStorageDevice &dev = v[iRow];
			MEMCARDMAN->m_sEditorMemoryCardOsMountPoint.Set( dev.sOsMountDir );
		}
	}
}

void ScreenOptionsMemoryCard::ProcessMenuStart( const InputEventPlus &input )
{
	int iCurRow = m_iCurrentRow[GAMESTATE->m_MasterPlayerNumber];

	const vector<UsbStorageDevice> &v = MEMCARDMAN->GetStorageDevices();
	if( iCurRow < v.size() )	// a card
	{
		const vector<UsbStorageDevice> v = MEMCARDMAN->GetStorageDevices();
		const UsbStorageDevice &dev = v[iCurRow];
		MEMCARDMAN->m_sEditorMemoryCardOsMountPoint.Set( dev.sOsMountDir );
		bool bSuccess = MEMCARDMAN->MountCard( PLAYER_1, dev );
		if( bSuccess )
		{
			this->BeginFadingOut();
		}
		else
		{
			CString s = ssprintf("error mounting card: %s", MEMCARDMAN->GetCardError(PLAYER_1).c_str() );
			ScreenPrompt::Prompt( SM_None, s );
		}
	}
	else
	{
		SCREENMAN->PlayInvalidSound();
	}
}

/*
 * (c) 2005 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
