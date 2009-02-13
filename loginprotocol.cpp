#include "headers.h"
#include "loginprotocol.h"
#include "clientproxy.h"
#include "serverproxy.h"
#include "modemanager.h"
#include "rsa.h"
#include "moviefile.h"
#include "optionsview.h"

LoginProtocol::LoginProtocol(ClientProxy *client)
	: Protocol(client)
{
	qDebug("LoginProtocol::LoginProtocol");

	mModeManager = ModeManager::instance();
	mMovieFile = mModeManager->getMovieFile();

	connect(this, SIGNAL(firstClientMessage(NetworkMessage&)), this, SLOT(parseFirstClientMessage(NetworkMessage&)));
}

LoginProtocol::~LoginProtocol()
{
	qDebug("LoginProtocol::~LoginProtocol");
}

void LoginProtocol::parseFirstClientMessage(NetworkMessage& msg)
{
	qDebug("LoginProtocol::parseFirstClientMessage");

	// client information
	uint16 clientos = msg.getU16();
	uint16 version  = msg.getU16();

	// check tibia version
	bool isVersionAvailable = false;
	for(int i=0; i<ConstTibia::VersionsNum; i++) {
		if(version == ConstTibia::AvailableVersionsIds[i])
			isVersionAvailable = true;
	}
	if(!isVersionAvailable)
		qWarning() << "[LoginProtocol::parseFirstClientMessage] Client connection with unsupported Tibia version.";

	// skip uneeded bytes
	uint8 skippedBytes[12];
	msg.getBytes(skippedBytes,12);

	// decrypt rsa
	if(!RSA_decrypt(RSA::instance(), msg)) {
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

	// disconnect client if invalid version
	if(!isVersionAvailable) {
		enableXTEAEncryption();
		disconnectWithError(tr("Your tibia version is not supported by Tibia Eye."));
		return;
	}

	// play mode
	if(mModeManager->getMode() & MODE_PLAY)
	{
		enableXTEAEncryption();
		sendMsg.reset();
		sendMsg.addByte(0x64); // character list opcode
		sendMsg.addByte(0x01); // 1 character
		sendMsg.addString(mMovieFile->getPlayerName()); // player name
		sendMsg.addString(mMovieFile->getPlayerWorld()); // player world
		sendMsg.addU32(0x0100007f); // 127.0.0.1 ip address
		sendMsg.addU16(8000); // proxy port
		sendMsg.addU16(0x00); // prem days
		sendClientMessage(sendMsg);
		disconnectClient();
	}
	// record mode
	else if(mModeManager->getMode() & MODE_RECORD)
	{
		// get the remote server
		QString host = OptionsView::instance()->getServerHost();
		uint16 port = OptionsView::instance()->getServerPort();

		// get account and password
		QString account = msg.getString();
		QString password = msg.getString();

		// setup send message with rsa for server login
		sendMsg.reset();
		sendMsg.addByte(0x01);                      // login server protid
		sendMsg.addU16(clientos);                   // OS
		sendMsg.addU16(version);                    // tibia version
		sendMsg.addBytes(skippedBytes,12);          // skipped signatures bytes
		sendMsg.addByte(0x00);                      // first RSA byte should be 0x00
		sendMsg.addBytes((const uint8*)&key,16);  // XTEA key
		sendMsg.addString(account);                 // account name
		sendMsg.addString(password);                // password

		// fill left bytes
		for(int i = 21 + account.length() + password.length(); i < 128; i++)
			sendMsg.addByte(0x00);

		// encrypt sendMsg with RSA
		if(!RSA_encrypt(RSA::instance(), sendMsg)) {
			disconnectClient();
			return;
		}

		// setup record
		mMovieFile->setServerHost(host);

		// connect to server via proxy
		ServerProxy *serverProxy = new ServerProxy(this);

		connect(this, SIGNAL(serverConnected()), this, SLOT(parseRecordServerConnect()));
		connect(this, SIGNAL(serverMessage(NetworkMessage&)), this, SLOT(parseRecordServerMessage(NetworkMessage&)));

		setServer(serverProxy);
		serverProxy->connectToHost(host, port);
	}
}

void LoginProtocol::parseRecordServerConnect()
{
	qDebug("LoginProtocol::parseRecordServerConnect");

	sendServerMessage(sendMsg);
	enableXTEAEncryption();
}

void LoginProtocol::parseRecordServerMessage(NetworkMessage& msg)
{
	qDebug("LoginProtocol::parseRecordServerMessage");

	sendMsg.reset();

	while(!msg.end())
	{
		uint8 opt = msg.getByte();
		sendMsg.addByte(opt);

		switch(opt)
		{
			case 0x0A: // error message
			case 0x14: // MOTD message
				sendMsg.addString(msg.getString());
				break;
			case 0x64: // character list
				{
					CharacterList charList;
					int8 numChars = msg.getByte();
					sendMsg.addByte(numChars);
					for(int i=0; i < numChars; i++) {
						Character *c = new Character;
						c->name = msg.getString();
						c->world = msg.getString();
						c->ip = msg.getU32();
						c->port = msg.getU16();

						charList.push_back(c);

						sendMsg.addString(c->name);
						sendMsg.addString(c->world);
						sendMsg.addU32(0x0100007f); // 127.0.0.1 ip address
						sendMsg.addU16(client()->listenPort()); // proxy port
					}
					sendMsg.addU16(msg.getU16()); //prem days
					mModeManager->setCharList(charList);
				}
				break;
			default:
				qWarning() << "[LoginProtocol::parseRecordServerMessage] Invalid loginserver opcode.";
				break;
		}
	}

	sendClientMessage(sendMsg);
	disconnectServer();
	disconnectClient();
}

void LoginProtocol::disconnectWithError(const QString &message)
{
	qDebug("LoginProtocol::disconnectWithError");

	sendMsg.reset();
	sendMsg.addByte(0x0A);
	sendMsg.addString(message);
	sendClientMessage(sendMsg);
	disconnectClient();
}
