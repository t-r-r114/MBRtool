#ifndef DISASSEMBLYWINDOWS_H
#define DISASSEMBLYWINDOWS_H

#include <QObject>

class DisassemblyWindows : public QObject
{
    Q_OBJECT
public:
    explicit DisassemblyWindows(QObject *parent = nullptr);

    Q_INVOKABLE int disassemblyWindows();

signals:
};

#endif // DISASSEMBLYWINDOWS_H
