#include "headers.h"
#include "moviefile.h"
#include "util.h"
#include "networkmessage.h"
#include "watchmoviesview.h"

MovieFile::MovieFile(const QString &filename) :
	QObject(), mFilename(filename)
{
	qDebug("MovieFile::MovieFile");
	resetEverything();
}

MovieFile::~MovieFile()
{
	qDebug("MovieFile::~MovieFile");

	foreach(Packet *packet, mPackets) {
		delete packet->data;
		delete packet;
	}
	mPackets.clear();
}

void MovieFile::resetEverything()
{
	mMovieType = EMT_OTHER;
	mPlayerLevel = 0;
	mWorldIp = 0;
	mTibiaVersion = 0;
	mMovieDate = 0;
	mDuration = 0;
	mSpeed = 1.0f;

	foreach(Packet *packet, mPackets) {
		delete packet->data;
		delete packet;
	}
	mPackets.clear();
	mMovieTitle.clear();
	mMovieDescription.clear();
	mPlayerName.clear();
	mPlayerWorld.clear();
	mServerHost.clear();

	mRecordLock = false;
	currentPacketIt = mPackets.end();
}

bool MovieFile::loadMovie(bool loadPackets)
{
    return loadMovie(mFilename, loadPackets);
}

bool MovieFile::loadMovie(QString filename, bool loadPackets)
{
	qDebug("MovieFile::loadMovie");

	resetEverything();
	mFilename = filename;

	// open the file
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly))
		return false;

	struct movieheader_t header;
	file.read((char *)&header, sizeof(struct movieheader_t));

	if(header.signature != MOVIE_FILE_SIGNATURE)
		return false;

	if(header.version < MOVIE_FILE_VERSION) {
		qWarning() << "[MovieFile::loadMovie] Reading an old Tibia Eye movie file version!";
	} else if(header.version > MOVIE_FILE_VERSION) {
		qWarning() << "[MovieFile::loadMovie] Reading a newer Tibia Eye movie file version!";
	}

	// setup input stream
	QByteArray byteArray = file.readAll();
	file.close();

	QDataStream in(&byteArray, QIODevice::ReadOnly);
	in.setVersion(QDataStream::Qt_4_4);

	// read the file
    uint8 opt;
	in >> opt;
	while(opt != OPT_EOF)
    {
        switch(opt)
        {
            case OPT_MOVIE_TITLE:
				in >> mMovieTitle;
                break;

            case OPT_MOVIE_DESCRIPTION:
                in >> mMovieDescription;
                break;

            case OPT_MOVIE_TYPE:
                in >> mMovieType;
                break;

            case OPT_MOVIE_DATE:
                in >> mMovieDate;
                break;

            case OPT_MOVIE_DURATION:
                in >> mDuration;
                break;

            case OPT_PLAYER_NAME:
				in >> mPlayerName;
                break;

            case OPT_PLAYER_LEVEL:
                in >> mPlayerLevel;
                break;

            case OPT_PLAYER_WORLD:
                in >> mPlayerWorld;
                break;

            case OPT_SERVER_HOST:
                in >> mServerHost;
                break;

            case OPT_WORLD_IP:
                in >> mWorldIp;
                break;

            case OPT_TIBIA_VERSION:
                in >> mTibiaVersion;
                break;

            case OPT_PLATFORM:
                in.skipRawData(1);
                break;

			case OPT_OSTYPE:
                in.skipRawData(2);
				break;

			case OPT_CRC:
			{
				uint32 checkSum = adlerChecksum((uint8*)byteArray.constData(), in.device()->pos());
				uint32 readCheckSum;
				in >> readCheckSum;
				if(checkSum != readCheckSum)
					return false;
				break;
			}

            case OPT_PACKET: // Packet.
			{
				if(!loadPackets)
					return true;

				uint32 numPackets;
				uint32 bufferSize;

				in >> numPackets;
				in >> bufferSize;

				QByteArray uncompressedPackets = qUncompress(in.device()->read(bufferSize));
				QBuffer packetsBuffer(&uncompressedPackets);
				packetsBuffer.open(QIODevice::ReadOnly);

				for(uint32 i = 0; i< numPackets; i++) {
					Packet *packet = new Packet;
					packetsBuffer.read((char *)&packet->time, sizeof(uint32));
					packetsBuffer.read((char *)&packet->size, sizeof(uint16));
					packet->data = new uint8 [packet->size];
					packetsBuffer.read((char *)packet->data, packet->size);
					mPackets.push_back(packet);
				}
				packetsBuffer.close();

				currentPacketIt = mPackets.begin();
                break;
            }

			default:
    			file.close();
                return false;
        }
        in >> opt;
    }

    return true;
}

void MovieFile::saveMovie()
{
	qDebug("MovieFile::saveMovie");

	if(mPackets.size() <= 0) {
		qWarning() << "[MovieFile::saveMovie] No packets to save.";
		return;
	}

	// open the file
	QFile file(mFilename);
	if(!file.open(QIODevice::WriteOnly)) {
		qCritical() << qPrintable(tr("Failed opening file %1 for writing.").arg(mFilename));
		return;
	}

	// write file header
	struct movieheader_t header;
	header.signature = MOVIE_FILE_SIGNATURE;
	header.version = MOVIE_FILE_VERSION;
	file.write((const char*)&header, sizeof(struct movieheader_t));

	// setup output stream
	QByteArray byteArray;
	QDataStream out(&byteArray, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_4);

	if(!mMovieTitle.isEmpty()) {
		out << (uint8)OPT_MOVIE_TITLE;
		out << mMovieTitle;
	}

	if(!mMovieDescription.isEmpty()) {
		out << (uint8)OPT_MOVIE_DESCRIPTION;
		out << mMovieDescription;
	}

	out << (uint8)OPT_MOVIE_TYPE;
    out << mMovieType;

	out << (uint8)OPT_MOVIE_DATE;
    out << mMovieDate;

	out << (uint8)OPT_MOVIE_DURATION;
    out << mDuration;

	if(!mPlayerName.isEmpty()) {
		out << (uint8)OPT_PLAYER_NAME;
		out << mPlayerName;
	}

	if(mPlayerLevel > 0) {
		out << (uint8)OPT_PLAYER_LEVEL;
		out << mPlayerLevel;
	}

	if(!mPlayerWorld.isEmpty()) {
		out << (uint8)OPT_PLAYER_WORLD;
		out << mPlayerWorld;
	}

	if(!mServerHost.isEmpty()) {
		out << (uint8)OPT_SERVER_HOST;
		out << mServerHost;
	}

	out << (uint8)OPT_WORLD_IP;
    out << mWorldIp;

	out << (uint8)OPT_TIBIA_VERSION;
    out << mTibiaVersion;

	out << (uint8)OPT_PLATFORM;
	out << (uint8)getPlataformType();

#ifdef Q_WS_WIN
	out << (uint8)OPT_OSTYPE;
	out << (uint16)QSysInfo::WindowsVersion;
#endif

	out << (uint8)OPT_CRC;
	out << adlerChecksum((uint8*)byteArray.constData(), out.device()->pos());

	QBuffer packetsBuffer;
	packetsBuffer.open(QIODevice::WriteOnly);

	foreach(Packet *packet, mPackets) {
		packetsBuffer.write((char *)&packet->time, sizeof(uint32));
		packetsBuffer.write((char *)&packet->size, sizeof(uint16));
		packetsBuffer.write((char *)packet->data, packet->size);
	}

	QByteArray compressedPackets =  qCompress(packetsBuffer.data(), 9);
	packetsBuffer.close();

	out << (uint8)OPT_PACKET;
	out << (uint32)mPackets.size();
	out << (uint32)compressedPackets.size();
	out.writeRawData(compressedPackets.constData(), compressedPackets.size());

	out << (uint8)OPT_EOF;

	file.write(byteArray);
	file.close();

	if(WatchMoviesView::instance())
		WatchMoviesView::instance()->updateMoviesList();
	emit movieSaved();
}

void MovieFile::stopMovie()
{
	qDebug("MovieFile::stopMovie");

	timeCounter = QTime();
	emit movieStopped();
}

int MovieFile::playMessage(NetworkMessage &msg)
{
	qDebug("MovieFile::playMessage");

	if(currentPacketIt == mPackets.end()) {
		emit movieTime(mDuration);
		stopMovie();
		currentPacketIt = mPackets.begin();
		return -1;
	}

	if(currentPacketIt == mPackets.begin()) {
		emit movieStarted();
		emit movieTime(0);
	}

	Packet *packet = *currentPacketIt++;

	Packet *nextPacket = NULL;
	if(currentPacketIt != mPackets.end())
		nextPacket = *currentPacketIt;

	msg.setMessageLength(packet->size);
	memcpy(msg.getBodyBuffer(), packet->data, packet->size);

	emit movieTime(packet->time);

	if(packet && nextPacket)
		return (uint32)((float)(nextPacket->time - packet->time)/mSpeed);
	return 2000;
}

void MovieFile::recordMessage(NetworkMessage &msg)
{
	qDebug("MovieFile::recordMessage");

	if(!mRecordLock && msg.getMessageLength() > 0) {
		if(mPackets.empty()) {
			mMovieDate = QDateTime::currentDateTime().toTime_t();
			timeCounter.start();
			mDuration = 0;
			emit movieStarted();
		} else if(timeCounter.isNull()) {
			mDuration += 1000;
			timeCounter.start();
			emit movieStarted();
		} else {
			mDuration += timeCounter.restart();
		}

		Packet *packet = new Packet;
		packet->time = mDuration;
		packet->size = msg.getMessageLength();
		packet->data = new uint8[packet->size];
		memcpy(packet->data, msg.getBodyBuffer(), packet->size);
		mPackets.push_back(packet);

		emit movieTime(mDuration);
	}
}

uint32 MovieFile::getPlayedTime()
{
    if(currentPacketIt == mPackets.begin())
        return 0;
    else if(currentPacketIt != mPackets.end())
        return (*currentPacketIt)->time;
    else
        return mDuration;
}

uint8 MovieFile::getPlayedPercent()
{
    if(currentPacketIt == mPackets.begin())
        return 0;
    else if(currentPacketIt != mPackets.end())
        return (uint8)(((*currentPacketIt)->time*100)/mDuration);
    else
        return 100;
}

QString MovieFile::getMovieTypeName() const
{
	switch(mMovieType) {
		case EMT_HUNT:
			return tr("Hunt");
		case EMT_QUEST:
			return tr("Quest");
		case EMT_EXPLORE:
			return tr("Explore");
		case EMT_PLAYERKILL:
			return tr("Player Kill");
		case EMT_WAR:
			return tr("War");
		case EMT_PARTY:
			return tr("Party");
		case EMT_OTHER:
		default:
			return tr("Other");
	}
}

void MovieFile::rewind()
{
	currentPacketIt = mPackets.begin();
}
