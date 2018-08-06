// RH_Repeater_Datagram.cpp
//
// Copyright (C) 2011 Mike McCauley
// $Id: RH_Repeater_Datagram.cpp,v 1.6 2014/05/23 02:20:17 mikem Exp $

#include <RH_Repeater_Datagram.h>

RH_Repeater_Datagram::RH_Repeater_Datagram(RHGenericDriver& driver, uint8_t thisAddress) 
    :
    _driver(driver),
    _thisAddress(thisAddress)
{
}

////////////////////////////////////////////////////////////////////
// Public methods
bool RH_Repeater_Datagram::init()
{
    bool ret = _driver.init();
    _driver.setPromiscuous(true); // Allows for receiving packages with any TO field
    if (ret)
	setThisAddress(_thisAddress);
    return ret;
}

void RH_Repeater_Datagram::setThisAddress(uint8_t thisAddress)
{
    _driver.setThisAddress(thisAddress);
    // Use this address in the transmitted FROM header
    setHeaderFrom(thisAddress);
    _thisAddress = thisAddress;
}

bool RH_Repeater_Datagram::sendto(uint8_t* buf, uint8_t len, uint8_t address)
{
    setHeaderTo(address);
    return _driver.send(buf, len);
}

bool RH_Repeater_Datagram::recvfrom(uint8_t* buf, uint8_t* len, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags)
{
    if (_driver.recv(buf, len))
    {
	if (from)  *from =  headerFrom();
	if (to)    *to =    headerTo(); // Don't care who the message is for
	if (id)    *id =    headerId();
	if (flags) *flags = headerFlags();
	return true;
    }
    return false;
}

bool RH_Repeater_Datagram::available()
{
    return _driver.available();
}

void RH_Repeater_Datagram::waitAvailable()
{
    _driver.waitAvailable();
}

bool RH_Repeater_Datagram::waitPacketSent()
{
    return _driver.waitPacketSent();
}

bool RH_Repeater_Datagram::waitPacketSent(uint16_t timeout)
{
    return _driver.waitPacketSent(timeout);
}

bool RH_Repeater_Datagram::waitAvailableTimeout(uint16_t timeout)
{
    return _driver.waitAvailableTimeout(timeout);
}

uint8_t RH_Repeater_Datagram::thisAddress()
{
    return _thisAddress;
}

void RH_Repeater_Datagram::setHeaderTo(uint8_t to)
{
    _driver.setHeaderTo(to);
}

void RH_Repeater_Datagram::setHeaderFrom(uint8_t from)
{
    _driver.setHeaderFrom(from);
}

void RH_Repeater_Datagram::setHeaderId(uint8_t id)
{
    _driver.setHeaderId(id);
}

void RH_Repeater_Datagram::setHeaderFlags(uint8_t set, uint8_t clear)
{
    _driver.setHeaderFlags(set, clear);
}

uint8_t RH_Repeater_Datagram::headerTo()
{
    return _driver.headerTo();
}

uint8_t RH_Repeater_Datagram::headerFrom()
{
    return _driver.headerFrom();
}

uint8_t RH_Repeater_Datagram::headerId()
{
    return _driver.headerId();
}

uint8_t RH_Repeater_Datagram::headerFlags()
{
    return _driver.headerFlags();
}



