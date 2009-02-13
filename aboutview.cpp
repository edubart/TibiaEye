#include "headers.h"
#include "aboutview.h"
#include "ui_aboutview.h"
#include "constants.h"

using namespace Constants;

AboutView::AboutView(QWidget *parent) :
	QWidget(parent),
	mUi(new Ui::AboutView)
{
	mUi->setupUi(this);

	const QString about = tr("<h2 align=\"center\">About Tibia Eye</h2><br/><br/>"
							 "<div align=\"center\">Application Name: %1<br/>"
							 "Version: %2<br/>"
							 "Build: %3<br/>"
							 "Authours: %4<br/>"
							 "Contact: %5<br/>"
							 "Website: %6<br/>"
							 "<br/>"
							 "<br/>"
							 "Copyright %7 %8. All rights reserved.<br/>"
							 "The program is provided AS IS with NO WARRANTY OF ANY KIND, "
							 "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A "
							 "PARTICULAR PURPOSE.<br/></div>")
							 .arg(APP_NAME, APP_VERSION_LONG, APP_BUILD_LONG, APP_AUTHOR, APP_CONTACT, APP_WEBSITE, APP_YEAR, APP_COMPANY);

	mUi->aboutText->setHtml(about);
	mUi->aboutText->setTextInteractionFlags(Qt::TextBrowserInteraction);
}

AboutView::~AboutView()
{
	delete mUi;
}
