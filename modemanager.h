#ifndef MODEMANAGER_H
#define MODEMANAGER_H

class MovieFile;
class ClientProxy;

enum eModes {
	MODE_NONE        = 0,
	MODE_RECORD      = 1,
	MODE_PLAY        = 2,
	MODE_STREAM      = 4,
	MODE_WATCHSTREAM = 8
};

struct Character
{
	QString name;
	QString world;
	uint32 ip;
	uint16 port;
};
typedef QVector<Character*> CharacterList;

class ModeManager : public QObject, public Singleton<ModeManager>
{
	Q_OBJECT

public:
    ModeManager();
	virtual ~ModeManager();

	bool startMode(eModes mode);
	void stopMode();

	void setCharList(CharacterList &charList) {
		qDeleteAll(mCharList);
		mCharList = charList;
	}
	const CharacterList &getCharList() { return mCharList; }

	bool isModeRunning() const { return mMode != MODE_NONE; }
	eModes getMode() const { return mMode; }

	void setMovieFile(MovieFile *movieFile) { mMovieFile = movieFile; }
	MovieFile *getMovieFile() const { return mMovieFile; }

private:
	ClientProxy *mClientProxy;
	MovieFile *mMovieFile;
	CharacterList mCharList;
	eModes mMode;
};

#endif // MODEMANAGER_H
