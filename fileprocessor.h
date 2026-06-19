#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMutex>
#include <QWaitCondition>
#include <QAtomicInt>

class FileProcessor : public QObject
{
    Q_OBJECT
public:
    explicit FileProcessor(QObject *parent = nullptr);

    struct Settings {
        QString inputPath;
        QString outputPath;
        QString mask;
        quint64 xorKey;
        bool deleteInput;
        bool overwrite;
        bool workByTimer;
        int timerIntervalMs;
    };

    void setSettings(const Settings &s) { m_settings = s; }

public slots:
    void process();
    void requestPause();
    void requestResume();
    void requestStop();

signals:
    void progressFile(int value);
    void statusMessage(QString msg);
    void finished();

private:
    void processSingleFile(const QString &filePath);
    QString getUniqueFileName(const QString &fullPath);

    Settings m_settings;

    std::atomic<bool> m_isPaused{false};
    std::atomic<bool> m_isStopped{false};

    QMutex m_pauseMutex;
    QWaitCondition m_pauseCondition;

    const qint64 BUFFER_SIZE = 1024 * 1024;
};

#endif // FILEPROCESSOR_H