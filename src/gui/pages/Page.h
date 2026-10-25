/* Base class and shared widgets for the ProtMate GUI pages. */

#ifndef PMATE_GUI_PAGE_H
#define PMATE_GUI_PAGE_H

#include <QString>
#include <QWidget>

#include <string>

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QVBoxLayout;

/* A titled page; subclasses add their content to `body`. */
class Page : public QWidget
{
public:
	Page(const QString& title, const QString& description, QWidget* parent = nullptr);

protected:
	QVBoxLayout* body;
};

namespace ui {

QDoubleSpinBox* doubleSpin(double min, double max, double value, int decimals = 3, const QString& suffix = QString());
QSpinBox* intSpin(int min, int max, int value);
QLabel* resultLabel();				/* Bold, selectable */
QLabel* noteLabel(const QString& text = QString());	/* Small, wrapped, for hints and errors */
QPlainTextEdit* textView();			/* Read-only monospace output */
QPlainTextEdit* sequenceEdit();		/* Monospace input box for pasted sequences */

QString num(double v);				/* Formatted like the terminal version; "—" if not finite */
QString num2(float v);				/* Rounded to 2 decimal places first */

/* A line edit with a "Browse…" button that opens a file dialog. */
QWidget* filePicker(QLineEdit*& edit, const QString& caption, const QString& filter, bool directory = false);

/* Asks where to save, then writes `text` atomically. Returns the path, or "" if cancelled or failed. */
QString saveText(QWidget* parent, const QString& caption, const QString& suggestedName,
				 const std::string& text, const QString& filter = QString());

/* Reads a whole file, refusing anything larger than maxBytes. Shows an error and returns false on failure. */
bool readFileBytes(QWidget* parent, const QString& path, std::string& out, qint64 maxBytes);

/* Lets the user pick a sequence file and puts its contents into `edit`. Returns the chosen path or "". */
QString loadSequenceInto(QWidget* parent, QPlainTextEdit* edit);

QString lastDir();
void rememberDir(const QString& path);
QString userName();

}

#endif
