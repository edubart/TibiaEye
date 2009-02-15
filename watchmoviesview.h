#ifndef WATCHMOVIESVIEW_H
#define WATCHMOVIESVIEW_H

namespace Ui {
	class WatchMoviesView;
}

class MovieFile;

class WatchMoviesView : public QWidget, public Singleton<WatchMoviesView>
{
	Q_OBJECT

public:
	WatchMoviesView(QWidget *parent);
	virtual ~WatchMoviesView();

public slots:
	void updateMoviesList();

private slots:

	void startPlay();
	void stopPlay();

	void speedChanged(int value);
	void movieChanged();

	void filterMovies(const QString &filter);

	void onPlayStart();
	void onPlayTime(uint32 mstime);

private:
	Ui::WatchMoviesView *mUi;

	MovieFile *mMovieFile;
};

#endif // WATCHMOVIESVIEW_H
