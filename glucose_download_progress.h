#ifndef GLUCOSE_DOWNLOAD_PROGRESS_H
#define GLUCOSE_DOWNLOAD_PROGRESS_H

#include <QObject>
#include <QDateTime>

class GlucoseDownloadProgress : public QObject
{
    Q_OBJECT
    int numberOfGluecose;
    int currentIndex;
    Sp::CommProtocolType selectedProtocol;
    int progressTimerID;

public:
    GlucoseDownloadProgress(QObject *parent = 0) : QObject(parent)
    {
        numberOfGluecose = 0;
        currentIndex = 0;
        selectedProtocol = Sp::CommProtocol1;
        progressTimerID = 0;
    }
    ~GlucoseDownloadProgress() {}

    void setNumberOfGluecose(int nog)
    {
        numberOfGluecose = nog; currentIndex = 0;
        emit downloadProgress(progress());
    }

    void setOnlyNumberOfGluecose(int nog)
    {
        numberOfGluecose = nog;
    }

    void setIndexOfGlucose(int iog)
    {
        currentIndex = iog;
    }

    unsigned int downloadableCount()
    {
        if(selectedProtocol == Sp::CommProtocol3)
        {   //data unit number = 27
            return (currentIndex >= numberOfGluecose) ? 0 : (numberOfGluecose - currentIndex > 27) ? 27 : (numberOfGluecose - currentIndex);
        }
        else
        {   //data unit number = 10
            return (currentIndex >= numberOfGluecose) ? 0 : (numberOfGluecose - currentIndex > 10) ? 10 : (numberOfGluecose - currentIndex);
        }
    }

    void setProcotol(Sp::CommProtocolType protocol)
    {
         selectedProtocol = protocol;
    }

    void setDownloadedCount(int cnt)
    {
        currentIndex += cnt;

        if(progressTimerID == 0)
        {
            progressTimerID = startTimer(500);
            emit downloadProgress(progress());
        }
    }

    float progress() {

        if(numberOfGluecose != 0)
        {
            return float(currentIndex)/float(numberOfGluecose);
        }
        else
        {
            return 0;
        }
    }

    int index()
    {
        if(selectedProtocol == Sp::CommProtocol3)
        {
            return currentIndex + 1;
        }
        else
        {
            return currentIndex;
        }
    }

    int getNunberOfGluecose()
    {
        return numberOfGluecose;
    }

    int getCurrentIndex()
    {
        return currentIndex;
    }

signals:
    void downloadProgress(float progress);                                  // 0~1 : 1 = 100%

protected:
    void timerEvent(QTimerEvent *event)
    {
        Q_UNUSED(event);
        emit downloadProgress(progress());

        if(progress() >= 0.999) {
            killTimer(progressTimerID); progressTimerID = 0;
        }
    }
};

#endif // GLUCOSE_DOWNLOAD_PROGRESS_H
