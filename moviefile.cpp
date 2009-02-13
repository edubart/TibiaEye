#include "headers.h"
#include "moviefile.h"
#include "util.h"
#include "networkmessage.h"

MovieFile::MovieFile(const QString &filename) :
	QObject(), mFilename(filename)
{
	qDebug("MovieFile::MovieFile");

	mMovieType = EMT_OTHER;
	mPlayerLevel = 0;
	mWorldIp = 0;
	mTibiaVersion = 0;

	mMovieDate = 0;
	mStartMSec = 0;
	mDuration = 0;

	mSpeed = 1.0f;

	mRecordLock = false;
	currentPacketIt = mPackets.end();
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

bool MovieFile::loadMovie(bool loadPackets)
{
    return loadMovie(mFilename, loadPackets);
}

bool MovieFile::loadMovie(QString filename, bool loadPackets)
{
	qDebug("MovieFile::loadMovie");

	mMovieType = EMT_OTHER;
	mPlayerLevel = 0;
	mWorldIp = 0;
	mTibiaVersion = 0;
	mMovieDate = 0;
	mStartMSec = 0;
	mDuration = 0;
	mSpeed = 1.0f;
	mFilename = filename;

	mMovieTitle.clear();
	mMovieDescription.clear();
	mPlayerName.clear();
	mPlayerWorld.clear();
	mServerHost.clear();

	mRecordLock = true;
	currentPacketIt = mPackets.end();

	// open the file
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) {
		qCritical() << qPrintable(tr("Failed opening movie file \"%1\" for reading.").arg(filename));
		return false;
	}

	// setup input stream
	QDataStream in(&file);

	// check file signature and version
	uint32 signature;
	uint8 version;
	in >> signature >> version;

	if(signature != MOVIE_FILE_SIGNATURE) {
		qCritical() << qPrintable(tr("Invalid Tibia Eye movie format: %1").arg(filename));
		return false;
	}

    if(version < MOVIE_FILE_VERSION) {
		qWarning() << "[MovieFile::loadMovie] Reading an old Tibia Eye movie file version!";
	} else if(version > MOVIE_FILE_VERSION) {
		qWarning() << "[MovieFile::loadMovie] Reading a newer Tibia Eye movie file version!";
	}

	in.setVersion(QDataStream::Qt_4_4);

	// clear older packets
	foreach(Packet *packet, mPackets) {
		delete packet->data;
		delete packet;
	}
    mPackets.clear();

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

            case OPT_PACKET: // Packet.
			{
                if(!loadPackets) {
                    file.close();
                    return true;
                }

                Packet *packet = new Packet;

                in >> packet->flags;
                in >> packet->time;
                in >> packet->size;

                packet->data = new uint8 [packet->size];
                in.readRawData((char*)packet->data, packet->size);

                mPackets.push_back(packet);
                break;
            }

            default:
				qCritical() << qPrintable(tr("Error reading movie file: %1").arg(filename));
    			file.close();
                return false;
        }
        in >> opt;
    }

    currentPacketIt = mPackets.begin();
    file.close();
    return true;
}

void MovieFile::saveMovie()
{
	qDebug("MovieFile::saveMovie");

	if(mPackets.size() <= 0)
		return;

	// open the file
	QFile file(mFilename);
	if(!file.open(QIODevice::WriteOnly)) {
		qCritical() << qPrintable(tr("Failed opening file %1 for writing.").arg(mFilename));
		return;
	}

	// setup output stream
	QDataStream out(&file);

	out << (uint32)MOVIE_FILE_SIGNATURE;
	out << (uint8)MOVIE_FILE_VERSION;

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

	foreach(Packet *packet, mPackets) {
		out << (uint8)OPT_PACKET;
		out << (uint8)packet->flags;
        out << packet->time;
        out << packet->size;
		out.writeRawData((const char*)packet->data, packet->size);
    }

	out << (uint8)OPT_EOF;

	file.close();

	emit movieSaved();
}


int MovieFile::playMessage(NetworkMessage &msg)
{
	qDebug("MovieFile::playMessage");

	if(currentPacketIt == mPackets.end()) {
		emit movieTime(mDuration);
		emit movieStopped();
		currentPacketIt = mPackets.begin();
		return -1;
	}

	if(currentPacketIt == mPackets.begin()) {
		emit movieTime(0);
		emit movieStarted();
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
	return 0;
}

void MovieFile::recordMessage(NetworkMessage &msg)
{
	qDebug("MovieFile::recordMessage");

	if(!mRecordLock) {
		QDateTime currentTime = QDateTime::currentDateTime();
		if(mPackets.empty()) {
			mMovieDate = currentTime.toTime_t();
			mStartMSec = currentTime.time().msec();
			mDuration = 0;

			emit movieStarted();
		} else {
			mDuration = (currentTime.toTime_t() - mMovieDate)*1000 + currentTime.time().msec() - mStartMSec;
		}

		Packet *packet = new Packet;
		packet->flags = PACKET_SERVER;
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
