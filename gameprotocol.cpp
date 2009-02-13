#include "headers.h"
#include "gameprotocol.h"
#include "clientproxy.h"
#include "serverproxy.h"
#include "servervirtual.h"
#include "modemanager.h"
#include "rsa.h"
#include "moviefile.h"

GameProtocol::GameProtocol(ClientProxy *client)
	: Protocol(client)
{
	qDebug("GameProtocol::GameProtocol");

	mModeManager = ModeManager::instance();
	mMovieFile = mModeManager->getMovieFile();

	connect(this, SIGNAL(firstClientMessage(NetworkMessage&)), this, SLOT(parseFirstClientMessage(NetworkMessage&)));
}

GameProtocol::~GameProtocol()
{
	qDebug("GameProtocol::~GameProtocol");
}

void GameProtocol::parseFirstClientMessage(NetworkMessage& msg)
{
	qDebug("GameProtocol::parseFirstClientMessage");

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

	// play mode
	if(mModeManager->getMode() & MODE_PLAY) {
		ServerVirtual *serverVirtual = new ServerVirtual(this, mMovieFile);

		connect(this, SIGNAL(serverConnected()), this, SLOT(enableXTEAEncryption()));
		connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(sendClientMessage(NetworkMessage&)));

		setServer(serverVirtual);
		serverVirtual->start();
	// record mode
	} else if (mModeManager->getMode() & MODE_RECORD) {
		// get account details
		uint8 isSetGM = msg.getByte();
		QString account = msg.getString();
		QString name = msg.getString();
		QString password = msg.getString();

		// setup sendMsg
		sendMsg.reset();
		sendMsg.addByte(0x0A);                      // login server protid
		sendMsg.addU16(clientos);                   // OS
		sendMsg.addU16(version);                    // tibia version
		sendMsg.addByte(0x00);                      // first RSA byte should be 0x00
		sendMsg.addBytes((const uint8*)&key,16);  // XTEA key
		sendMsg.addByte(isSetGM);                   // gm byte
		sendMsg.addString(account);                 // account name
		sendMsg.addString(name);                    // character name
		sendMsg.addString(password);                // password

		//fill left bytes
		for(int i = 24 + account.length() + password.length() + name.length(); i < 128; i++)
			sendMsg.addByte(0x00);

		//encrypt sendMsg with RSA
		if(!RSA_encrypt(RSA::instance(), sendMsg)) {
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
			qWarning() << "[GameProtocol::parseFirstClientMessage] Character in game login not found.";
			enableXTEAEncryption();
			disconnectWithError(tr("You may have restarted Tibia Eye, please relogin."));
			return;
		}

		//convert ip
		QString ip = QString("%1.%2.%3.%4").arg((uint8)(c->ip)).arg((uint8)(c->ip >> 8)).arg((uint8)(c->ip >> 16)).arg((uint8)(c->ip >> 24));

		//setup record
		//TODO: these sets replaces the first sets in multiple record mode, so fix
		mMovieFile->setPlayerName(c->name);
		mMovieFile->setPlayerWorld(c->world);
		mMovieFile->setWorldIp(c->ip);
		mMovieFile->setTibiaVersion(version);

		//connect to server via proxy
		ServerProxy *serverProxy = new ServerProxy(this);

		//TODO: do not record waiting list
		connect(this, SIGNAL(serverConnected()), this, SLOT(parseRecordServerConnect()));
		connect(this, SIGNAL(serverDisconnected()), mMovieFile, SLOT(saveMovie()));
		connect(this, SIGNAL(serverMessage(NetworkMessage&)), mMovieFile, SLOT(recordMessage(NetworkMessage&)));
		connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(sendClientMessage(NetworkMessage&)));
		connect(this, SIGNAL(clientMessage(NetworkMessage&)), this, SLOT(sendServerMessage(NetworkMessage&)));

		setServer(serverProxy);
		serverProxy->connectToHost(ip, c->port);
	}
}

void GameProtocol::parseRecordServerConnect()
{
	qDebug("GameProtocol::parseRecordServerConnect");

	sendServerMessage(sendMsg);
	enableXTEAEncryption();
}

void GameProtocol::disconnectWithError(const QString &message)
{
	qDebug("GameProtocol::disconnectWithError");

	sendMsg.reset();
	sendMsg.addByte(0x14);
	sendMsg.addString(message);
	sendClientMessage(sendMsg);
	disconnectClient();
}
