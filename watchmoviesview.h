#ifndef WATCHMOVIESVIEW_H
#define WATCHMOVIESVIEW_H

namespace Ui {
	class WatchMoviesView;
}

class MovieFile;

class WatchMoviesView : public QWidget
{
	Q_OBJECT

public:
	WatchMoviesView(QWidget *parent);
	virtual ~WatchMoviesView();

protected slots:
	void onPlayStart();
	void onPlayFinish();
	void onPlayTime(uint32 mstime);

private slots:
	void updateMoviesList();
	void activatePlay();
	void stopPlay();
	void speedChanged(int value);
	void movieChanged();
	void filterMovies(const QString &filter);

private:
	Ui::WatchMoviesView *mUi;

	MovieFile *mMovieFile;
};

#endif // WATCHMOVIESVIEW_H
