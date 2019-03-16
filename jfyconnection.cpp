//Modified by Graham Auld March 2019

#include "jfyconnection.h"

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include "jfyspot.h"
#include "jfycommon.h"
#include "jfyexception.h"
#include "jfyserial.h"

using namespace std;
using namespace Jfy;

Connection::Connection()
:	_conn( new Serial ),
	_registered( false ),
	_sourceAddress( 1 ),
	_destinationAddress( 1 )
{
}

Connection::Connection( const string& device )
:	_conn( new Serial( device ) ),
	_registered( false ),
	_sourceAddress( 1 ),
	_destinationAddress( 1 )
{
}

Connection::~Connection()
{
	close();
	delete _conn;
}

bool Connection::init()
{
	srand( time( 0 ) );
	
	if ( !_conn->open() )
		return false;

	try {
		// Re-register wtih the inverter
		_conn->sendRequest( Data( Jfy::ReRegister, _sourceAddress, 0 ) );
		
		// Get the serial number
		Data response = _conn->sendRequestReadResponse( Data( Jfy::OfflineQuery, _sourceAddress, 0 ) );

		if ( !response.isValid() )
			throw Exception( "Cannot read the serial number from the device." );

		_serialNumber = response.toString();

		sleep( 1 );

		Data request( SendRegisterAddress, _sourceAddress, 0 );
		request.addData( _serialNumber );
		request.addData( _destinationAddress );

		response = _conn->sendRequestReadResponse( request );

		if ( !response.isValid() || response.size() == 0 || response.data()[ 0 ] != AckResponseCode )
			throw Exception( "Cannot read the registration response." );

		_registered = true;
	}
	catch ( Exception e ) {
			//a little bodgy override if you want to force some comms with nothing connected
			//_registered=true;
			//return true;
		cerr << e.what() << endl;
		close();
		return false;
	}

	return true;
}

void Connection::close()
{
	if ( _registered ) {
		Data response = _conn->sendRequestReadResponse( Data( RemoveRegister, _sourceAddress, _destinationAddress ) );

		if ( !response.isValid() || response.size() == 0 || response.data()[ 0 ] != AckResponseCode )
			cerr << "Could not send the remove registration request to the device." << endl;
	}

	_conn->close();
	_sourceAddress = 0;
	_destinationAddress = 0;
	_registered = false;
	_serialNumber = "";
}

bool Connection::isRegistered() const
{
	return _registered;
}

string Connection::serialNumber() const
{
	return _serialNumber;
}

unsigned char Connection::registrationAddress() const
{
	return _destinationAddress;
}

bool Connection::readNormalInfo( InverterData* data )
{
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}

	Data response = _conn->sendRequestReadResponse( Data( QueryNormalInfo, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}

	const unsigned char* buf = response.data();
	int size = response.size();

	data->mode=buf[25];
	data->temperature = Common::buildShort( buf[ 0 ], buf[ 1 ] ) / 10.0;
	data->energyToday = Common::buildShort( buf[ 2 ], buf[ 3 ] ) / 100.0;
	data->voltageDc = Common::buildShort( buf[ 4 ], buf[ 5 ] ) / 10.0;
	data->currentAc = Common::buildShort( buf[ 6 ], buf[ 7 ] ) / 10.0;
	data->voltageAc = Common::buildShort( buf[ 8 ], buf[ 9 ] ) / 10.0;
	data->frequency = Common::buildShort( buf[ 10 ], buf[ 11 ] ) / 100.0;
	data->energyCurrent = Common::buildShort( buf[ 12 ], buf[ 13 ] ) / 1.0;
	//16 17 18 19 for energy total
	data->energyTotal = Common::buildLong( buf[16],buf[17],buf[18],buf[19]) / 10.0;
	data->currentDc = 0; 

	return true;
}

bool Connection::readInverterIDInfo( InverterIDData* data )
{
	cerr << "trying to read inverter id"<<endl;
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}


	Data response = _conn->sendRequestReadResponse( Data( QueryInverterIdInfo, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}
	cerr << "got a response"<<endl;
	const unsigned char* buf = response.data();
	int size = response.size();
	string ret[65];
	//ret=response.toString();

	data->phases=buf[0];
	memcpy(&data->VArating,&buf[1],6);
	memcpy(&data->firmwareVer,&ret[7],5);  
	memcpy(&data->modelName,&ret[12],16); 
	memcpy(&data->manufacturer,&ret[28],16);
	memcpy(&data->serialNumber,&ret[44],16); 
	memcpy(&data->nominalVpv,&ret[60],4); 

	return true;
}

bool Connection::syncRTC() {
	//registered?
	cerr << "trying to sync RTC" << endl;
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}
	if (verbose) {
		cerr << "stage 1 - read the rtc" << endl;
	}
	//start by reading RTC time
	Data response = _conn->sendRequestReadResponse( Data( ReadRtcTime, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}
	const unsigned char* buf = response.data();
	if (verbose) {
		fprintf(stderr, "response %u bytes\n %02X/%02X/%02X %02X:%02X", response.size(),buf[2],buf[1],buf[0],buf[3],buf[4]);
	}
//presumably this response was 5 bytes - year 0-9, month 0-12, day -31 hour 0-24 minutes 0-59

	// Get the current time
	time_t now = time(0);
	//Now have a bash at writing
//	Data response = _conn->sendRequestReadResponse( Data( ExeCalibrateRTCTime, _sourceAddress, _destinationAddress ) );
	Data request( ExeCalibrateRTCTime, _sourceAddress, _destinationAddress );
	request.addData("0"); //who cares about the year?
	request.addData(std::localtime(&now)->tm_mon+1);
	request.addData(std::localtime(&now)->tm_mday);
	request.addData(std::localtime(&now)->tm_hour);
	request.addData(std::localtime(&now)->tm_min);
	response = _conn->sendRequestReadResponse( request );

	buf = response.data();
	if (verbose) {
		fprintf(stderr,"response of %u bytes was %02X - 6 or 8 is proablby ack, 15ish is probably nack\n",response.size(),buf[0]);	
	}
	if (buf[0]==AckResponseCode) {
		return true;
	}
	return false;
}

bool Connection::readDescript() {
	//registered?
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}
	Data response = _conn->sendRequestReadResponse( Data( ReadDescription, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}
	return true;
}

bool Connection::readWriteDescript() {
	//registered?
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}
	Data response = _conn->sendRequestReadResponse( Data( ReadWriteDescription, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}
	return true;
}

bool Connection::readSetInfo() {
	//registered?
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}
	Data response = _conn->sendRequestReadResponse( Data( ReadSetInfo, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}
	return true;
}


bool Connection::readReiello() {
	//registered?
	if ( !isRegistered() ) {
		cerr << "Device is not registered." << endl;
		return false;
	}
	Data response = _conn->sendRequestReadResponse( Data( ReadRielloFixSize, _sourceAddress, _destinationAddress ) );

	if ( !response.isValid() ) {
		cerr << "Could not read the information response." << endl;
		return false;
	}
	return true;
}

