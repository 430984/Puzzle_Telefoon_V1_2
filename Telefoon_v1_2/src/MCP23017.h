/*	MCP23017.h
 *	This is a library for the MCP23017 IO Expander.
 *
 *	By:			Jordy van Dillen
 *	Date:		14-12-2019
 *	Version:	V1.1
 
Release notes:

V1.0 - 14-12-2019
	Initial release

V1.1 - 14-12-2019
	When calling begin(), the IO directions will now be
	initialised as inputs without the pullup resistor
	enabled, in case the IO expander is being initialised
	multiple times while powered on (i.e. new sketch 
	uploaded to Arduino).
 */



#ifndef MCP23017_H
#define MCP23017_H
#include <Arduino.h>

#define MCP23017_ADDRESS 0x20

class MCP23017
{
private:

	// IOCON.BANK needs to be zero!
	enum registers{
		IODIRA = 0,		// IO direction register
		IODIRB = 1,
		IPOLA = 2,		// Input Polarity register
		IPOLB = 3,
		GPINTENA = 4,	// Interrupt-on-change register
		GPINTENB = 5,
		DEFVALA = 6,	// Default value register
		DEFVALB = 7,
		INTCONA = 8,	// Interrupt-on-change control register
		INTCONB = 9,
		IOCON = 10,		// IO expander configuration register
		//IOCON = 11,	// IOCON heeft 2 adressen maar deze wijzen naar hetzelfde adres in het geheugen.
		GPPUA = 12,		// GPIO pull-up resistor register
		GPPUB = 13,	
		INTFA = 14,		// Interrupt flag register
		INTFB = 15,
		INTCAPA = 16,	// Interrupt captured value for port register 
		INTCAPB = 17,
		GPIOA = 18,		// General purpose IO register
		GPIOB = 19,
		OLATA = 20,		// Output latch register
		OLATB = 21,
	};

	uint8_t bChipAddress;
	uint8_t readRegister(uint8_t bRegisterAddress);
	void writeRegister(uint8_t bRegisterAddress, uint8_t bValue);
	void writeBitInRegister(uint8_t bRegisterAddress, uint8_t bBitNumber, uint8_t bValue);
	uint8_t readBitInRegister(uint8_t bRegisterAddress, uint8_t bBitNumber);
	
	uint8_t getBankForPin(uint8_t bPin);
	uint8_t getBitForPin(uint8_t bPin);
	
public:
	
	enum banks{
		BANK_A = 0,
		BANK_B = 1,
	};
	// Constructors
	MCP23017();
	
	void begin();
	void begin(uint8_t bRegisterAddress);
	
	// Set pinMode for individual pins
	void pinMode(uint8_t bPin, uint8_t bMode);
	// Set pinMode for a complete IO bank (0 = input, 1 = output)
	void setPinMode(uint8_t bBank, uint8_t bMode);
	void setPinMode(uint16_t wMode);
	
	// Enable pull-up resistors for individual pins
	void pullUp(uint8_t bPin);
	// Disable pull-up resistors for individual pins
	void noPullUp(uint8_t bPin);
	// Set pull-up resistor states for multiple pins
	void setPullUp(uint8_t bBank, uint8_t bState);
	void setPullUp(uint16_t wState);
	
	// Read individual pins
	uint8_t digitalRead(uint8_t bPin);
	// Read multiple pins
	uint8_t getPins(uint8_t bBank);
	uint16_t getPins();
	
	// Write individual pins
	void digitalWrite(uint8_t bPin, uint8_t bState);
	// Write multiple pins
	void setPins(uint8_t bBank, uint8_t bState);
	void setPins(uint16_t wState);
};

#endif