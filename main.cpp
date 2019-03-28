//Modified by Graham Auld March 2019
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <string.h>
#include <sstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "jfyspot.h"
#include "jfyconnection.h"

using namespace std;

const char *switch_device = "-device";
const char *switch_info = "-info";
const char *switch_sync = "-sync";
const char *switch_check = "-check";
const char *switch_help = "-help";
const char *switch_verbose = "-verbose";
const char *switch_test = "-test";

bool	verbose=false;

void usage(char *prog_name) {
  cout << "JFYspot is designed to extract spot point readings from your JFY inverter and produce an output compatable with 123solar similarly to SBFspot" << endl
       << "Usage: " << prog_name << " ["
       << " " << switch_device << "/dev/ttyUSB0" 
       << " " << switch_info
       << " " << switch_sync
       << " " << switch_check
       << " " << switch_verbose
       << " " << switch_help
       <<  " ]" << endl
       << switch_device << ": choose which serial device to connect to" << endl
       << switch_info << ": show some more detailed system information" << endl
       << switch_sync << ": RTC sync - not available on 3000 so untested" << endl
       << switch_check << ": not yet implimented - would check the alarms and return some status" << endl
       << switch_verbose << ": turn on more verbose output to stderr" << endl
       << switch_help    << ": display this help text" << endl;
}



//further considerations
//it's called once per second - just try the comms and only fall back to setting up the intverter address if it dosn't work
//support -info -sync
//and -check
//sync is for time set
//check is for checking alarms
//info is run at startup to find out about the inverter - probably a good one to start by connecting
//still to work out how to change the list of data sent to include string current!!!
//what about -log??? could log raw output to /dev/shm/jfyspot.log (restarting the file at day change?)
//and -v verbose - turn on or re-direct all the error info???

int main( int argc, char** argv )
{
	bool	do_info=false;
	bool	do_sync=false;
	bool	do_test=false;
	string serialPort = "/dev/ttyUSB0";
//	if (argc < 2) {
//		usage(argv[0]);
//		return 1;
//	}

for (unsigned i = 1; i < static_cast<unsigned>(argc); ++i) {
   if (strncmp(argv[i], switch_help, 2) == 0) {
      usage(argv[0]);
      return 0;
    }
    else if (strncmp(argv[i], switch_device, 2) == 0 ) {
//	printf("device paramater passed, %s\n",argv[i+1]);
	serialPort=argv[i+1];
    }
    else if (strncmp(argv[i], switch_info, 2) == 0) {
	do_info=true;
    }
    else if (strncmp(argv[i], switch_sync, 2) == 0) {
	do_sync=true;
    }
    else if(strncmp(argv[i], switch_check, 2) == 0) {
    }
    else if(strncmp(argv[i], switch_verbose, 2) == 0) {
	    verbose=true;
    }
    else if(strncmp(argv[i], switch_test, 2) == 0) {
	    do_test=true;
    } else {
	//just run the standard
    }
}


	float dcwatts;
	float eff;
	Jfy::Connection conn( serialPort );
	string logPathRoot = "/var/log/solarmonj/";
	string status= "NOK";
	

	if ( !conn.init() ) {
		cerr << "Cannot initialise the connection." << endl;
		return 1;
	}
	
	// Get the current time
	time_t now = time(0);
	
	// Get the data

	if (do_test) {
	conn.readDescript();
	conn.readWriteDescript();
	conn.readSetInfo();
	conn.readReiello();
	return 1;
	}

	if (do_sync) {
		if (conn.syncRTC()) {
			cout << "Succesfully sunk RTC" << endl;
			return 0;
		} else {
			cout << "Failed to sync RTC" << endl;
			return 1;
		}
	}

	if (do_info) {
		Jfy::InverterIDData data;
		conn.readInverterIDInfo(&data);
		cout << "Phases: "<< data.phases << endl
		<< "VA Rating: " << data.VArating << endl
		<< "Firmware: " << data.firmwareVer << endl
		<< "Model: " << data.modelName << endl
		<< "Manufacturer: " << data.manufacturer << endl
		<< "Serial Number: " << data.serialNumber << endl
		<< "Nominal PV Volts: " << data.nominalVpv << endl;
		return 0;
	}
	Jfy::InverterData data;
	conn.readNormalInfo( &data );

	//quality check...
	//energy 0-4kW and today 0-50kWh - otherwise we're nuts
	if (
			((data.mode==1) || (data.mode==0)) &&
			((data.energyCurrent>=0) && (data.energyCurrent<=4000)) &&
			((data.energyToday>=0) && (data.energyToday<50))
	   ) {
 		status="OK";
	} else {
		status="NOK";
		cout << "0 " << data.voltageAc << " " << data.currentAc << " " << data.energyCurrent << " 0 0 0 0 0 0 " << data.frequency << " " << eff << " " << data.temperature << " 0 " << data.energyToday << " " << data.voltageDc << " " << data.currentDc << " " << dcwatts << " 0 0 0 x x " << data.energyTotal << " >>>S123:" <<status << endl;
		//output some diagnostics....
		return 1;
	}
	
	// Write to lo
//cout << now << "\n" <<   data.temperature << " C ," << data.energyCurrent << "," << data.energyToday << "," << data.currentDc << "," << data.voltageDc << "," << data.voltageAc << "," << data.frequency << "," << data.currentAc << "\n";
//emulate SBFSpot output
//DTE GridVA GridAA GridWA GridVB GridAB GridWB GridVC GridAC GridWC GridHz Efficiency? InverterTemp BoostTemp DaykWh PanelVa PanelAa PanelWa PanelVb PanelAb PanelWb
//efficiency is probably output/input*100 in watts
//need watts anyway...


//data.energyToday is problematic
//for the first little while in each day it reports the previous day's total
//only when there's enough generation to increment beyond zero does it move to today's value
//this means on any invocation:

//if there's a last reading file and it was modified before midnight - if the current reading == last reading - output zero (NO UPDATE)
//	else (last reading file before midnight but current reading is different) output reading and update last reading file

//if there's no last reading file. store this value in a file with yesterday's date and output zero (NO UPDATE)

//if there's a last reading file but it's since midnight - all shoudl be good

	// Get the current day std::localtime(&now)->tm_mday


//try to read from file

string	readline;
int	prevtime;
float	prevetot;
float	energyToday=0;

	ifstream preve ("/dev/shm/jfyspot.data");
	if (preve.is_open()) {
		//there is a file
		getline (preve,readline);
		prevtime=stoi(readline);
		getline (preve,readline);
		prevetot=stof(readline);
		getline (preve,readline);
		energyToday=stof(readline);
		preve.close();
		if (verbose) {
		cerr << "file has energyToday " << energyToday << endl;
		}
//		cerr << "File is " << prevtime  << " current is " << std::localtime(&now)->tm_mday << endl;
		if (prevtime != std::localtime(&now)->tm_mday) {
			//file is from not today (probably yesterday)
			energyToday=0;
//			cerr << "last reading is " << prevetot << " current is " << data.energyToday << endl;
//			cerr << "new day, energyToday blanked" << endl;
		} else {
			//file from today
			if (prevetot>data.energyToday) {
				if (verbose) {
				cerr << "inverter just reset daily total" << endl;
				}
				//use the actual total as energy today (because we probably went to >0)
				//mmm, but we were on 0.04 when it reset... let's add the new value
				//energyToday+=data.energyToday;
				//that produces a little bump on every inverter reset which makes me presume that actually
                                //on reset it probably hops to the correct value for the day so really we just set to the day value
                                energyToday=data.energyToday;
                                //if this is correct then after this happens we should always just use the provided figure
                                //that would require some note that the daily reset has occurred though, probably easier
                                //to stick with tracking changes
			} else {
				//normally we just add when there's a difference
				energyToday+=data.energyToday-prevetot;
			}
		}

  	} else {
		//there is no previous file
		//this is the first time running today so just start with energy=0
		energyToday=0;
		cerr << "No previous file";
	}
//now update the file...
	ofstream laste;
	laste.open("/dev/shm/jfyspot.data", ios::trunc);
	if (laste.is_open()) {
		laste << std::localtime(&now)->tm_mday << endl << data.energyToday << endl << energyToday;
		laste.close();
	}
	if (verbose) {
	cerr << "inverter says " <<data.energyToday<< " previous total was " << prevetot << "energy today now "<<energyToday<<endl;
	}



//energyToday is problematic, it seems to progress for a few points before it decides it's a new day an resets
//123solar only looks at 0.1kwh anyway so we're now going to feed it with energyTotal which just continually advances
//not so, it does the difference more often but rounds to only show on graphs every 0.1 - trying a patch to not advance on negatives...
//options are 123solar patch - convince maintainer to include or include as option on inverter
//or make a daemonised inverter comms package
//note -info and -sync don't get done for about an hour
//either multi entry exe which launches daemon if it's not running or
//use daemon_start.php to kick of daemon and probalby just tweak 123solar jfy protocol to read from shared memory...
//at least daemon could establish comms and keep it running rather than always connect and disconnect...
//or just actually maintain our own etotal in that file we're keepoing, reset it ourselves and track changes in etotal...
	dcwatts=data.voltageDc*data.currentDc;
	if (dcwatts>0) {
		eff=data.energyCurrent/dcwatts*100;
	} else {
		eff=0;
	}



	cout << "0 " << data.voltageAc << " " << data.currentAc << " " << data.energyCurrent << " 0 0 0 0 0 0 " << data.frequency << " " << eff << " " << data.temperature << " 0 " << energyToday << " " << data.voltageDc << " " << data.currentDc << " " << dcwatts << " 0 0 0 x " << data.energyToday << " " << data.energyTotal << " >>>S123:" << status << endl;

/*	ofstream logfile;
	logfile.open("/dev/shm/solar.log", ios::app);
	if (logfile.is_open()) {
		logfile << "0 " << data.voltageAc << " " << data.currentAc << " " << data.energyCurrent << " 0 0 0 0 0 0 " << data.frequency << " " << eff << " " << data.temperature << " 0 " << data.energyToday << " " << data.voltageDc << " " << data.currentDc << " " << dcwatts << " 0 0 0 x " << data.energyTotal << " " << originaletot<< " >>>S123:" <<status << endl;
		logfile.close();
	}
*/
	
	return 0;
}

