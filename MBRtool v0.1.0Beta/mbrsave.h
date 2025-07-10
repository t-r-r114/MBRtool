#ifndef MBRSAVE_H
#define MBRSAVE_H

#include <QObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <qqmlengine.h>
#include <QQmlContext>

class MBRsave : public QObject
{
    Q_OBJECT
public:
    explicit MBRsave(QObject *parent = nullptr);

    Q_INVOKABLE int mBRSAVE();

signals: 
};

#endif // MBRSAVE_H
