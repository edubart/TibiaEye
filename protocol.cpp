#include "headers.h"
#include "protocol.h"
#include "rsa.h"
#include "clientproxy.h"
#include "abstractserverproxy.h"

Protocol::Protocol(ClientProxy *client)
	: QObject(client), mClient(client), mServer(NULL), mEncryptionEnabled(false)
{
	qDebug("Protocol::Protocol");

	mKey[0] = 0; mKey[1] = 0; mKey[2] = 0; mKey[3] = 0;
	mUse = 1;
}

Protocol::~Protocol()
{
	qDebug("Protocol::~Protocol");

}

void Protocol::decrenseUse()
{
	qDebug("Protocol::decrenseUse");

	mUse--;
	if(mUse <= 0)
		deleteLater();
}

void Protocol::onServerConnect()
{
	qDebug("Protocol::onServerConnect");

	emit serverConnected();
}

void Protocol::onServerDisconnect()
{
	qDebug("Protocol::onServerDisconnect");

	mServer = NULL;

	emit serverDisconnected();

	disconnectClient();
	decrenseUse();
}

void Protocol::onClientDisconnect()
{
	qDebug("Protocol::onClientDisconnect");

	mClient = NULL;

	emit clientDisconnected();

	disconnectServer();
	decrenseUse();
}

void Protocol::XTEA_encrypt(NetworkMessage& msg)
{
	qDebug("Protocol::XTEA_encrypt");

	uint32 k[4];
	k[0] = mKey[0]; k[1] = mKey[1]; k[2] = mKey[2]; k[3] = mKey[3];

	uint16 messageLength = msg.getMessageLength();
	memmove(msg.getBodyBuffer() + 2, msg.getBodyBuffer(), messageLength);
	*(uint16 *)msg.getBodyBuffer() = messageLength;
	msg.setReadPos(messageLength + 8);

	messageLength += 2;
	msg.setMessageLength(messageLength);

	//add bytes until reach 8 multiple
	uint16 n;
	if((messageLength % 8) != 0){
		n = 8 - (messageLength % 8);
		msg.addPaddingBytes(n);
		messageLength = messageLength + n;
	}

	uint16 read_pos = 0;
	uint32* buffer = (uint32*)msg.getBodyBuffer();
	while(read_pos < messageLength/4){
		uint32 v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
		uint32 delta = 0x61C88647;
		uint32 sum = 0;

		for(int i = 0; i < 32; i++) {
			v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
			sum -= delta;
			v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum>>11 & 3]);
		}
		buffer[read_pos] = v0; buffer[read_pos + 1] = v1;
		read_pos = read_pos + 2;
	}
}

bool Protocol::XTEA_decrypt(NetworkMessage& msg)
{
	qDebug("Protocol::XTEA_decrypt");

	if(msg.getMessageLength() % 8 != 0){
		qWarning() << "[Protocol::XTEA_decrypt]. Not valid encrypted message size";
		return false;
	}

	uint32 k[4];
	k[0] = mKey[0]; k[1] = mKey[1]; k[2] = mKey[2]; k[3] = mKey[3];

	uint32* buffer = (uint32*)(msg.getBodyBuffer());
	uint16 read_pos = 0;
	uint16 messageLength = msg.getMessageLength();
	while(read_pos < messageLength/4){
		uint32 v0 = buffer[read_pos], v1 = buffer[read_pos + 1];
		uint32 delta = 0x61C88647;
		uint32 sum = 0xC6EF3720;

		for(int i = 0; i < 32; i++) {
			v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum>>11 & 3]);
			sum += delta;
			v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
		}
		buffer[read_pos] = v0; buffer[read_pos + 1] = v1;
		read_pos = read_pos + 2;
	}

	uint16 tmp = *(uint16*)(msg.getBodyBuffer());
	if(tmp > msg.getMessageLength() - 2){
		qWarning() << "[Protocol::XTEA_decrypt]. Not valid unencrypted message size " << tmp << " of " << (msg.getMessageLength() - 2);
		return false;
	}

	memmove(msg.getBuffer() + 6, msg.getBuffer() + 8, tmp);
	msg.setMessageLength(tmp);
	return true;
}

bool Protocol::RSA_decrypt(RSA* rsa, NetworkMessage& msg)
{
	qDebug("Protocol::RSA_decrypt");

	if(msg.getMessageLength() + 6 - msg.getReadPos() != 128){
		qWarning() << "[Protocol::RSA_decrypt]. Not valid packet size";
		return false;
	}

	rsa->decrypt((char*)(msg.getBuffer() + msg.getReadPos()), 128);

	if(msg.getByte() != 0){
		qWarning() << "[Protocol::RSA_decrypt]. First byte != 0";
		return false;
	}

	return true;
}

bool Protocol::RSA_encrypt(RSA* rsa, NetworkMessage& msg)
{
	qDebug("Protocol::RSA_encrypt");

	if(msg.getReadPos() < 128) {
		qWarning() << "[Protocol::RSA_encrypt]. Not valid packet size";
		return false;
	}

	rsa->encrypt((char*)(msg.getBuffer() + msg.getReadPos() - 128), 128);
	return true;
}


bool Protocol::onRecvMessage(NetworkMessage& msg, bool decrypt)
{
	qDebug("Protocol::onRecvMessage");

	if(decrypt && mEncryptionEnabled)
		return XTEA_decrypt(msg);
	return true;
}


void Protocol::onRecvFirstClientMessage(NetworkMessage& msg)
{
	qDebug("Protocol::onRecvFirstClientMessage");

	emit firstClientMessage(msg);
}

void Protocol::onRecvClientMessage(NetworkMessage& msg)
{
	qDebug("Protocol::onRecvClientMessage");

	if(mClient) {
		if(onRecvMessage(msg))
			emit clientMessage(msg);
		else
			disconnectClient();
	}
}

void Protocol::onRecvServerMessage(NetworkMessage& msg, bool decrypt)
{
	qDebug("Protocol::onRecvServerMessage");

	if(mServer) {
		if(onRecvMessage(msg, decrypt))
			emit serverMessage(msg);
		else
			disconnectServer();
	}
}

void Protocol::onSendMessage(NetworkMessage& msg)
{
	qDebug("Protocol::onSendMessage");

	if(mEncryptionEnabled){
		XTEA_encrypt(msg);
	}
	msg.writeSum();
	msg.writeMessageLength();
}

void Protocol::sendClientMessage(NetworkMessage &msg)
{
	qDebug("Protocol::sendClientMessage");

	if(mClient) {
		onSendMessage(msg);
		mClient->sendMessage(msg);
	}
}

void Protocol::sendServerMessage(NetworkMessage &msg)
{
	qDebug("Protocol::sendServerMessage");

	if(mServer) {
		onSendMessage(msg);
		mServer->sendMessage(msg);
	}
}

void Protocol::disconnectClient()
{
	qDebug("Protocol::disconnectClient");

	if(mClient) {
		mClient->disconnect();
		mClient = NULL;
	}
}

void Protocol::disconnectServer()
{
	qDebug("Protocol::disconnectServer");

	if(mServer) {
		mServer->disconnect();
		mServer = NULL;
	}
}
