#include "mainwindow.h"
#include <QApplication>

QScopedPointer<QFile> m_logFile;
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void logfilecreate(void);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    logfilecreate();

//  Set handler
    qInstallMessageHandler(messageHandler);

    MainWindow w;
    w.show();

    return a.exec();
}


void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Open stream file writes
    QTextStream out(m_logFile.data());

    // Write the date of recording
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

    // By type determine to what level belongs message
#if 1
    switch (type)
    {
        case QtInfoMsg:     out << "INF "; break;
        case QtDebugMsg:    out << "DBG "; break;
        case QtWarningMsg:  out << "WRN "; break;
        case QtCriticalMsg: out << "CRT "; break;
        case QtFatalMsg:    out << "FTL "; break;
    }
#endif

    // Write to the output category of the message and the message itself
    out << context.category << ": "
          << msg << endl;
    out.flush();    // Clear the buffered data
}

void logfilecreate(void)
{
    //Log Message Process
    QString logfolderpath = ("/home/pi/LogMsg/");
    QString logworking_date = QDateTime::currentDateTime().toString("yyyyMMdd");

    QDir fdir(logfolderpath);

    if(fdir.exists() == false)
    {
        bool result = fdir.mkdir(logfolderpath);
        Log() << "log directory = " << result;
    }

    QDir fdir_date(logfolderpath + logworking_date);

    if(fdir_date.exists() == false)
    {
        bool result = fdir_date.mkdir(logfolderpath + logworking_date + "/");
        Log() << " directory = " << result;
    }

    QString  logfilepath = logfolderpath + logworking_date + "/" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+ ".log";

    // Set the logging file
    // check which a path to file you use
    m_logFile.reset(new QFile(logfilepath));

    // Open the file logging
    m_logFile.data()->open(QFile::Append | QFile::Text);

    // Set handler
    qInstallMessageHandler(messageHandler);
}
