#include "headers.h"
#include "protocolgame.h"
#include "modemanager.h"
#include "moviefile.h"
#include "serverproxy.h"
#include "rsa.h"

void ProtocolGame::parseRecordFirstClientMessage(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parseRecordFirstClientMessage");

	// client information
	uint16 clientos = msg.getU16();
	uint16 version  = msg.getU16();

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

	// get account details
	uint8 isSetGM = msg.getByte();
	QString account = msg.getString();
	QString name = msg.getString();
	QString password = msg.getString();

	// setup mMsg
	mMsg.reset();
	mMsg.addByte(0x0A);                      // login server protid
	mMsg.addU16(clientos);                   // OS
	mMsg.addU16(version);                    // tibia version
	mMsg.addByte(0x00);                      // first RSA byte should be 0x00
	mMsg.addBytes((const uint8*)&key,16);    // XTEA key
	mMsg.addByte(isSetGM);                   // gm byte
	mMsg.addString(account);                 // account name
	mMsg.addString(name);                    // character name
	mMsg.addString(password);                // password

	//fill left bytes
	for(int i = 24 + account.length() + password.length() + name.length(); i < 128; i++)
		mMsg.addByte(0x00);

	//encrypt mMsg with RSA
	if(!RSA_encrypt(RSA::instance(), mMsg)) {
		disconnectClient();
		return;
	}

	// search character
	Character *c = NULL;
	CharacterList charList = mModeManager->getCharList();
	foreach(Character *cc, charList) {
		if(name == cc->name) {
			c = cc;
			break;
		}
	}
	if(!c) {
		qWarning() << "[ProtocolGame::parseFirstClientMessage] Character in game login not found.";
		enableXTEAEncryption();
		disconnectWithError(tr("Character not detected by Tibia Eye, please relogin your account."));
		return;
	}

	//convert ip
	QString ip = QString("%1.%2.%3.%4").arg((uint8)(c->ip)).arg((uint8)(c->ip >> 8)).arg((uint8)(c->ip >> 16)).arg((uint8)(c->ip >> 24));

	//setup record
	MovieFile *movieFile = mModeManager->getMovieFile();

	if(!movieFile->getPlayerName().isEmpty()) {
		if(movieFile->getPlayerName() != c->name ||
		   movieFile->getPlayerWorld() != c->world) {
			enableXTEAEncryption();
			disconnectWithError(tr("You cannot record multiple characters in the same movie."));
			return;
		}
		else if(movieFile->getTibiaVersion() != version) {
			enableXTEAEncryption();
			disconnectWithError(tr("You cannot record a different Tibia clients versions in the same movie."));
			return;
		}
	} else {
		movieFile->setPlayerName(c->name);
		movieFile->setPlayerWorld(c->world);
		movieFile->setTibiaVersion(version);
	}

	movieFile->setWorldIp(c->ip);

	//connect to server via proxy
	ServerProxy *server = new ServerProxy(this);

	connect(this, SIGNAL(serverConnected()), this, SLOT(parseRecordServerConnect()));
	connect(this, SIGNAL(serverDisconnected()), this, SLOT(parseRecordServerDisconnect()));

	connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(parseRecordServerMessage(NetworkMessage&)));
	connect(this, SIGNAL(clientMessage(NetworkMessage&)), this, SLOT(parseRecordClientMessage(NetworkMessage&)));

	connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(sendClientMessage(NetworkMessage&)));
	connect(this, SIGNAL(clientMessage(NetworkMessage&)), this, SLOT(sendServerMessage(NetworkMessage&)));

	setServer(server);
	server->connectToHost(ip, c->port);
}

void ProtocolGame::parseRecordServerConnect()
{
	qDebug("ProtocolGame::parseRecordServerConnect");

	sendServerMessage(mMsg);
	enableXTEAEncryption();
}

void ProtocolGame::parseRecordServerDisconnect()
{
	qDebug("ProtocolGame::parseRecordServerDisconnect");

	mModeManager->getMovieFile()->stopMovie();
	mModeManager->getMovieFile()->saveMovie();
}

void ProtocolGame::parseRecordServerMessage(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parseRecordServerMessage");

	bool record = false;

	uint8 opt = msg.getByte();
	switch(opt) {
		case 0x14: // error message
			msg.getString(); // error string
			break;
		case 0x16: // waitlist
			msg.getString(); // error string
			msg.getByte(); // retry time
			break;
/*
		case 0x0A: // player login
			msg.getU32(); // player id
			msg.getByte();
			msg.getByte();
			msg.getByte(); // can report bugs
			break;
		case 0x0B: // gm actions
			msg.skipBytes(32);
			break;
		case 0x64: // map description
			break;
		case 0x79: // remove inventory item
			msg.getByte(); // inventory id
			break;
		case 0xA0: // player stats
			parseRecordPlayerStats(msg);
			break;
*/
		default:
			record = true;
			break;
	}

	if(record)
		mModeManager->getMovieFile()->recordMessage(msg);
}

void ProtocolGame::parseRecordClientMessage(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parseRecordClientMessage");

	uint8 opt = msg.getByte();
	switch(opt) {
		case 0x96: // say something
			parseRecordClientPrivateMessage(msg);
			break;

		case 0x14: // logout
		case 0x1E: // keep alive / ping response
		case 0x64: // move with steps
		case 0x65: // move north
		case 0x66: // move east
		case 0x67: // move south
		case 0x68: // move west
		case 0x69: // stop-autowalk
		case 0x6A: // move northeast
		case 0x6B: // move southeast
		case 0x6C: // move southwest
		case 0x6D: // move northwest
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
		case 0xA1: // attack
		case 0xA2: // follow
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
			qWarning() << "[ProtocolGame::parseRecordClientMessage] Unknown opt " << qPrintable(QString().sprintf("0x%02X",opt));
			break;
	}
}

void ProtocolGame::parseRecordClientPrivateMessage(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parseRecordClientPrivateMessage");

	QString reciver;
	uint8 type = msg.getByte();

	switch(type){
		case SPEAK_PRIVATE:
		case SPEAK_PRIVATE_RED:
			reciver = msg.getString();
			break;
		default:
			return;
	}

	QString text = msg.getString();
	text.prepend(">> ");

	mMsg.reset();
	mMsg.addByte(0xAA);
	mMsg.addU32(0x00000000);
	mMsg.addString(reciver);
	mMsg.addU16(0); // speaker level
	mMsg.addByte(type);
	mMsg.addString(text);

	mModeManager->getMovieFile()->recordMessage(mMsg);
}

void ProtocolGame::parseRecordPlayerStats(NetworkMessage& msg)
{
	qDebug("ProtocolGame::parseRecordPlayerStats");

	msg.getU16(); // health
	msg.getU16(); // max health
	msg.getU32(); // free capacity
	msg.getU32(); // ??

	uint16 level = msg.getU16(); // level
	mModeManager->getMovieFile()->setPlayerLevel(level);

	msg.getByte(); // level percent
	msg.getU16(); // mana
	msg.getU16(); // max mana
	msg.getByte(); // magic level
	msg.getByte(); // magic level percent
	msg.getByte(); // soul
	msg.getU16(); // stamina
}
