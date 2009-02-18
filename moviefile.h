#ifndef MOVIEFILE_H
#define MOVIEFILE_H

#define MOVIE_FILE_SIGNATURE	0x91F856FE
#define MOVIE_FILE_VERSION		0x04
#define MAX_MOVIE_STRING_SIZE	8192

enum eMovieTypes {
	EMT_HUNT = 1,
	EMT_QUEST,
	EMT_EXPLORE,
	EMT_PLAYERKILL,
	EMT_WAR,
	EMT_PARTY,
	EMT_OTHER
};

struct Packet
{
	uint32 time; // sent time
	uint16 size; // size of the data
	uint8 *data; // data of the packet without encryption and checksum
};
typedef QVector<Packet*> PacketVector;

class NetworkMessage;

//TODO: movie system total remake, move MovieFile to just Movie
class MovieFile : public QObject
{
	Q_OBJECT

	struct movieheader_t {
		uint32 signature;
		uint8 version;
	};

	enum eMovieOpcodes {
		OPT_MOVIE_TITLE = 0x01,
		OPT_MOVIE_DESCRIPTION = 0x02,
		OPT_MOVIE_TYPE = 0x03,
		OPT_MOVIE_DATE = 0x04,
		OPT_MOVIE_DURATION = 0x05,
		OPT_PLAYER_NAME = 0x06,
		OPT_PLAYER_LEVEL = 0x07,
		OPT_PLAYER_WORLD = 0x08,
		OPT_SERVER_HOST = 0x09,
		OPT_WORLD_IP = 0x10,
		OPT_TIBIA_VERSION = 0x11,
		OPT_PLATFORM = 0x12,
		OPT_OSTYPE = 0x13,
		OPT_CRC = 0x14,
		OPT_PACKET = 0xFE,
		OPT_EOF = 0xFF,
	};

public:
	MovieFile(const QString &filename = QString());
	virtual ~MovieFile();

	bool loadMovie(QString filename, bool loadPackets = true);
	bool loadMovie(bool loadPackets = true);

	int playMessage(NetworkMessage &msg);
	void rewind();

	void lockRecord(bool lock) { mRecordLock = lock; }
	bool isRecordLocked() { return mRecordLock; }

	// sets
	void setMovieTitle(QString movieTitle) { mMovieTitle = movieTitle; }
	void setMovieDescription(QString movieDescription) { mMovieDescription = movieDescription; }
	void setMovieType(uint8 movieType) { mMovieType = movieType; }
	void setPlayerName(QString playerName) { mPlayerName = playerName; }
	void setPlayerLevel(uint16 playerLevel) { mPlayerLevel = playerLevel; }
	void setPlayerWorld(QString playerWorld) { mPlayerWorld = playerWorld; }
	void setServerHost(QString serverHost) { mServerHost = serverHost; }
	void setWorldIp(uint32 worldIp) { mWorldIp = worldIp; }
	void setTibiaVersion(uint16 tibiaVersion) { mTibiaVersion = tibiaVersion; }
	void setSpeed(float speed) { mSpeed = speed; }

	// gets
	const QString &getMovieFile() const { return mFilename; }
	const QString &getMovieTitle() const { return mMovieTitle; }
	const QString &getMovieDescription() const { return mMovieDescription; }
	uint8 getMovieType() const { return mMovieType; }
	QString getMovieTypeName() const;
	const QString &getPlayerName() const { return mPlayerName; }
	uint16 getPlayerLevel() const { return mPlayerLevel; }
	const QString &getPlayerWorld() const { return mPlayerWorld; }
	const QString &getServerHost() const { return mServerHost; }
	uint32 getWorldIp() const { return mWorldIp; }
	uint16 getTibiaVersion() const { return mTibiaVersion; }
	float getSpeed() const { return mSpeed; }

	uint32 getMovieDate() const { return mMovieDate; }
	uint32 getMovieDuration() const { return mDuration; }

	uint32 getPlayedTime();
	uint8 getPlayedPercent();

signals:
	void movieStarted();
	void movieTime(uint32);
	void movieStopped();
	void movieSaved();

public slots:
	void stopMovie();
	void saveMovie();

	void recordMessage(NetworkMessage &msg);

private:
	void resetEverything();

	QString mFilename;
	QString mMovieTitle;
	QString mMovieDescription;
	uint8 mMovieType;
	QString mPlayerName;
	uint16 mPlayerLevel;
	QString mPlayerWorld;
	QString mServerHost;
	uint32 mWorldIp;
	uint16 mTibiaVersion;

	uint32 mMovieDate;
	uint32 mDuration;
	PacketVector mPackets;
	PacketVector::iterator currentPacketIt;
	float mSpeed;

	bool mRecordLock;

	QTime timeCounter;
};

#endif // MOVIEFILE_H
