#ifndef MBRWRITE_H
#define MBRWRITE_H

#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqmlengine.h>
#include <QQmlContext>

class MBRwrite : public QObject
{
    Q_OBJECT
public:
    explicit MBRwrite(QObject *parent = nullptr);

    Q_INVOKABLE int mBRWRITE();

signals:
};

#endif // MBRWRITE_H
