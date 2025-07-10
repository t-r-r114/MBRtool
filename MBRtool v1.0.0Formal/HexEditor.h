#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QSettings>

class HexEditor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString hexContent READ hexContent NOTIFY hexContentChanged)
public:
    explicit HexEditor(QObject *parent = nullptr);

    Q_INVOKABLE void loadFile();
    Q_INVOKABLE void saveFile(const QString &hexText);
    Q_INVOKABLE QString getFilePath();
    Q_INVOKABLE QVariantList getHexLines();

    QString hexContent() const;

signals:
    void hexContentChanged();

private:
    QString m_filePath;
    QByteArray m_data;
    QString byteArrayToHex(const QByteArray &data) const;
    QByteArray hexToByteArray(const QString &hexStr) const;
    void readConfig();
    void createDefaultConfig();
};
