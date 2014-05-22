/*
 * This file is part of the Doodle3D project (http://doodle3d.com).
 *
 * Copyright (c) 2013, Doodle3D
 * This software is licensed under the terms of the GNU GPL v2 or later.
 * See file LICENSE.txt or visit http://www.gnu.org/licenses/gpl.html for full license details.
 */

#include <stdlib.h>
#include <string.h>
#include "MarlinDriver.h"

using std::string;

//NOTE: see Server.cpp for comments on this macro
#define LOG(lvl, fmt, ...) log_.log(lvl, "MLND", fmt, ##__VA_ARGS__)

const int MarlinDriver::UPDATE_INTERVAL = 200;

MarlinDriver::MarlinDriver(Server& server, const std::string& serialPortPath, const uint32_t& baudrate)
: AbstractDriver(server, serialPortPath, baudrate),
  checkTemperatureInterval_(2000),
  checkConnection_(true),
  checkTemperatureAttempt_(0),
  maxCheckTemperatureAttempts_(2) {

	/*//temperature parsing tests
	readCode(*new string("T:19.1 /0.0 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.1 /0.0 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.5 /0.0 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.6 /0.0 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /0.0 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19 /0.0 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /180.5 B:0.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /180.5 B:90.0 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /180.5 B:90.6 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /180.5 B:90 /0.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /180.5 B:90.0 /180.0 @:0 B@:0"));
	readCode(*new string("ok T:19.9 /180.5 B:90.0 /180.9 @:0 B@:0"));
	readCode(*new string("ok T:19.1 @:0"));
	readCode(*new string("T:19.1 @:0"));
	readCode(*new string("ok T:19.1 /0.0 @:0 B:29.0 /0.0 "));
	readCode(*new string("ok T:19.1 @:0 B:29.0 "));*/

	/*// updateGCodeInfo parsing tests
	startPrint("M109 S220");
	startPrint("M109 S180");
	startPrint("M109 S70");
	startPrint("M109 S60\nM90\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	startPrint("M90\nM91\nM109 S60\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	startPrint("M90\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000\nM109 S60");
	startPrint("M90\nM109 S60\nM91\nM92\nM109 S60\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");*/

	/*//filterGCode parsing tests
	LOG(Logger::BULK, "A");
	setGCode("M90\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	LOG(Logger::BULK, "B");
	setGCode(";comment bla bla\nM90\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	LOG(Logger::BULK, "C");
	setGCode("M90\n;comment bla bla\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	LOG(Logger::BULK, "D");
	setGCode("M90\n;comment bla bla\n;comment2 bla bla\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	LOG(Logger::BULK, "E");
	setGCode("M90\n;comment bla bla\r;comment2 bla bla\rM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000");
	LOG(Logger::BULK, "F");
	setGCode("M90\n;comment bla bla\r;comment2 bla bla\rM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000\n;comment bla bla");*/
}

int MarlinDriver::update() {
	if (!isConnected()) return -1;

	if(checkTemperatureInterval_ != -1 && temperatureTimer_.getElapsedTimeInMilliSec() > checkTemperatureInterval_) {
		//LOG(Logger::VERBOSE, "update temperature()");
		temperatureTimer_.start(); // restart timer

		// We check the temperature
		// during startup we use this to check for a valid connection, when it's established we stop checking
		if(checkConnection_) {
			if(checkTemperatureAttempt_ < maxCheckTemperatureAttempts_) {
				LOG(Logger::VERBOSE, "(checking connection) check temperature %i/%i", checkTemperatureAttempt_, maxCheckTemperatureAttempts_);
				checkTemperature();
				checkTemperatureAttempt_++;
			} else {
				switchBaudrate();
				checkTemperatureAttempt_ = 0;
			}
		} else {
			//LOG(Logger::VERBOSE, "  check temperature");
			checkTemperature();
		}
	}

	if (state_ == PRINTING || state_ == STOPPING || timer_.getElapsedTimeInMilliSec() > UPDATE_INTERVAL) {
		//LOG(Logger::BULK, "update()");
		int rv = readData();
		if (rv > 0) {
			string* line;
			while((line = serial_.extractLine()) != NULL) {
				readResponseCode(*line);
				delete line;
			}
		}
		timer_.start(); // restart timer
	}

	//request to be called again after the time that's left of the update interval
	return UPDATE_INTERVAL - timer_.getElapsedTimeInMilliSec();
}


/***********************
 * PROTECTED FUNCTIONS *
 ***********************/

bool MarlinDriver::startPrint(STATE state) {
	if (!AbstractDriver::startPrint(state)) return false;
	printNextLine();
	return true;
}

void MarlinDriver::readResponseCode(std::string& code) {
	LOG(Logger::VERBOSE, "readResponseCode(): '%s'",code.c_str());

	bool tempMessage = code.find("ok T:")==0;
	bool heatingMessage = code.find("T:")==0;
	if(tempMessage || heatingMessage) { // temperature or heating

		parseTemperatures(code);
		//checkTemperatureAttempt_ = -1; //set to -1 to disable baud rate switching mechanism
		if (checkConnection_) {
			checkConnection_ = false; // stop checking connection (and switching baud rate)
			setState(IDLE);
		}
		//maxCheckTemperatureAttempts_ = 1;

		// determine checkTempInterval
		//if(heatingMessage) checkTemperatureInterval_ = -1; // if it's heating we don't really need to ask
		if(state_ == PRINTING || state_ == STOPPING) checkTemperatureInterval_ = 5000; // if it's printing we ask it less frequently
		else checkTemperatureInterval_ = 1500; // normal

		//LOG(Logger::VERBOSE, "  checkTemperatureInterval_: '%i'",checkTemperatureInterval_);
	} else if(code.find("ok") == 0) { // confirmation that code is received okay

		//sendCode("M105"); // temp
		if(state_ == PRINTING || state_ == STOPPING) {
			gcodeBuffer_.eraseLine();
			printNextLine();
		}

	} else if(code.find("start") != string::npos) {

		//sendCode("M105"); // temp
		//startPrint("M90\nM91\nM92\nG0 X10.600 Y10.050 Z0.200 F2100.000 E0.000"); // temp

	} else if(code.find("Resend:") != string::npos) { // please resend line

		//currentLine_ = atoi(strstr(code.c_str(), "Resend:") + 7); // correct current line
		gcodeBuffer_.setCurrentLine(getCurrentLine() - 1);
		LOG(Logger::VERBOSE, "  Resend: %i", gcodeBuffer_.getCurrentLine());
		printNextLine();

	}
}

void MarlinDriver::parseTemperatures(string& code) {
	// Examples:
	//   ok T:19.1 /0.0 B:0.0 /0.0 @:0 B@:0
	//   T:19.51 B:-1.00 @:0
	//   T:19.5 E:0 W:?

	//LOG(Logger::VERBOSE, "parseTemperatures(): '%s'",code.c_str());
	// temperature hotend
	std::size_t posT = code.find("T:");
	temperature_ = AbstractDriver::findNumber(code,posT+2);
	//LOG(Logger::VERBOSE, "  temperature '%i'", temperature_);

	// target temperature hotend
	std::size_t posTT = code.find('/',posT);
	if(posTT != std::string::npos) {
		targetTemperature_ = AbstractDriver::findNumber(code,posTT+1);
		//LOG(Logger::VERBOSE, "  targetTemperature '%i'", targetTemperature_);
	}

	// bed temperature
	std::size_t posB = code.find("B:");
	if(posB != std::string::npos) {
		bedTemperature_ = AbstractDriver::findNumber(code,posB+2);
		//LOG(Logger::VERBOSE, "  bedTemperature '%i'", bedTemperature_);

		// target bed temperature
		std::size_t posTBT = code.find('/',posB);
		if(posTBT != std::string::npos) {
			targetBedTemperature_ = AbstractDriver::findNumber(code,posTBT+1);
			//LOG(Logger::VERBOSE, "  targetBedTemperature '%i'",targetBedTemperature_);
		}
	}
}

void MarlinDriver::checkTemperature() {
	sendCode("M105");
}

void MarlinDriver::sendCode(const std::string& code) {
	LOG(Logger::VERBOSE, "sendCode(): %s", code.c_str());
	if (isConnected()) {
		AbstractDriver::extractGCodeInfo(code);
		serial_.send((code+"\n").c_str());
	}
}



//STATIC
const AbstractDriver::DriverInfo& MarlinDriver::getDriverInfo() {
	static AbstractDriver::vec_FirmwareDescription supportedFirmware;
	static AbstractDriver::DriverInfo info;

	if (supportedFirmware.empty()) {
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("rigidbot") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("ultimaker") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("ultimaker2") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("printrbot") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("bukobot") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("cartesio") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("cyrus") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("delta_rostockmax") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("deltamaker") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("eventorbot") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("felix") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("gigabot") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("kossel") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("leapfrog_creatr") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("lulzbot_aO_101") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("makergear_m2") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("makergear_prusa") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("makibox") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("orca_0_3") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("ord_bot_hadron") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("printxel_3d") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("prusa_i3") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("prusa_iteration_2") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("rapman") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("reprappro_huxley") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("reprappro_mendel") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("robo_3d_printer") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("shapercube") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("tantillus") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("minifactory") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("vision_3d_printer") );
		supportedFirmware.push_back( AbstractDriver::FirmwareDescription("marlin_generic") );

		info.supportedFirmware = supportedFirmware;
		info.creator = &MarlinDriver::create;
	};

	return info;
}

AbstractDriver* MarlinDriver::create(Server& server, const std::string& serialPortPath, const uint32_t& baudrate) {
	return new MarlinDriver(server, serialPortPath, baudrate);
}
