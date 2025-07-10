#include "HexEditor.h"
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QVariantList>
#include <QVariantMap>


HexEditor::HexEditor(QObject *parent) : QObject(parent) {
    readConfig();
}

QVariantList HexEditor::getHexLines() {
    QVariantList lines;
    const int bytesPerLine = 16;

    for (int i = 0; i < m_data.size(); i += bytesPerLine) {
        QVariantMap line;
        QString address = QString("%1").arg(i, 8, 16, QChar('0')).toUpper();
        QString hexStr;

        for (int j = 0; j < bytesPerLine && (i + j) < m_data.size(); ++j) {
            hexStr += QString("%1 ").arg(static_cast<unsigned char>(m_data[i + j]), 2, 16, QChar('0')).toUpper();
        }

        line["address"] = address;
        line["hexData"] = hexStr.trimmed();
        lines.append(line);
    }

    return lines;
}

void HexEditor::readConfig() {
    QSettings settings("HexEdit.ini", QSettings::IniFormat);
    m_filePath = settings.value("FilePath", "./mbr_backup.bin").toString();

    if (!QFile::exists("HexEdit.ini")) {
        createDefaultConfig();
    }
}

void HexEditor::createDefaultConfig() {
    QSettings settings("HexEdit.ini", QSettings::IniFormat);
    settings.setValue("FilePath", "./mbr_backup.bin");
}

QString HexEditor::getFilePath() {
    return m_filePath;
}

void HexEditor::loadFile() {
    printf("load file\n");
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << m_filePath;
        return;
    }

    m_data = file.readAll();
    emit hexContentChanged();
    file.close();
}

void HexEditor::saveFile(const QString &hexText) {
    QByteArray data = hexToByteArray(hexText);
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to write file:" << m_filePath;
        return;
    }

    file.write(data);
    file.close();
    qDebug() << "File saved.";
}

QString HexEditor::hexContent() const {
    return byteArrayToHex(m_data);
}

QString HexEditor::byteArrayToHex(const QByteArray &data) const {
    QString result;
    for (int i = 0; i < data.size(); ++i) {
        result += QString("%1 ").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
        if ((i + 1) % 16 == 0) result += '\n';
    }
    return result;
}

QByteArray HexEditor::hexToByteArray(const QString &hexStr) const {
    QByteArray result;
    QStringList byteStrings = hexStr.simplified().split(" ");
    for (const QString &byteStr : byteStrings) {
        bool ok;
        char byte = static_cast<char>(byteStr.toUInt(&ok, 16));
        if (ok) result.append(byte);
    }
    return result;
}
