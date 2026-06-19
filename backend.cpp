#include "backend.h"

Backend::Backend(QObject *parent) : QObject(parent) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Backend::onTimerTimeout);
}

Backend::~Backend() {
    stop();
}

QString Backend::urlToPath(QString url) {
    if (url.startsWith("file:///")) {
#ifdef Q_OS_WIN
        return url.mid(8);
#else
        return url.mid(7);
#endif
    }
    return url;
}

void Backend::start(QString inputPath, QString outputPath, QString mask,
                    QString hexKey, bool deleteInput, bool overwrite,
                    bool useTimer, int intervalSec)
{
    m_progress = 0;
    emit progressChanged();

    m_status = "Processing";
    emit statusChanged();

    m_currentSettings.inputPath = urlToPath(inputPath);
    m_currentSettings.outputPath = urlToPath(outputPath);
    m_currentSettings.mask = mask;
    m_currentSettings.xorKey = hexKey.toULongLong(nullptr, 16);
    m_currentSettings.deleteInput = deleteInput;
    m_currentSettings.overwrite = overwrite;

    m_isBusy = true;
    emit isBusyChanged();

    if (useTimer) {
        m_timer->start(intervalSec * 1000);
        m_status = "Timer activated";
        emit statusChanged();
        runProcessor();
    } else {
        runProcessor();
    }
}

void Backend::runProcessor() {
    if (m_isRunning) return;

    m_thread = new QThread();
    m_worker = new FileProcessor();
    m_worker->setSettings(m_currentSettings);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &FileProcessor::process);
    connect(m_worker, &FileProcessor::progressFile, this, &Backend::onWorkerProgress);
    connect(m_worker, &FileProcessor::statusMessage, this, &Backend::onWorkerStatus);
    connect(m_worker, &FileProcessor::finished, this, &Backend::onWorkerFinished);

    connect(m_worker, &QObject::destroyed, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_thread->start();
    m_isRunning = true;
    emit isRunningChanged();
}

void Backend::pause() {
    if (m_worker) {
        m_worker->requestPause();
        m_isPaused = true;
        emit isPausedChanged();
        m_status = "Paused";
        emit statusChanged();
    }
}

void Backend::resume() {
    if (m_worker) {
        m_worker->requestResume();
        m_isPaused = false;
        emit isPausedChanged();
        m_status = "Processing";
        emit statusChanged();
    }
}

void Backend::stop() {
    m_timer->stop();
    if (m_worker) {
        m_worker->requestStop();
    }
    else {
        m_isBusy = false;
        m_isRunning = false;
        m_isPaused = false;
        m_progress = 0;

        emit isBusyChanged();
        emit isRunningChanged();
        emit isPausedChanged();
        emit progressChanged();
    }

    m_status = "Stopped";
    emit statusChanged();
}

void Backend::onWorkerProgress(int val) { m_progress = val; emit progressChanged(); }
void Backend::onWorkerStatus(QString msg) { m_status = msg; emit statusChanged(); }
void Backend::onWorkerFinished() {
    if (m_worker) {
        m_worker->deleteLater();
        m_worker = nullptr;
    }
    m_thread = nullptr;

    if (m_status != "Stopped" && !m_status.startsWith("Error")) {
        m_status = "Finished";
        emit statusChanged();
    }

    m_progress = 0;
    emit progressChanged();

    m_isRunning = false;
    m_isPaused = false;
    emit isRunningChanged();
    emit isPausedChanged();

    if (!m_timer->isActive()) {
        m_isBusy = false;
        emit isBusyChanged();
    }
}
void Backend::onTimerTimeout() { runProcessor(); }