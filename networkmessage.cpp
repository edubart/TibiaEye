#include "headers.h"
#include "networkmessage.h"
#include "util.h"

NetworkMessage::NetworkMessage()
{
	reset();
}

NetworkMessage::~NetworkMessage()
{

}

void NetworkMessage::reset()
{
	mMsgSize = 0;
	mReadPos = 6;
}

uint8 NetworkMessage::getByte()
{
	if(!canGet(1))
		return 0;
	return mMsgBuf[mReadPos++];
}

uint16 NetworkMessage::getU16()
{
	if(!canGet(2))
		return 0;
	uint16 v = *(uint16*)(mMsgBuf + mReadPos);
	mReadPos += 2;
	return v;
}

uint32 NetworkMessage::getU32()
{
	if(!canGet(4))
		return 0;
	uint32 v = *(uint32*)(mMsgBuf + mReadPos);
	mReadPos += 4;
	return v;
}

uint16 NetworkMessage::getBytes(uint8 *buffer, uint16 size) {
	if(!canGet(size))
		return 0;
	memcpy(buffer, mMsgBuf + mReadPos, size);
	mReadPos += size;
	return size;
}

QString NetworkMessage::getString()
{
	uint16 stringlen = getU16();
	if(stringlen == 0 || !canGet(stringlen))
		return QString();

	char *string = (char*)(mMsgBuf + mReadPos);
	mReadPos += stringlen;
	return QString::fromAscii(string, stringlen);
}

void NetworkMessage::addByte(uint8  value)
{
	if(!canAdd(1))
		return;
	mMsgBuf[mReadPos++] = value;
	mMsgSize++;
}

void NetworkMessage::addU16(uint16 value)
{
	if(!canAdd(2))
		return;
	*(uint16*)(mMsgBuf + mReadPos) = value;
	mReadPos += 2; mMsgSize += 2;
}

void NetworkMessage::addU32(uint32 value)
{
	if(!canAdd(4))
		return;
	*(uint32*)(mMsgBuf + mReadPos) = value;
	mReadPos += 4; mMsgSize += 4;
}

void NetworkMessage::addString(const char* value)
{
	uint16 stringlen = (uint16)strlen(value);
	if(!canAdd(stringlen+2) || stringlen > 8192)
		return;

	addU16(stringlen);
	strcpy((char*)(mMsgBuf + mReadPos), value);
	mReadPos += stringlen;
	mMsgSize += stringlen;
}

void NetworkMessage::addBytes(const uint8* bytes, uint16 size)
{
	if(!canAdd(size))
		return;

	memcpy(mMsgBuf + mReadPos, bytes, size);
	mReadPos += size;
	mMsgSize += size;
}

void NetworkMessage::addPaddingBytes(uint16 n)
{
	if(!canAdd(n))
		return;

	memset((void*)&mMsgBuf[mReadPos], 0x33, n);
	mReadPos += n;
	mMsgSize += n;
}

bool NetworkMessage::checkSum()
{
	uint32 recvChecksum = *(uint32*)(mMsgBuf + 2);
	if(adlerChecksum(getBodyBuffer(), mMsgSize) != recvChecksum)
		return false;
	return true;
}

void NetworkMessage::writeSum()
{
	*(uint32*)(mMsgBuf + 2) = adlerChecksum(getBodyBuffer(), mMsgSize);
}

bool NetworkMessage::canAdd(uint16 size)
{
	if(size + mReadPos < NETWORKMESSAGE_MAXSIZE - 2)
		return true;
	qWarning() << "[NetworkMessage::canAdd] Buffer is full.";
	return false;
}

bool NetworkMessage::canGet(uint16 size)
{
	if(size + mReadPos < NETWORKMESSAGE_MAXSIZE)
		return true;
	qWarning() << "[NetworkMessage::canAdd] Buffer is at EOF.";
	return false;
}
