#include "headers.h"
#include "protocolgame.h"
#include "modemanager.h"
#include "servervirtual.h"
#include "rsa.h"

void ProtocolGame::parsePlayFirstClientMessage(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parsePlayFirstClientMessage");

	// skip client information
	msg.getU16();
	msg.getU16();

	// decrypt rsa
	if(!RSA_decrypt(RSA::instance(), msg)){
		disconnectClient();
		return;
	}

	// setup XTEA key
	uint32 key[4];
	key[0] = msg.getU32();
	key[1] = msg.getU32();
	key[2] = msg.getU32();
	key[3] = msg.getU32();
	setXTEAKey(key);

	// connect to a virtual server
	ServerVirtual *server = new ServerVirtual(this);

	connect(this, SIGNAL(serverConnected()), this, SLOT(parsePlayServerConnect()));
	connect(this, SIGNAL(serverDisconnected()), this, SLOT(parsePlayServerDisconnect()));

	connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(parsePlayServerMessage(NetworkMessage&)));
	connect(this, SIGNAL(clientMessage(NetworkMessage&)), this, SLOT(parsePlayClientMessage(NetworkMessage&)));

	connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(sendClientMessage(NetworkMessage&)));

	setServer(server);
	server->start();
}

void ProtocolGame::parsePlayServerConnect()
{
	qDebug("ProtocolGame::parsePlayServerConnect");

	enableXTEAEncryption();
}

void ProtocolGame::parsePlayServerDisconnect()
{
	qDebug("ProtocolGame::parsePlayServerDisconnect");

}

void ProtocolGame::parsePlayServerMessage(NetworkMessage&)
{
	qDebug("ProtocolGame::parsePlayServerMessage");

}

void ProtocolGame::parsePlayClientMessage(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parsePlayClientMessage");

	uint8 opt = msg.getByte();
	switch(opt) {
		case 0x14: // logout
			parsePlayClientLogout(msg);
			break;
		case 0x65: // move north
		case 0x66: // move east
		case 0x67: // move south
		case 0x68: // move west
		case 0x6A: // move northeast
		case 0x6B: // move southeast
		case 0x6C: // move southwest
		case 0x6D: // move northwest
			parsePlayClientWalk(msg);
			break;
		case 0xA1: // attack
		case 0xA2: // follow
			parsePlayClientAttackOrFollow(msg);
			break;
		case 0x96: // say something
			//parsePlayClientSay(msg);
			break;
		case 0x1E: // keep alive / ping response
		case 0x64: // move with steps
		case 0x69: // stop-autowalk
		case 0x6F: // turn north
		case 0x70: // turn east
		case 0x71: // turn south
		case 0x72: // turn west
		case 0x78: // throw item
		case 0x79: // description in shop window
		case 0x7A: // player bought from shop
		case 0x7B: // player sold to shop
		case 0x7C: // player closed shop window
		case 0x7D: // request trade
		case 0x7E: // look at an item in trade
		case 0x7F: // accept trade
		case 0x80: // close/cancel trade
		case 0x82: // use item
		case 0x83: // use item
		case 0x84: // battle window
		case 0x85: // rotate item
		case 0x87: // close container
		case 0x88: // "up-arrow" - container
		case 0x89: // text window
		case 0x8A: // house window
		case 0x8C: // throw item
		case 0x97: // request channels
		case 0x98: // open Channel
		case 0x99: // close Channel
		case 0x9A: // open priv
		case 0x9B: // process report
		case 0x9C: // gm closes report
		case 0x9D: // player cancels report
		case 0x9E: // close NPC
		case 0xA0: // set attack and follow mode
		case 0xA3: // invite to party
		case 0xA4: // join party
		case 0xA5: // revoke party invitation
		case 0xA6: // pass party leadership
		case 0xA7: // leave party
		case 0xA8: // enable shared party experience
		case 0xAA: // create private channel
		case 0xAB: // channel invite
		case 0xAC: // channel exclude
		case 0xBE: // cancel move
		case 0xC9: // client request to resend the tile
		case 0xCA: // client request to resend the container (happens when you store more than container maxsize)
		case 0xD2: // request outfit
		case 0xD3: // set outfit
		case 0xDC: // add vip
		case 0xDD: // remove vip
		case 0xE8: // debug assert
		case 0xF0: // quest log
		case 0xF1: // quest line
			break;
		default:
			qWarning() << "[ProtocolGame::parsePlayClientMessage] Unknown opt " << qPrintable(QString().sprintf("0x%02X",opt));
			break;
	}
}

void ProtocolGame::parsePlayClientLogout(NetworkMessage&)
{
	qDebug("ProtocolGame::parsePlayClientStop");
	disconnectClient();
}

void ProtocolGame::parsePlayClientWalk(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parsePlayClientWalk");

	mMsg.reset();
	mMsg.addByte(0xB5);
	mMsg.addByte(msg.getByte());

	sendClientMessage(mMsg);
}

void ProtocolGame::parsePlayClientAttackOrFollow(NetworkMessage&)
{
	qDebug("ProtocolGame::parsePlayClientAttackOrFollow");

	mMsg.reset();
	mMsg.addByte(0xA3);

	sendClientMessage(mMsg);
}

/*
void parsePlayClientSay(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parsePlayClientSay");

}
*/
