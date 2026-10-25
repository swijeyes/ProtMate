/* ProtMate main window: a list of modules beside the selected module's page. */

#ifndef PMATE_GUI_MAINWINDOW_H
#define PMATE_GUI_MAINWINDOW_H

#include <QMainWindow>

class Page;
class QListWidget;
class QStackedWidget;

class MainWindow : public QMainWindow
{
public:
	explicit MainWindow(QWidget* parent = nullptr);

private:
	void addSection(const QString& title);
	void addPage(const QString& name, Page* page);
	void createMenus();
	void openInRasmol();
	void about();

	QListWidget* nav;
	QStackedWidget* pages;
	QString rasmolPath;
};

#endif
