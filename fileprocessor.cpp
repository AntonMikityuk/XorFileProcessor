#include "fileprocessor.h"
#include "qregularexpression.h"
#include <QDebug>
#include <QThread>

FileProcessor::FileProcessor(QObject *parent) : QObject(parent) {}

void FileProcessor::requestPause() { m_isPaused = true; }
void FileProcessor::requestResume() {
    m_isPaused = false;
    m_pauseCondition.wakeAll();
}
void FileProcessor::requestStop() {
    m_isStopped = true;
    requestResume();
}

void FileProcessor::process() {
    m_isStopped = false;
    m_isPaused = false;

    QDir inputDir(m_settings.inputPath);

    if (!inputDir.exists()) {
        emit statusMessage("Error, input path does not exist");
        emit finished();
        return;
    }

    QDir outputDir(m_settings.outputPath);

    if (!outputDir.exists()) {
        emit statusMessage("Error: output path does not exist");
        emit finished();
        return;
    }

    QStringList filters = m_settings.mask.split(QRegularExpression("[,; ]+"), Qt::SkipEmptyParts);
    QFileInfoList files = inputDir.entryInfoList(filters, QDir::Files);

    if (files.isEmpty()) {
        emit statusMessage("Files not found");
        emit finished();
        return;
    }

    for (const QFileInfo &fileInfo : files) {
        if (m_isStopped) break;
        processSingleFile(fileInfo.absoluteFilePath());
    }

    emit statusMessage("Processing completed");
    emit finished();
}

void FileProcessor::processSingleFile(const QString &filePath) {
    QFile inFile(filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        emit statusMessage("Error opening: " + inFile.fileName());
        return;
    }

    QString outDir = m_settings.outputPath;
    QString fileName = QFileInfo(filePath).fileName();
    QString outPath = outDir + "/" + fileName;

    QString actualOutPath = outPath;
    bool isSameFile = (QFileInfo(filePath).absoluteFilePath() == QFileInfo(outPath).absoluteFilePath());
    bool fileExistsAtDest = QFile::exists(outPath);
    bool usingTemp = false;

    if (m_settings.overwrite) {
        if (fileExistsAtDest) {
            actualOutPath = outPath + ".tmp";
            usingTemp = true;
        }
    }   else {
            if (fileExistsAtDest) {
                actualOutPath = getUniqueFileName(outPath);
            }
        }

    QFile outFile(actualOutPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        emit statusMessage("Error writing: " + actualOutPath);
        inFile.close();
        return;
    }

    emit statusMessage("Processing: " + QFileInfo(filePath).fileName());

    qint64 fileSize = inFile.size();
    qint64 processedSize = 0;
    quint64 key = m_settings.xorKey;

    while (processedSize < fileSize && !m_isStopped) {
        m_pauseMutex.lock();
        if (m_isPaused) {
            emit statusMessage("Paused");
            m_pauseCondition.wait(&m_pauseMutex);
            if (!m_isStopped) emit statusMessage("Processing");
        }
        m_pauseMutex.unlock();

        if (m_isStopped) break;

        QByteArray buffer = inFile.read(BUFFER_SIZE);
        if (buffer.isEmpty()) break;

        // XOR
        uchar *data = reinterpret_cast<uchar*>(buffer.data());
        int len = buffer.size();
        int i = 0;

        uint8_t keyBytes[8];
        std::memcpy(keyBytes, &key, 8);

        for (; i <= len - 8; i += 8) {
            uint64_t chunk;
            std::memcpy(&chunk, data + i, 8);
            chunk ^= key;
            std::memcpy(data + i, &chunk, 8);
        }

        for (; i < len; ++i) {
            data[i] ^= keyBytes[i % 8];
        }

        outFile.write(buffer);
        processedSize += buffer.size();
        emit progressFile(static_cast<int>((processedSize * 100) / fileSize));
    }

    inFile.close();
    outFile.close();

    if (m_isStopped) {
        // Если была остановка, то удаляем созданный файл
        if (actualOutPath != filePath) {
            QFile::remove(actualOutPath);
        }
    } else {
        if (usingTemp) {
            // Если использовали временный файл
            QFile::remove(outPath);
            QFile::rename(actualOutPath, outPath);
        }

        // Если другая папка и нужно удалить оригинал
        if (m_settings.deleteInput && !isSameFile) {
            QFile::remove(filePath);
        }
    }
}

QString FileProcessor::getUniqueFileName(const QString &fullPath) {
    QFileInfo info(fullPath);
    QString path = info.absolutePath();
    QString baseName = info.baseName();
    QString ext = info.completeSuffix();
    int counter = 1;
    QString newPath = fullPath;
    while (QFile::exists(newPath)) {
        newPath = QString("%1/%2_%3.%4").arg(path, baseName, QString::number(counter++), ext);
    }
    return newPath;
}