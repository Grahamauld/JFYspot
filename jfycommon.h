//Modified by Graham Auld March 2019
#ifndef JFY_H
#define JFY_H

namespace Jfy
{
	enum RequestCode {
		InvalidRequestCode = 0x0000,
		// Registration codes
		OfflineQuery = 0x3040,
		SendRegisterAddress = 0x3041,
		RemoveRegister = 0x3042,
		ReconnectRemovedInverter = 0x3043,
		ReRegister = 0x3044,
		// Read codes
		ReadDescription = 0x3140,
		ReadWriteDescription = 0x3141,
		QueryNormalInfo = 0x3142,
		QueryInverterIdInfo = 0x3143,
		ReadSetInfo = 0x3144,
		ReadRtcTime = 0x3145,
		ReadModelInfo = 0x3146,
		ReadRielloFixSize = 0x3147,
		ReadPv33SlaveAInfo = 0x3148,
		ReadPv33SlaveBInfo = 0x3149,
		ReadDcCurrentInjection = 0x314a,
		ReadMasterSlaveLoggerVersion = 0x314b,
		//Write codes
		WriteVpvStart = 0x3240,
		WriteTStart = 0x3241,
		WriteVpvHighStop = 0x3242,
		WriteVpvLowStop = 0x3243,
		WriteVacMin = 0x3244,
		WriteVacMax = 0x3245,
		WriteFacMin = 0x3246,
		WriteFacMax = 0x3247,
		WriteVacMax2 = 0x3252,
		WriteVacMin2 = 0x3253,
		WriteFacMax2 = 0x3254,
		WriteFacMin2 = 0x3255,
		WriteVacMaxT = 0x3256,
		WriteVacMinT = 0x3257,
		WriteFacMaxT = 0x3258,
		WriteFacMinT = 0x3259,
		WriteActivePowerCode = 0x3250,
		WriteReactivePowerCode = 0x3251,
		WriteSerNum = 0x324C,
		//Execute Codes
		ExeResetEtotalAndHtotal = 0x3340,
		ExeCalibrateGridVoltsPhaseR = 0x3350,
		ExeCalibrateGridVoltsPhaseS = 0x3355,
		ExeCalibrateGridVoltsPhaseT = 0x3360,
		ExeCalibrateGridPower = 0x3351,
		ExeCalibrateRTCTime = 0x3352,
		ExeCalibratePV1Volts = 0x3353,
		ExeCalibratePV2Volts = 0x3354,
		ExeConfigMachineParamaters = 0x3358
	};

	enum ResponseCode {
		InvalidResponseCode = 0x00,
		AckResponseCode = 0x06,
		NackResponseCode = 0x15,
		// Registration codes
		OfflineQueryResponseCode = 0x30bf,
		SendRegisterAddressResponseCode = 0x30be,
		RemoveRegisterResponseCode = 0x30bd,
		ReconnectRemovedInverterResponseCode = 0x30bc,
		ReRegisterResponseCode = 0x30bb,
		// Read codes
		ReadDescriptionResponseCode = 0x31bf,
		ReadWriteDescriptionResponseCode = 0x31be,
		QueryNormalInfoResponseCode = 0x31bd,
		QueryInverterIdInfoResponseCode = 0x31bc,
		ReadSetInfoResponseCode = 0x31bb,
		ReadRtcTimeResponseCode = 0x31ba,
		ReadModelInfoResponseCode = 0x31b9,
		RielloFixSizeResponseCode = 0x31b8,
		Pv33SlaveAInfoResponseCode = 0x31b7,
		Pv33SlaveBInfoResponseCode = 0x31b6,
		ReadDcCurrentInjectionResponseCode = 0x31b5,
		ReadMasterSlaveLoggerVersionResponseCode = 0x31b4
	};

	template< typename T > T bitReverse( T value ) {
		T result = value;
		int s = sizeof( value ) * 8 - 1;

		for ( value >>= 1; value; value >>= 1 ) {
			result <<= 1;
			result |= value & 1;
			s--;
		}

		result <<= s;

		return result;
	}

	class Common
	{
	public:
		static short buildShort( int b1, int b2 );
		static long buildLong( int b1, int b2, int b3, int b4 );
	};
}

#endif // JFY_H
