//============================================================================
// TwsApi Test
//============================================================================
#include "TwsApiL0.h"
#include "TwsApiDefs.h"
using namespace TwsApi;

// to use the Sleep function
#ifdef WIN32
	#include <windows.h>		// Sleep(), in miliseconds
	#include <process.h>
	#define CurrentThreadId GetCurrentThreadId
#else
	#include <unistd.h>			// usleep(), in microseconds
	#define Sleep( m ) usleep( m*1000 )
	#include <pthread.h>
	#define CurrentThreadId pthread_self
#endif

#define PrintProcessId printf("%-8d  ", CurrentThreadId() )

#include <time.h>



//----------------------------------------------------------------------------
// MyEWrapper
//----------------------------------------------------------------------------
class MyEWrapper: public EWrapperL0
{
	virtual void tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute )
	{
	///	int x = 10, y = 0; x = x/y; // Divide by zero to test EWrapperL0Protected
	///	int *x = 0; *x = 100;

	///	throw(0);

	//	time_t		_t	= GetEventTime();		// !! from L1
		time_t		_t; time(&_t);
		struct tm*	_tm	= localtime( &_t );
		PrintProcessId,printf
			( "TP: %4d %02d:%02d:%02d %10s %5.3f\n"
			, tickerId
			, _tm->tm_hour, _tm->tm_min, _tm->tm_sec
			, *(TickTypes::ENUMS)field, price
			);
	}

	virtual void tickSize( TickerId tickerId, TickType field, int size )
	{
	//	time_t		_t	= GetEventTime();		// !! from L1
		time_t		_t; time(&_t);
		struct tm*	_tm	= localtime( &_t );
		PrintProcessId,printf
			( "TS: %4d %02d:%02d:%02d %10s %5d\n"
			, tickerId
			, _tm->tm_hour, _tm->tm_min, _tm->tm_sec
			, *(TickTypes::ENUMS)field, size
			);
	}

	virtual void winError( const std::string& str, int lastError )
	{
		PrintProcessId,printf( "WinError: %d = %s\n", lastError, (const char*)str );
	}

	virtual void connectionClosed()
	{
		PrintProcessId,printf( "Connection Closed\n");
	}

	virtual void updateAccountValue( const std::string& key, const std::string& val, const std::string& currency, const std::string& accountName )
	{
		UpdateAccountValueKey::ENUMS UAVK;
		if( UAVK *= key )
			PrintProcessId,printf( "AC: %s %5s   %-30s %s\n", (const char*)accountName, (const char*)currency, *UAVK,            (const char*)val );
		else
			PrintProcessId,printf( "AC: %s %5s ? %-30s %s\n", (const char*)accountName, (const char*)currency, (const char*)key, (const char*)val );
	}

	virtual void accountDownloadEnd( const std::string& accountName )
	{
		PrintProcessId,printf( "AC: %s end\n", (const char*)accountName );
	}

	virtual void nextValidId( OrderId orderId )
	{
		PrintProcessId,printf( "nextValidId = %d\n", orderId );
	}

	virtual void contractDetails( const ContractDetails& contractDetails )
	{
		const Contract& C = contractDetails.summary;

		PrintProcessId,printf	( "CD: %10s %5s %8s, %5.2f\n", C.localSymbol, (const char*)C.secType, (const char*)C.expiry, C.strike );
	}

	virtual void error( const int id, const int errorCode, const std::string errorString )
	{
		PrintProcessId,printf( "Error for id=%d: %d = %s\n", id, errorCode, (const char*)errorString );
	}

	virtual void historicalData( TickerId reqId, const std::string& date, double open, double high, double low, double close, int volume, int barCount, double WAP, int hasGaps )
	{
		if( IsEndOfHistoricalData(date) )
		{
			PrintProcessId,printf( "HD: %s\n", Finished() );
			return;
		}

		PrintProcessId,printf( "HD: %4d %10s %5.3f %5.3f %5.3f %5.3f %7d %7d\n", reqId, (const char*)date, open, high, low, close, volume, barCount );
	}

   virtual void updateMktDepth(TickerId id, int position, int operation, int side, double price, int size )
   {
	   PrintProcessId,printf
			( "MD: %4d %2d %10s %5s %7.2f %5d\n"
			, id
			, position
			, *(MktDepthOperation::ENUMS) operation
			, *(MktDeptSide::ENUMS)side
			, price
			, size
			);
   }

   virtual void updateMktDepthL2(TickerId id, int position, std::string marketMaker, int operation, int side, double price, int size )
   {
   }


	virtual void connectionOpened( void )
	{
		PrintProcessId,printf( "Connection Opened\n");
	}

	virtual void checkMessagesStarted( void )
	{
		PrintProcessId,printf( ">>> checkMessagesStarted\n");
	}

	virtual void checkMessagesStopped( void )
	{
		PrintProcessId,printf( "<<< checkMessagesStopped\n");
	}
};

//----------------------------------------------------------------------------
// TestEnums
//----------------------------------------------------------------------------
void TestEnums( void )
{
	std::string			x = *TickTypes::Bid;
	x = *OrderStatus::PendingCancel;

	std::string			y;
	y = *TriggerMethod::LastPrice;

	OrderStatus::ENUMS	e;
	if( e *= "PendingCancel" )
	{ printf("OK\t"); }
	else
	{ printf("NOK\t"); }
	printf( "%4d %s\n", e, *e );


	if( e *= "not a status" )
	{ printf("OK\t"); }
	else
	{ printf("NOK\t"); }
	printf( "%4d %s\n", e, *e );

	int a = 11 * OrderStatus::PendingCancel;

	switch( OrderStatus("PendingCancelxxxx") )
	{
	case OrderStatus::PendingCancel:
		{ printf("OK\n"); } break;
	case OrderStatus::_INVALID_:
		{ printf("NOK\n"); } break;
	default:
		{ printf("??\n"); } break;
	}

	// The iterator has a similar interface as the of the std::map
	for( UpdateAccountValueKey::iterator ac = UpdateAccountValueKey::begin(); ac != UpdateAccountValueKey::end(); ++ac )
		printf( "%4d %s\n", ac->first, ac->second );

}


//----------------------------------------------------------------------------
// main
//----------------------------------------------------------------------------
#define TEST( T, X ) ( printf( "T%7d %s\n", T, #X ), X )

int main( void )
{
//	TestEnums();

	Contract			C;
	C.symbol			= "MSFT";
	C.secType			= *SecType::STK;		//"STK"
	C.currency			= "USD";
	C.exchange			= *Exchange::IB_SMART;	//"SMART";
//	C.primaryExchange	= *Exchange::AMEX;

	// from version 9.63 on, the protected ewrapper is active by default
	MyEWrapper	MW;
	EClientL0*	EC = EClientL0::New( &MW );

	printf( "ClientVersion = %d\n", EC->clientVersion() );

	TEST(0, EC->eDisconnect() );	// only for test purposes

	if( TEST(0, EC->eConnect( "", 7496, 100 )) )
	{
		PrintProcessId,printf( "ServerVersion = %d\n", EC->serverVersion() );

//		EC->reqNewsBulletins( true );
//		EC->reqNewsBulletins( true );
/** /
		TEST( 0, EC->reqAccountUpdates( true, "" ) );
/**/

//		for( int i = 0; i < 60; i++ )
		TEST( 100, EC->reqMktData( 100, C, "", false ) );
/** /
		EC->reqMktDepth( 11, C, 3 );
/** /
		EC->reqHistoricalData
			( 20
			, C
			, EndDateTime(2008,07,10), DurationStr(1, DH_Days), *BarSizeSetting::_1_hour
			, *WS_TRADES
			, true
			, FD_AsDate
			);
/** /
//		EC->reqMktData( 20, C, false );
		EC->reqHistoricalData
		( 20//EC->GetNextValidId()
		, C
		, EndDateTime(2006,10,03), DurationStr(1, DH_Days), *BarSizeSetting::_1_secs
		, *WS_TRADES
		, true
		, FD_AsDate
		);
/** /
		{
		Contract C;
		C.symbol	= "c";
		C.secType	= *SecType::OPT;		//"STK"
		C.currency	= "USD";
		C.exchange	= *Exchange::IB_SMART;	//"SMART";
		EC->reqContractDetails( C );


//		EC->reqContractDetails( C );
		}
/** /QQQQ
		C.symbol	= "DNEX";
		EC->reqContractDetails( C );

		C.symbol	= "MSFT";
		EC->reqContractDetails( C );
/**/
	}

	int x = 1;
	while( x++ < 1000 )
	{
		if( !MW.IsCalledFromThread() ) EC->checkMessages();
		Sleep( 100 );

	/* for 'stress' testing */
		if( 0 == x%50)
			TEST(x, EC->reqMktData( x, C, "", false ) );
	/**/
		if( 30 == x)
			TEST(x, EC->eDisconnect() );
	/**/
		if( 40 == x)
			TEST(x, EC->eDisconnect() );


		if( 60 == x || 70 == x )
			TEST(x, EC->reqMktData( x, C, "", false ) );	// will fail of course


		if( 130 == x)
			TEST(x, EC->eConnect( "", 7496, 10 ) );

		if( 150 == x)
			TEST(x, EC->eConnect( "", 7496, 10 ) );		// raises already connected error

		if( 190 == x )
			TEST(x, 
			EC->reqHistoricalData
				( x
				, C
				, EndDateTime(2010,07,23), DurationStr(1, *DurationHorizon::Days), *BarSizeSetting::_1_hour
				, *WhatToShow::TRADES
				, true
				, FormatDate::AsDate
				)
			);

		if( 200 == x )
			TEST(x, EC->reqMktData( x, C, "", false ) );

	/**/
		if( 800 == x)
			TEST(x, EC->cancelMktData( 200 ) );
	/**/
	}


	TEST( 0, EC->eDisconnect() );

	delete EC;

	{ PrintProcessId,printf( "Press return to end\n" ); char s[10]; gets(s); }
	return 0;
}
