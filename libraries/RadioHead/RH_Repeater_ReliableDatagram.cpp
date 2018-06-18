// RH_Repeater_ReliableDatagram.cpp
//
// Define addressed datagram
// 
// Part of the Arduino RH library for operating with HopeRF RH compatible transceivers 
// (see http://www.hoperf.com)
// RH_Repeater_Datagram will be received only by the addressed node or all nodes within range if the 
// to address is RH_BROADCAST_ADDRESS
//
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// $Id: RH_Repeater_ReliableDatagram.cpp,v 1.17 2017/03/08 09:30:47 mikem Exp $

// #include <RH_Repeater_ReliableDatagram.h> From original
#include <RH_Repeater_ReliableDatagram.h>

////////////////////////////////////////////////////////////////////
// Constructors
RH_Repeater_ReliableDatagram::RH_Repeater_ReliableDatagram(RHGenericDriver& driver, uint8_t thisAddress) 
    : RH_Repeater_Datagram(driver, thisAddress)
{
    _retransmissions = 0;
    _lastSequenceNumber = 0;
    _timeout = RH_DEFAULT_TIMEOUT;
    _retries = RH_DEFAULT_RETRIES;
    memset(_seenIds, 0, sizeof(_seenIds));
}

////////////////////////////////////////////////////////////////////
// Public methods
void RH_Repeater_ReliableDatagram::setTimeout(uint16_t timeout)
{
    _timeout = timeout;
}

////////////////////////////////////////////////////////////////////
void RH_Repeater_ReliableDatagram::setRetries(uint8_t retries)
{
    _retries = retries;
}

////////////////////////////////////////////////////////////////////
uint8_t RH_Repeater_ReliableDatagram::retries()
{
    return _retries;
}

////////////////////////////////////////////////////////////////////
bool RH_Repeater_ReliableDatagram::sendtoWaitRepeater(uint8_t* buf, uint8_t len, uint8_t address, uint8_t mySenderAddress)
{
    // Pretend to be the originator of the message
    setThisAddress(mySenderAddress);
    setHeaderFrom(mySenderAddress);

    // Assemble the message
    uint8_t thisSequenceNumber = ++_lastSequenceNumber;
    uint8_t retries = 0;
    while (retries++ <= _retries)
    {
	setHeaderId(thisSequenceNumber);
	setHeaderFlags(RH_FLAGS_NONE, RH_FLAGS_ACK); // Clear the ACK flag
	sendto(buf, len, address);
	waitPacketSent();

	// Never wait for ACKS to broadcasts:
	if (address == RH_BROADCAST_ADDRESS)
	    return true;

	if (retries > 1)
	    _retransmissions++;
	unsigned long thisSendTime = millis(); // Timeout does not include original transmit time

	// Compute a new timeout, random between _timeout and _timeout*2
	// This is to prevent collisions on every retransmit
	// if 2 nodes try to transmit at the same time
#if (RH_PLATFORM == RH_PLATFORM_RASPI) // use standard library random(), bugs in random(min, max)
	uint16_t timeout = _timeout + (_timeout * (random() & 0xFF) / 256);
#else
	uint16_t timeout = _timeout + (_timeout * random(0, 256) / 256);
#endif
	int32_t timeLeft;
        while ((timeLeft = timeout - (millis() - thisSendTime)) > 0)
	{
	    if (waitAvailableTimeout(timeLeft))
	    {
		uint8_t from, to, id, flags;
		if (recvfrom(0, 0, &from, &to, &id, &flags)) // Discards the message
		{
		    // Now have a message: is it our ACK?
		    if (   from == address 
			   && to == _thisAddress 
			   && (flags & RH_FLAGS_ACK) 
			   && (id == thisSequenceNumber))
		    {
			// Its the ACK we are waiting for
			return true;
		    }
		    else if (   !(flags & RH_FLAGS_ACK)
				&& (id == _seenIds[from]))
		    {
			// This is a request we have already received. ACK it again
			acknowledge(id, from, to); // added to
		    }
		    // Else discard it
		}
	    }
	    // Not the one we are waiting for, maybe keep waiting until timeout exhausted
	    YIELD;
	}
	// Timeout exhausted, maybe retry
	YIELD;
    }
    // Retries exhausted
    return false;
}

////////////////////////////////////////////////////////////////////
bool RH_Repeater_ReliableDatagram::recvfromAck(uint8_t* buf, uint8_t* len, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags)
{  
    uint8_t _from;
    uint8_t _to;
    uint8_t _id;
    uint8_t _flags;
    // Get the message before its clobbered by the ACK (shared rx and tx buffer in some drivers
    if (available() && recvfrom(buf, len, &_from, &_to, &_id, &_flags))
    {
	// Never ACK an ACK
//	if (!(_flags & RH_FLAGS_ACK))
//	{
	    // Its a normal message not an ACK
	    //if (_to ==_thisAddress) 
        if(_to != RH_BROADCAST_ADDRESS)
	    {
		// Its not a broadcast, so ACK it
		// Acknowledge message with ACK set in flags and ID set to received ID
		acknowledge(_id, _from, _to); // Was from
	    }
	    // If we have not seen this message before, then we are interested in it
	    if (_id != _seenIds[_from])
	    {
		if (from)  *from =  _from;
		if (to)    *to =    _to; // We don't care who the message is for
		if (id)    *id =    _id;
		if (flags) *flags = _flags;
		_seenIds[_from] = _id;
		return true;
	    }
	    // Else just re-ack it and wait for a new one
//	}
    }
    // No message for us available
    return false;
}

bool RH_Repeater_ReliableDatagram::recvfromAckTimeout(uint8_t* buf, uint8_t* len, uint16_t timeout, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags)
{
    unsigned long starttime = millis();
    int32_t timeLeft;
    while ((timeLeft = timeout - (millis() - starttime)) > 0)
    {
	if (waitAvailableTimeout(timeLeft))
	{
	    if (recvfromAck(buf, len, from, to, id, flags))
		return true;
	}
	YIELD;
    }
    return false;
}

uint32_t RH_Repeater_ReliableDatagram::retransmissions()
{
    return _retransmissions;
}

void RH_Repeater_ReliableDatagram::resetRetransmissions()
{
    _retransmissions = 0;
}
 
void RH_Repeater_ReliableDatagram::acknowledge(uint8_t id, uint8_t from, uint8_t to)
{
    setHeaderId(id);
    setHeaderFlags(RH_FLAGS_ACK);
    setHeaderFrom(to); // Changes our address 

    // We would prefer to send a zero length ACK,
    // but if an RH_RF22 receives a 0 length message with a CRC error, it will never receive
    // a 0 length message again, until its reset, which makes everything hang :-(
    // So we send an ACK of 1 octet
    // REVISIT: should we send the RSSI for the information of the sender?
    uint8_t ack = '!';
    sendto(&ack, sizeof(ack), from); 
    waitPacketSent();
}

