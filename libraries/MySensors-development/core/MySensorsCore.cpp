/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "MySensorsCore.h"

#if defined(__linux__)
	#include <stdlib.h>
	#include <unistd.h>
#endif

ControllerConfig _cc;	// Configuration coming from controller
NodeConfig _nc;			// Essential settings for node to work
MyMessage _msg;			// Buffer for incoming messages
MyMessage _msgTmp;		// Buffer for temporary messages (acks and nonces among others)

bool _nodeRegistered = false;

#if defined(MY_DEBUG)
	char _convBuf[MAX_PAYLOAD*2+1];
#endif

void (*_timeCallback)(unsigned long); // Callback for requested time messages

void _process(void) {
	hwWatchdogReset();

	#if defined (MY_DEFAULT_TX_LED_PIN) || defined(MY_DEFAULT_RX_LED_PIN) || defined(MY_DEFAULT_ERR_LED_PIN)
		ledsProcess();
	#endif

	#if defined(MY_INCLUSION_MODE_FEATURE)
		inclusionProcess();
	#endif

	#if defined(MY_GATEWAY_FEATURE)
		gatewayTransportProcess();
	#endif

	#if defined(MY_SENSOR_NETWORK)
		transportProcess();
	#endif

	#if defined(__linux__)
		// To avoid high cpu usage
		usleep(10000); // 10ms
	#endif
}

void _infiniteLoop(void) {
	while(1) {
		yield();
		#if defined (MY_DEFAULT_TX_LED_PIN) || defined(MY_DEFAULT_RX_LED_PIN) || defined(MY_DEFAULT_ERR_LED_PIN)
			ledsProcess();
		#endif
		#if defined(__linux__)
			exit(1);
		#endif
	}
}

void _begin(void) {
	hwWatchdogReset();

	if (preHwInit) {
		preHwInit();
	}

	hwInit();

	debug(PSTR("MCO:BGN:INIT " MY_NODE_TYPE ",CP=" MY_CAPABILITIES ",VER=" MYSENSORS_LIBRARY_VERSION "\n"));

	// Call before() in sketch (if it exists)
	if (before) {
		debug(PSTR("MCO:BGN:BFR\n"));	// before callback
		before();
	}

	#if defined(MY_DEFAULT_TX_LED_PIN) || defined(MY_DEFAULT_RX_LED_PIN) || defined(MY_DEFAULT_ERR_LED_PIN)
		ledsInit();
	#endif

	signerInit();

	// Read latest received controller configuration from EEPROM
	// Note: _cc.isMetric is bool, hence empty EEPROM (=0xFF) evaluates to true (default)
	hwReadConfigBlock((void*)&_cc, (void*)EEPROM_CONTROLLER_CONFIG_ADDRESS, sizeof(ControllerConfig));

	#if defined(MY_OTA_FIRMWARE_FEATURE)
		// Read firmware config from EEPROM, i.e. type, version, CRC, blocks
		readFirmwareSettings();
	#endif

	#if defined(MY_SENSOR_NETWORK)
		// Save static parent id in eeprom (used by bootloader)
		hwWriteConfig(EEPROM_PARENT_NODE_ID_ADDRESS, MY_PARENT_NODE_ID);
		transportInitialise();
		while (!isTransportReady()) {
			hwWatchdogReset();
			transportProcess();
			yield();
		}
	#endif



	#ifdef MY_NODE_LOCK_FEATURE
		// Check if node has been locked down
		if (hwReadConfig(EEPROM_NODE_LOCK_COUNTER) == 0) {
			// Node is locked, check if unlock pin is asserted, else hang the node
			pinMode(MY_NODE_UNLOCK_PIN, INPUT_PULLUP);
			// Make a short delay so we are sure any large external nets are fully pulled
			unsigned long enter = hwMillis();
			while (hwMillis() - enter < 2) {}
			if (digitalRead(MY_NODE_UNLOCK_PIN) == 0) {
				// Pin is grounded, reset lock counter
				hwWriteConfig(EEPROM_NODE_LOCK_COUNTER, MY_NODE_LOCK_COUNTER_MAX);
				// Disable pullup
				pinMode(MY_NODE_UNLOCK_PIN, INPUT);
				setIndication(INDICATION_ERR_LOCKED);
				debug(PSTR("MCO:BGN:NODE UNLOCKED\n"));
			} else {
				// Disable pullup
				pinMode(MY_NODE_UNLOCK_PIN, INPUT);
				nodeLock("LDB"); //Locked during boot
			}
		} else if (hwReadConfig(EEPROM_NODE_LOCK_COUNTER) == 0xFF) {
			// Reset walue
			hwWriteConfig(EEPROM_NODE_LOCK_COUNTER, MY_NODE_LOCK_COUNTER_MAX);
		}
	#endif

	#if defined(MY_GATEWAY_FEATURE)
		#if defined(MY_INCLUSION_BUTTON_FEATURE)
	    	inclusionInit();
		#endif

	    // initialise the transport driver
		if (!gatewayTransportInit()) {
			setIndication(INDICATION_ERR_INIT_GWTRANSPORT);
			debug(PSTR("!MCO:BGN:TSP FAIL\n"));
			// Nothing more we can do
			_infiniteLoop();
		}
	#endif

	#if !defined(MY_GATEWAY_FEATURE)
		presentNode();
	#endif

	// register node
	_registerNode();

	// Call sketch setup
	if (setup) {
		debug(PSTR("MCO:BGN:STP\n"));	// setup callback
		setup();
	}

	debug(PSTR("MCO:BGN:INIT OK,ID=%d,PAR=%d,DIS=%d,REG=%d\n"), _nc.nodeId, _nc.parentNodeId, _nc.distance, _nodeRegistered);

	hwWatchdogReset();
}


void _registerNode(void) {
	#if defined (MY_REGISTRATION_FEATURE) && !defined(MY_GATEWAY_FEATURE)
		debug(PSTR("MCO:REG:REQ\n"));	// registration request
		setIndication(INDICATION_REQ_REGISTRATION);
		_nodeRegistered = MY_REGISTRATION_DEFAULT;
		uint8_t counter = MY_REGISTRATION_RETRIES;
		// only proceed if register response received or retries exceeded
		do {
			(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_REGISTRATION_REQUEST).set(MY_CORE_VERSION));
		} while (!wait(2000, C_INTERNAL, I_REGISTRATION_RESPONSE) && counter--);

	#else
		_nodeRegistered = true;
		debug(PSTR("MCO:REG:NOT NEEDED\n"));
	#endif
}

void presentNode(void) {
	setIndication(INDICATION_PRESENT);
	// Present node and request config
	#if defined(MY_GATEWAY_FEATURE)
		// Send presentation for this gateway device
		#if defined(MY_REPEATER_FEATURE)
			(void)present(NODE_SENSOR_ID, S_ARDUINO_REPEATER_NODE);
		#else
			(void)present(NODE_SENSOR_ID, S_ARDUINO_NODE);
		#endif
	#else

		#if defined(MY_OTA_FIRMWARE_FEATURE)
			presentBootloaderInformation();
		#endif

		// Send signing preferences for this node to the GW
		signerPresentation(_msgTmp, GATEWAY_ADDRESS);

			// Send presentation for this radio node
		#if defined(MY_REPEATER_FEATURE)
			(void)present(NODE_SENSOR_ID, S_ARDUINO_REPEATER_NODE);
		#else
			(void)present(NODE_SENSOR_ID, S_ARDUINO_NODE);
		#endif

		// Send a configuration exchange request to controller
		// Node sends parent node. Controller answers with latest node configuration
		(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_CONFIG).set(_nc.parentNodeId));

		// Wait configuration reply.
		(void)wait(2000, C_INTERNAL, I_CONFIG);

	#endif

	if (presentation) {
		presentation();
	}
}


uint8_t getNodeId(void) {
	return _nc.nodeId;
}

uint8_t getParentNodeId(void) {
	return _nc.parentNodeId;
}

ControllerConfig getConfig(void) {
	return _cc;
}


bool _sendRoute(MyMessage &message) {
	#if defined(MY_CORE_ONLY)
		(void)message;
	#endif
	#if defined(MY_GATEWAY_FEATURE)
		if (message.destination == _nc.nodeId) {
			// This is a message sent from a sensor attached on the gateway node.
			// Pass it directly to the gateway transport layer.
			return gatewayTransportSend(message);
		}
	#endif
	#if defined(MY_SENSOR_NETWORK)
		return transportSendRoute(message);
	#else
		return false;
	#endif
}

bool send(MyMessage &message, const bool enableAck) {
	message.sender = _nc.nodeId;
	mSetCommand(message, C_SET);
	mSetRequestAck(message, enableAck);

	#if defined(MY_REGISTRATION_FEATURE) && !defined(MY_GATEWAY_FEATURE)
		if (_nodeRegistered) {
			return _sendRoute(message);
		}
		else {
			debug(PSTR("!MCO:SND:NODE NOT REG\n"));	// node not registered
			return false;
		}
	#else
		return _sendRoute(message);
	#endif
	}

bool sendBatteryLevel(const uint8_t value, const bool ack) {
	return _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_BATTERY_LEVEL, ack).set(value));
}

bool sendHeartbeat(const bool ack) {
	#if defined(MY_SENSOR_NETWORK)
		const uint32_t heartbeat = transportGetHeartbeat();
		return _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_HEARTBEAT_RESPONSE, ack).set(heartbeat));
	#else
		(void)ack;
		return false;
	#endif
}

bool present(const uint8_t childSensorId, const uint8_t sensorType, const char *description, const bool ack) {
	return _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, childSensorId, C_PRESENTATION, sensorType, ack).set(childSensorId==NODE_SENSOR_ID?MYSENSORS_LIBRARY_VERSION:description));
}

bool sendSketchInfo(const char *name, const char *version, const bool ack) {
	bool result = true;
	if (name) {
		result &= _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_SKETCH_NAME, ack).set(name));
	}
  if (version) {
		result &= _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_SKETCH_VERSION, ack).set(version));
	}
	return result;
}

bool request(const uint8_t childSensorId, const uint8_t variableType, const uint8_t destination) {
	return _sendRoute(build(_msgTmp, destination, childSensorId, C_REQ, variableType).set(""));
}

bool requestTime(const bool ack) {
	return _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_TIME, ack).set(""));
}

// Message delivered through _msg
bool _processInternalMessages(void) {
	const uint8_t type = _msg.type;

	if (_msg.sender == GATEWAY_ADDRESS) {
		if (type == I_REBOOT) {
			#if !defined(MY_DISABLE_REMOTE_RESET)
				// Requires MySensors or other bootloader with watchdogs enabled
				setIndication(INDICATION_REBOOT);
				hwReboot();
			#endif
		}
		else if (type == I_REGISTRATION_RESPONSE) {
			#if defined (MY_REGISTRATION_FEATURE) && !defined(MY_GATEWAY_FEATURE)
				_nodeRegistered = _msg.getBool();
				setIndication(INDICATION_GOT_REGISTRATION);
				debug(PSTR("MCO:PIM:NODE REG=%d\n"), _nodeRegistered);	// node registration
			#endif
		}
		else if (type == I_CONFIG) {
			// Pick up configuration from controller (currently only metric/imperial) and store it in eeprom if changed
			_cc.isMetric = _msg.data[0] == 0x00 || _msg.data[0] == 'M'; // metric if null terminated or M
			hwWriteConfig(EEPROM_CONTROLLER_CONFIG_ADDRESS, _cc.isMetric);
		}
		else if (type == I_PRESENTATION) {
			// Re-send node presentation to controller
			presentNode();
		}
		else if (type == I_HEARTBEAT_REQUEST) {
			(void)sendHeartbeat();
		}
		else if (type == I_TIME) {
			// Deliver time to callback
			if (receiveTime) {
				receiveTime(_msg.getULong());
			}
		}
		else if (type == I_CHILDREN) {
			#if defined(MY_REPEATER_FEATURE)
				if (_msg.data[0] == 'C') {
					// Clears child relay data for this node
					setIndication(INDICATION_CLEAR_ROUTING);
					transportClearRoutingTable();
					(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_CHILDREN).set("OK"));
				}
			#endif
		}
		else if (type == I_DEBUG) {
			#if defined(MY_DEBUG) || defined(MY_SPECIAL_DEBUG)
				const char debug_msg = _msg.data[0];
				if (debug_msg == 'R') {		// routing table
				#if defined(MY_REPEATER_FEATURE)
					for (uint16_t cnt = 0; cnt < SIZE_ROUTES; cnt++) {
						const uint8_t route = transportGetRoute(cnt);
						if (route != BROADCAST_ADDRESS) {
							debug(PSTR("MCO:PIM:ROUTE N=%d,R=%d\n"), cnt, route);
							uint8_t outBuf[2] = { (uint8_t)cnt,route };
							(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_DEBUG).set(outBuf, 2));
							wait(200);
						}
					}
				#endif
				}
				else if (debug_msg == 'V') {	// CPU voltage
					(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_DEBUG).set(hwCPUVoltage()));
				}
				else if (debug_msg == 'F') {	// CPU frequency in 1/10Mhz
					(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_DEBUG).set(hwCPUFrequency()));
				}
				else if (debug_msg == 'M') {	// free memory
					(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_DEBUG).set(hwFreeMem()));
				}
				else if (debug_msg == 'E') {	// clear MySensors eeprom area and reboot
					(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID, C_INTERNAL, I_DEBUG).set("OK"));
					for (int i = EEPROM_START; i<EEPROM_LOCAL_CONFIG_ADDRESS; i++) {
						hwWriteConfig(i, 0xFF);
					}
					setIndication(INDICATION_REBOOT);
					hwReboot();
				}
			#endif
		}
		else return false;
	}
	else {
		// sender is a node
		if (type == I_REGISTRATION_REQUEST) {
			#if defined(MY_GATEWAY_FEATURE)
				// registeration requests are exclusively handled by GW/Controller
				#if !defined(MY_REGISTRATION_CONTROLLER)
					// auto registration if version compatible
					bool approveRegistration = true;

					#if defined(MY_CORE_COMPATIBILITY_CHECK)
							approveRegistration = (_msg.getByte() >= MY_CORE_MIN_VERSION);
					#endif

					#if (F_CPU>16000000)
						// delay for fast GW and slow nodes
						delay(5);
					#endif
					(void)_sendRoute(build(_msgTmp, _msg.sender, NODE_SENSOR_ID, C_INTERNAL, I_REGISTRATION_RESPONSE).set(approveRegistration));
				#else
					return false;	// processing of this request via controller
				#endif
			#endif
		}
		else return false;
	}
	return true;
}


void saveState(const uint8_t pos, const uint8_t value) {
	hwWriteConfig(EEPROM_LOCAL_CONFIG_ADDRESS+pos, value);
}
uint8_t loadState(const uint8_t pos) {
	return hwReadConfig(EEPROM_LOCAL_CONFIG_ADDRESS+pos);
}


void wait(const uint32_t waitingMS) {
	const uint32_t enteringMS = hwMillis();
	while (hwMillis() - enteringMS < waitingMS) {
		_process();
		yield();
	}
}

bool wait(const uint32_t waitingMS, const uint8_t cmd, const uint8_t msgtype) {
	const uint32_t enteringMS = hwMillis();
	// invalidate msg type
	_msg.type = !msgtype;
	bool expectedResponse = false;
	while ( (hwMillis() - enteringMS < waitingMS) && !expectedResponse ) {
		_process();
		yield();
		expectedResponse = (mGetCommand(_msg) == cmd && _msg.type == msgtype);
	}
	return expectedResponse;
}

int8_t _sleep(const uint32_t sleepingMS, const bool smartSleep, const uint8_t interrupt1, const uint8_t mode1, const uint8_t interrupt2, const uint8_t mode2) {
	debug(PSTR("MCO:SLP:MS=%lu,SMS=%d,I1=%d,M1=%d,I2=%d,M2=%d\n"), sleepingMS, smartSleep, interrupt1, mode1, interrupt2, mode2);
	// OTA FW feature: do not sleep if FW update ongoing
	#if defined(MY_OTA_FIRMWARE_FEATURE)
		if (_fwUpdateOngoing) {
			debug(PSTR("!MCO:SLP:FWUPD\n"));	// sleeping not possible, FW update ongoing
			wait(sleepingMS);
			return MY_SLEEP_NOT_POSSIBLE;
		}
	#endif
	// repeater feature: sleeping not possible
	#if defined(MY_REPEATER_FEATURE)
		(void)smartSleep;
		(void)interrupt1;
		(void)mode1;
		(void)interrupt2;
		(void)mode2;

		debug(PSTR("!MCO:SLP:REP\n"));	// sleeping not possible, repeater feature enabled
		wait(sleepingMS);
		return MY_SLEEP_NOT_POSSIBLE;
	#else
		uint32_t sleepingTimeMS = sleepingMS;
		#if defined(MY_SENSOR_NETWORK)
			// Do not sleep if transport not ready
			if (!isTransportReady()) {
				debug(PSTR("!MCO:SLP:TNR\n"));	// sleeping not possible, transport not ready
				const uint32_t sleepEnterMS = hwMillis();
				uint32_t sleepDeltaMS = 0;
				while (!isTransportReady() && (sleepDeltaMS < sleepingTimeMS) && (sleepDeltaMS < MY_SLEEP_TRANSPORT_RECONNECT_TIMEOUT_MS)) {
					_process();
					yield();
					sleepDeltaMS = hwMillis() - sleepEnterMS;
				}
				// sleep remainder
				if (sleepDeltaMS < sleepingTimeMS) {
					sleepingTimeMS -= sleepDeltaMS;		// calculate remaining sleeping time
					debug(PSTR("MCO:SLP:MS=%lu\n"), sleepingTimeMS);
				}
				else {
					// no sleeping time left
					return MY_SLEEP_NOT_POSSIBLE;
				}
			}
		#endif

		if (smartSleep) {
			// notify controller about going to sleep
			(void)sendHeartbeat();
			wait(MY_SMART_SLEEP_WAIT_DURATION_MS);		// listen for incoming messages
		}

		#if defined(MY_SENSOR_NETWORK)
			debug(PSTR("MCO:SLP:TPD\n"));	// sleep, power down transport
			transportPowerDown();
		#endif

		setIndication(INDICATION_SLEEP);

		int8_t result = MY_SLEEP_NOT_POSSIBLE;	// default

		if (interrupt1 != INTERRUPT_NOT_DEFINED && interrupt2 != INTERRUPT_NOT_DEFINED) {
			// both IRQs
			result = hwSleep(interrupt1, mode1, interrupt2, mode2, sleepingTimeMS);
		}
		else if (interrupt1 != INTERRUPT_NOT_DEFINED && interrupt2 == INTERRUPT_NOT_DEFINED) {
			// one IRQ
			result = hwSleep(interrupt1, mode1, sleepingTimeMS);
		}
		else if (interrupt1 == INTERRUPT_NOT_DEFINED && interrupt2 == INTERRUPT_NOT_DEFINED) {
			// no IRQ
			result = hwSleep(sleepingTimeMS);
		}

		setIndication(INDICATION_WAKEUP);
		debug(PSTR("MCO:SLP:WUP=%d\n"), result);	// sleep wake-up
		return result;
	#endif
}

// sleep functions
int8_t sleep(const uint32_t sleepingMS, const bool smartSleep) {
	return _sleep(sleepingMS, smartSleep);
}

int8_t sleep(const uint8_t interrupt, const uint8_t mode, const uint32_t sleepingMS, const bool smartSleep) {
	return _sleep(sleepingMS, smartSleep, interrupt, mode);
}

int8_t sleep(const uint8_t interrupt1, const uint8_t mode1, const uint8_t interrupt2, const uint8_t mode2, const uint32_t sleepingMS, const bool smartSleep) {
	return _sleep(sleepingMS, smartSleep, interrupt1, mode1, interrupt2, mode2);
}

// deprecated smartSleep() functions
int8_t smartSleep(const uint32_t sleepingMS) {
	// compatibility
	return _sleep(sleepingMS, true);
}

int8_t smartSleep(const uint8_t interrupt, const uint8_t mode, const uint32_t sleepingMS) {
	// compatibility
	return _sleep(sleepingMS, true, interrupt, mode);
}

int8_t smartSleep(const uint8_t interrupt1, const uint8_t mode1, const uint8_t interrupt2, const uint8_t mode2, const uint32_t sleepingMS) {
	// compatibility
	return _sleep(sleepingMS, true, interrupt1, mode1, interrupt2, mode2);
}


#ifdef MY_NODE_LOCK_FEATURE
void nodeLock(const char* str) {
	// Make sure EEPROM is updated to locked status
	hwWriteConfig(EEPROM_NODE_LOCK_COUNTER, 0);
	while (1) {
		setIndication(INDICATION_ERR_LOCKED);
		debug(PSTR("MCO:NLK:NODE LOCKED. TO UNLOCK, GND PIN %d AND RESET\n"), MY_NODE_UNLOCK_PIN);
		yield();
		(void)_sendRoute(build(_msgTmp, GATEWAY_ADDRESS, NODE_SENSOR_ID,C_INTERNAL, I_LOCKED).set(str));
		#if defined(MY_SENSOR_NETWORK)
			transportPowerDown();
			debug(PSTR("MCO:NLK:TPD\n"));	// power down transport
		#endif
		setIndication(INDICATION_SLEEP);
		(void)hwSleep((unsigned long)1000*60*30); // Sleep for 30 min before resending LOCKED message
		setIndication(INDICATION_WAKEUP);
	}
}
#endif
