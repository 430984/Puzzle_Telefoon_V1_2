#include "MCP23017.h"
#include <Arduino.h>
#include <Wire.h>

// Constructors
MCP23017::MCP23017()
{
	this->bChipAddress = 0;
}

// Private functions
uint8_t MCP23017::readRegister(uint8_t bRegisterAddress)
{
	// Read contents of a single register
	uint8_t bRegisterValue;
	
	Wire.beginTransmission(MCP23017_ADDRESS | bChipAddress);
	Wire.write(bRegisterAddress);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS | bChipAddress, 1);
	bRegisterValue = Wire.read();
	//Wire.endTransmission();
	
	return bRegisterValue;
}
// Writes the contents of a single register in the IO expander
void MCP23017::writeRegister(uint8_t bRegisterAddress, uint8_t bValue)
{
	// Write a single register
	Wire.beginTransmission(MCP23017_ADDRESS | bChipAddress);
	Wire.write(bRegisterAddress);
	Wire.write(bValue);
	Wire.endTransmission();
}

void MCP23017::writeBitInRegister(uint8_t bRegisterAddress, uint8_t bBitNumber, uint8_t bValue)
{
	// Read contents of register
	uint8_t bRegisterValue = readRegister(bRegisterAddress);
	
	// Clear or set bit
	if(bValue)
		bRegisterValue |= (1 << bBitNumber);
	else
		bRegisterValue &= ~(1 << bBitNumber);
	
	// Write the new contents back to register
	writeRegister(bRegisterAddress, bRegisterValue);
}
uint8_t MCP23017::readBitInRegister(uint8_t bRegisterAddress, uint8_t bBitNumber)
{
	// Read contents of register
	return (readRegister(bRegisterAddress) & (1 << bBitNumber)) ? 1 : 0;
}

uint8_t MCP23017::getBankForPin(uint8_t bPin)
{
	// Get the bank for the selected pin 
	// (0..7 = bank A, 8..15 = bank B)
	uint8_t bBank;
	
	if(bPin >= 8) 
		bBank = BANK_A;
	else
		bBank = BANK_B;
	return bBank;
}

uint8_t MCP23017::getBitForPin(uint8_t bPin)
{
	// Return remainder after dividing
	return bPin % 8;
}

// Public functions
void MCP23017::begin()
{
	Wire.begin();
	
	setPinMode(0xFFFF);
	setPullUp(0x0000);
}
void MCP23017::begin(uint8_t bAddress)
{
	// Als lokale adres hoger is dan 7, gebruik 7
	// Anders, gebruik lokaal adres als globaal adres
	this->bChipAddress = (bAddress > 7 ? 7 : bAddress);
	begin();
}

// Set pin mode for a single pin (INPUT/OUTPUT/INPUT_PULLUP)
void MCP23017::pinMode(uint8_t bPin, uint8_t bMode)
{
	switch(bMode)
	{
		case OUTPUT:
			writeBitInRegister(IODIRA + getBankForPin(bPin), getBitForPin(bPin), 0);
			break;
		case INPUT_PULLUP:
			writeBitInRegister(IODIRA + getBankForPin(bPin), getBitForPin(bPin), 1);
			writeBitInRegister(GPPUA + getBankForPin(bPin), getBitForPin(bPin), 1);
			break;
		case INPUT:
		default:
			writeBitInRegister(IODIRA + getBankForPin(bPin), getBitForPin(bPin), 1);
			writeBitInRegister(GPPUA + getBankForPin(bPin), getBitForPin(bPin), 0);
			break;
	}
}
// Set pinMode for a complete IO bank
void MCP23017::setPinMode(uint8_t bBank, uint8_t bMode)
{
	writeRegister(IODIRA + (bBank > 0 ? 1 : 0), bMode ^ 0xFF);
}
void MCP23017::setPinMode(uint16_t wMode)
{
	writeRegister(IODIRB, uint8_t(wMode >> 8));
	writeRegister(IODIRA, uint8_t(wMode & 0xFF));	
}

// Enable pull-up resistors for individual pins
void MCP23017::pullUp(uint8_t bPin)
{
	writeBitInRegister(GPPUA + getBankForPin(bPin), getBitForPin(bPin), 1);
}
// Disable pull-up resistors for individual pins
void MCP23017::noPullUp(uint8_t bPin)
{
	writeBitInRegister(GPPUA + getBankForPin(bPin), getBitForPin(bPin), 0);
}
// Set pull-up resistor states for multiple pins
void MCP23017::setPullUp(uint8_t bBank, uint8_t bState)
{
	writeRegister(GPPUA + (bBank > 0 ? 1 : 0), bState);
}
void MCP23017::setPullUp(uint16_t wState)
{
	writeRegister(GPPUB, uint8_t(wState >> 8));
	writeRegister(GPPUA, uint8_t(wState & 0xFF));
}
// Read individual pins
uint8_t MCP23017::digitalRead(uint8_t bPin)
{
	return readBitInRegister(GPIOA + getBankForPin(bPin), getBitForPin(bPin));
}
// Read multiple pins
uint8_t MCP23017::getPins(uint8_t bBank)
{
	return readRegister(GPIOA + (bBank > 0 ? 1 : 0));
}

uint16_t MCP23017::getPins()
{
	uint16_t wRegAValue, wRegBValue;
	
	wRegAValue = uint16_t(readRegister(GPIOA));
	wRegBValue = uint16_t(readRegister(GPIOB));
	
	return (wRegBValue << 8) | wRegAValue;
}
// Write individual pins
void MCP23017::digitalWrite(uint8_t bPin, uint8_t bState)
{
	writeBitInRegister(OLATA + getBankForPin(bPin), getBitForPin(bPin), bState);
}
// Write multiple pins
void MCP23017::setPins(uint8_t bBank, uint8_t bState)
{
	writeRegister(OLATA + (bBank > 0 ? 1 : 0), bState);
}
void MCP23017::setPins(uint16_t wState)
{
	writeRegister(OLATB, uint8_t(wState >> 8));
	writeRegister(OLATA, uint8_t(wState & 0xFF));
}