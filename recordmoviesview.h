#ifndef RECORDMOVIESVIEW_H
#define RECORDMOVIESVIEW_H

namespace Ui {
	class RecordMoviesView;
}

class MovieFile;

class RecordMoviesView : public QWidget
{
	Q_OBJECT

public:
	RecordMoviesView(QWidget *parent);
	virtual ~RecordMoviesView();
	
private slots:
	void startRecord();
	void stopRecord();

	void onRecordStart();
	void onRecordTime(uint32 mstime);
	void onRecordSave();

	void browseMovie();

private:
	MovieFile *mMovieFile;
	Ui::RecordMoviesView *mUi;
};

#endif // RECORDMOVIESVIEW_H
