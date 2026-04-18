#ifndef DISASSEMBLYWINDOWS_H
#define DISASSEMBLYWINDOWS_H

#include <QObject>

class DisassemblyWindows : public QObject
{
    Q_OBJECT
public:
    explicit DisassemblyWindows(QObject *parent = nullptr);

    Q_INVOKABLE int disassemblyWindows();

    Q_INVOKABLE void setMode(int mode);

    Q_INVOKABLE bool startDualDisassembly(const QString &inputFile);

    Q_INVOKABLE void setTargetDisk(int diskNumber);

    Q_INVOKABLE int getTargetDisk();

signals:
};

#endif // DISASSEMBLYWINDOWS_H
