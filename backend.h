#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include "fileprocessor.h"

class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(bool isPaused READ isPaused NOTIFY isPausedChanged)

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();

    Q_INVOKABLE void start(QString inputPath, QString outputPath, QString mask,
                           QString hexKey, bool deleteInput, bool overwrite,
                           bool useTimer, int intervalSec);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();

    double progress() const { return m_progress; }
    QString status() const { return m_status; }
    bool isRunning() const { return m_isRunning; }
    bool isBusy() const { return m_isBusy; }
    bool isPaused() const { return m_isPaused; }

signals:
    void progressChanged();
    void statusChanged();
    void isRunningChanged();
    void isBusyChanged();
    void isPausedChanged();

private slots:
    void onWorkerProgress(int val);
    void onWorkerStatus(QString msg);
    void onWorkerFinished();
    void onTimerTimeout();

private:
    void runProcessor();
    QString urlToPath(QString url);

    FileProcessor *m_worker = nullptr;
    QThread *m_thread = nullptr;
    QTimer *m_timer = nullptr;

    FileProcessor::Settings m_currentSettings;

    double m_progress = 0;
    QString m_status = "Ready";
    bool m_isRunning = false;
    bool m_isBusy = false;
    bool m_isPaused = false;
};

#endif // BACKEND_H