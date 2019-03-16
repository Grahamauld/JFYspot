//Modified by Graham Auld March 2019

#ifndef JFYCONNECTION_H
#define JFYCONNECTION_H

#include <string>

using namespace std;

namespace Jfy
{
	typedef struct {
		unsigned char  mode;
		float temperature; 
		float energyToday;  
		float voltageDc; 
		float currentAc;
		float voltageAc; 
		float frequency; 
		float energyCurrent; 
		float currentDc; 
		float energyTotal;
	} InverterData;

	typedef struct {
		unsigned char  phases;
		string VArating; 
		string firmwareVer;  
		string modelName; 
		string manufacturer;
		string serialNumber; 
		string nominalVpv; 
	} InverterIDData;
	
	class Serial;
	
	class Connection
	{
	public:
		Connection();
		Connection( const string& device );
		virtual ~Connection();

		bool init();
		void close();

		bool isRegistered() const;
		string serialNumber() const;
		unsigned char registrationAddress() const;

		bool readNormalInfo( InverterData* data );
		bool readInverterIDInfo( InverterIDData* data );
		bool syncRTC();

		bool readDescript();
		bool readWriteDescript();
		bool readSetInfo();
		bool readReiello();

	private:
		Serial* _conn;
		bool _registered;
		string _serialNumber;
		unsigned char _sourceAddress;
		unsigned char _destinationAddress;
	};
}

#endif // JFYCONNECTION_H
