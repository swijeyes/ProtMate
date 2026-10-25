#include "Page.h"

#include "pmate_core.h"

#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

#include <cmath>

Page::Page(const QString& title, const QString& description, QWidget* parent)
	: QWidget(parent)
{
	body = new QVBoxLayout(this);
	body->setContentsMargins(24, 20, 24, 20);
	body->setSpacing(12);

	QLabel* heading = new QLabel(title);
	QFont f = heading->font();
	f.setPointSizeF(f.pointSizeF() * 1.5);
	f.setBold(true);
	heading->setFont(f);
	body->addWidget(heading);

	if (!description.isEmpty())
	{
		QLabel* desc = new QLabel(description);
		desc->setWordWrap(true);
		body->addWidget(desc);
	}
}

namespace ui {

QDoubleSpinBox* doubleSpin(double min, double max, double value, int decimals, const QString& suffix)
{
	QDoubleSpinBox* s = new QDoubleSpinBox;
	s->setDecimals(decimals);
	s->setRange(min, max);
	s->setValue(value);
	s->setSuffix(suffix);
	s->setAccelerated(true);
	s->setMinimumWidth(140);
	return s;
}

QSpinBox* intSpin(int min, int max, int value)
{
	QSpinBox* s = new QSpinBox;
	s->setRange(min, max);
	s->setValue(value);
	s->setAccelerated(true);
	s->setMinimumWidth(140);
	return s;
}

QLabel* resultLabel()
{
	QLabel* l = new QLabel;
	QFont f = l->font();
	f.setBold(true);
	l->setFont(f);
	l->setTextInteractionFlags(Qt::TextSelectableByMouse);
	return l;
}

QLabel* noteLabel(const QString& text)
{
	QLabel* l = new QLabel(text);
	l->setWordWrap(true);
	l->setTextInteractionFlags(Qt::TextSelectableByMouse);
	return l;
}

QPlainTextEdit* textView()
{
	QPlainTextEdit* t = new QPlainTextEdit;
	t->setReadOnly(true);
	t->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	t->setLineWrapMode(QPlainTextEdit::NoWrap);
	return t;
}

QPlainTextEdit* sequenceEdit()
{
	QPlainTextEdit* t = new QPlainTextEdit;
	t->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	t->setMaximumHeight(140);
	return t;
}

QString num(double v)
{
	if (!std::isfinite(v)) return QStringLiteral("—");
	return QString::fromStdString(pmate::fmtNum(v));
}

QString num2(float v)
{
	return num(pmate::round2(v));
}

QWidget* filePicker(QLineEdit*& edit, const QString& caption, const QString& filter, bool directory)
{
	QWidget* w = new QWidget;
	QHBoxLayout* h = new QHBoxLayout(w);
	h->setContentsMargins(0, 0, 0, 0);
	edit = new QLineEdit;
	QPushButton* browse = new QPushButton(QObject::tr("Browse…"));
	h->addWidget(edit, 1);
	h->addWidget(browse);

	QLineEdit* e = edit;
	QObject::connect(browse, &QPushButton::clicked, w, [w, e, caption, filter, directory]() {
		QString start = e->text().isEmpty() ? lastDir() : QFileInfo(e->text()).absolutePath();
		QString path = directory
			? QFileDialog::getExistingDirectory(w, caption, start)
			: QFileDialog::getOpenFileName(w, caption, start, filter);
		if (path.isEmpty()) return;
		rememberDir(path);
		e->setText(path);
	});
	return w;
}

QString saveText(QWidget* parent, const QString& caption, const QString& suggestedName,
				 const std::string& text, const QString& filter)
{
	QString start = QFileInfo(suggestedName).isAbsolute() ? suggestedName : QDir(lastDir()).filePath(suggestedName);

	/* The dialog asks before replacing an existing file */
	QString path = QFileDialog::getSaveFileName(parent, caption, start, filter);
	if (path.isEmpty()) return QString();
	rememberDir(path);

	/* QSaveFile writes to a temporary file and renames it into place, so a planted
	   symbolic link is replaced rather than written through. Binary mode keeps the
	   bytes identical to the terminal version's output on every platform. */
	QSaveFile f(path);
	if (!f.open(QIODevice::WriteOnly) || f.write(text.data(), qint64(text.size())) != qint64(text.size()) || !f.commit())
	{
		QMessageBox::warning(parent, caption, QObject::tr("Unable to write %1:\n%2").arg(path, f.errorString()));
		return QString();
	}
	return path;
}

bool readFileBytes(QWidget* parent, const QString& path, std::string& out, qint64 maxBytes)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(parent, QObject::tr("Open file"), QObject::tr("Unable to open %1:\n%2").arg(path, f.errorString()));
		return false;
	}
	if (f.size() > maxBytes)
	{
		QMessageBox::warning(parent, QObject::tr("Open file"),
			QObject::tr("%1 is too large (limit %2 MB).").arg(path).arg(maxBytes / (1024 * 1024)));
		return false;
	}
	QByteArray data = f.readAll();
	out.assign(data.constData(), size_t(data.size()));
	return true;
}

QString loadSequenceInto(QWidget* parent, QPlainTextEdit* edit)
{
	QString path = QFileDialog::getOpenFileName(parent, QObject::tr("Open sequence file"), lastDir(),
		QObject::tr("Sequence files (*.seq *.txt);;All files (*)"));
	if (path.isEmpty()) return QString();
	rememberDir(path);

	std::string data;
	if (!readFileBytes(parent, path, data, 16 * qint64(pmate::MAX_SEQ_LEN))) return QString();
	edit->setPlainText(QString::fromUtf8(data.data(), qsizetype(data.size())));
	return path;
}

QString lastDir()
{
	QSettings s;
	return s.value(QStringLiteral("lastDir"), QDir::homePath()).toString();
}

void rememberDir(const QString& path)
{
	QFileInfo fi(path);
	QSettings s;
	s.setValue(QStringLiteral("lastDir"), fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath());
}

QString userName()
{
	QString u = qEnvironmentVariable("USER");
	if (u.isEmpty()) u = qEnvironmentVariable("USERNAME");
	return u;
}

}
