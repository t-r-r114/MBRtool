#include "FileReader.h"
#include <QFile>
#include <QTextStream>
#include <QUrl>

FileReader::FileReader(QObject *parent) : QObject(parent) {}

QString FileReader::fileContent() const
{
    return m_fileContent;
}

void FileReader::loadFile(const QString &filePath){
    QUrl url(filePath);
    QString localPath = url.isLocalFile() ? url.toLocalFile() : filePath; // 转换 URL 为本地路径

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_fileContent = in.readAll();  // 读取文件内容
        file.close();
        emit fileContentChanged();  // 内容更改，通知 QML 更新
    } else {
        m_fileContent = "Failed to open file!";
        emit fileContentChanged();  // 如果文件打开失败，通知 QML
    }
}
