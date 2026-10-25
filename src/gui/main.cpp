/*********************************************************
 ProtMate (Laboratory formula calculator) - graphical front end.
 Developed by Sanjeeva Wijeyesakere.
*********************************************************/

#include "MainWindow.h"
#include "pmate_core.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QApplication::setApplicationName(QStringLiteral("ProtMate"));
	QApplication::setOrganizationName(QStringLiteral("ProtMate"));
	QApplication::setApplicationVersion(QString::fromLatin1(pmate::VERSION));
	/* Window and taskbar icon on Linux and Windows; macOS takes the bundle icon */
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/protmate.png")));

	MainWindow w;
	w.show();
	return app.exec();
}
