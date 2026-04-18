#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include <QString>

class FileReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileContent READ fileContent NOTIFY fileContentChanged)

public:
    explicit FileReader(QObject *parent = nullptr);

    QString fileContent() const;

    Q_INVOKABLE void loadFile(const QString &filePath);

signals:
    void fileContentChanged();

private:
    QString m_fileContent;
};

#endif // FILEREADER_H
