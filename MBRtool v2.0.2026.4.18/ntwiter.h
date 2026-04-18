#ifndef NTWITER_H
#define NTWITER_H

#include <QObject>
#include <QByteArray>

class ntWiter : public QObject
{
    Q_OBJECT
public:
    explicit ntWiter(QObject *parent = nullptr);
    ~ntWiter();

    // 打开驱动设备句柄
    bool openDevice();

    // 关闭驱动设备句柄
    void closeDevice();

    // 向指定物理磁盘的指定偏移位置写入数据
    // diskNumber: 物理磁盘号 (例如 1 代表 Disk 1)
    // offset: 写入的字节偏移 (必须对齐扇区，通常是 512 或 4096 的倍数)
    // data: 要写入的数据 (根据 public.h，单次最大 512 字节)
    bool writeToDisk(quint32 diskNumber, quint64 offset, const QByteArray &data);

    // 获取最后的系统错误码 (用于调试)
    Q_INVOKABLE quint32 getLastError() const;

    Q_INVOKABLE bool executeSafeWrite(quint32 diskNumber, quint64 offset);

private:
    bool prepareDriverEnvironment();
    bool startDriver(QString exeDir, QString binDir);
    bool downloadFile(const QString &urlStr, const QString &savePath);
    bool extractZip(const QString &zipPath, const QString &destDir);

    void* m_hDevice; // 底层保存的 Windows HANDLE，使用 void* 避免包含 windows.h
    quint32 m_lastError;
};

#endif // NTWITER_H
