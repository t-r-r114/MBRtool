#include "ntwiter.h"
#include <windows.h>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <shellapi.h>
#include <QFileInfo>
#include "Public.h"

ntWiter::ntWiter(QObject *parent)
    : QObject{parent}, m_hDevice(INVALID_HANDLE_VALUE), m_lastError(0)
{
    qDebug() << "[ntWiter] 实例化: ntWiter 对象已创建。";
}

bool ntWiter::executeSafeWrite(quint32 diskNumber, quint64 offset)
{
    qInfo() << QString("[ntWiter] executeSafeWrite 被调用: 目标磁盘号 DiskNumber=%1, 偏移量 Offset=%2").arg(diskNumber).arg(offset);

    // 1. 弹出 Windows 原生高危确认弹窗 (保留后端弹窗机制)
    qDebug() << "[ntWiter] 准备弹出高危操作确认弹窗...";
    QString warnMsg = QString("警告：您即将通过内核驱动对物理磁盘 %1 进行直接扇区写入！\n\n"
                              "此操作极度危险且不可逆！即将把备份的 MBR 数据写入该磁盘。\n"
                              "如果您选择错了磁盘编号，可能导致系统蓝屏或数据永久损毁。\n\n"
                              "您确定要继续执行写入操作吗？").arg(diskNumber);

    int msgBoxRet = MessageBoxW(
        NULL,
        (LPCWSTR)warnMsg.utf16(),
        L"MBRtool - 危险操作确认",
        MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_SYSTEMMODAL
        );

    if (msgBoxRet != IDYES) {
        m_lastError = ERROR_CANCELLED; // 1223: 操作被用户取消
        qWarning() << "[ntWiter] 操作已取消: 用户在弹窗中拒绝了高危写入操作。";
        return false;
    }
    qInfo() << "[ntWiter] 用户已确认高危写入操作，继续执行。";

    // ========== 2. 读取 mbr_backup_diskX.bin 文件 ==========

    // 获取程序运行的当前目录 (即 .exe 所在的文件夹)
    QString exeDir = QCoreApplication::applicationDirPath();
    QString binDir = exeDir + QDir::separator() + "bin";

    // 动态拼接带有磁盘号的文件名，与 mbrsave.cpp 保持一致
    QString fileName = QString("mbr_backup_disk%1.bin").arg(diskNumber);
    QString backupFilePath = exeDir + QDir::separator() + fileName;

    qDebug() << "[ntWiter] 正在解析备份文件路径:" << backupFilePath;

    QFile backupFile(backupFilePath);

    // 尝试以只读模式打开文件
    if (!backupFile.open(QIODevice::ReadOnly)) {
        m_lastError = ERROR_FILE_NOT_FOUND; // 2: 系统找不到指定的文件
        qCritical() << "[ntWiter] 严重错误: 无法打开备份文件。路径:" << backupFilePath << "系统错误码:" << m_lastError;

        MessageBoxW(NULL, L"无法找到对应的 MBR 备份文件，请先执行备份操作！", L"写入失败", MB_ICONERROR | MB_OK);
        return false;
    }

    // MBR 固定为 512 字节，我们只读取前 512 字节
    qDebug() << "[ntWiter] 文件打开成功，准备读取前 512 字节数据...";
    QByteArray dataToWrite = backupFile.read(512);
    backupFile.close();

    // 校验文件大小
    if (dataToWrite.size() != 512) {
        m_lastError = ERROR_INVALID_DATA; // 13: 数据无效
        qCritical() << "[ntWiter] 严重错误: 备份文件大小错误！预期 512 字节，实际读取:" << dataToWrite.size() << "字节。操作中止。";
        MessageBoxW(NULL, L"备份文件损坏或大小异常（非512字节），写入已中止！", L"数据异常", MB_ICONERROR | MB_OK);
        return false;
    }
    qInfo() << "[ntWiter] 文件读取成功，数据校验通过 (大小 512 字节)。";

    // ========== 3. 开始调用底层驱动写入 ==========

    qDebug() << "[ntWiter] 准备打开底层驱动设备...";
    if (!openDevice()) {
        qCritical() << "[ntWiter] 驱动设备打开失败，写入流程终止。错误码:" << m_lastError;
        return false;
    }

    // 调用底层驱动写入方法
    qDebug() << "[ntWiter] 准备调用 writeToDisk 执行底层写入...";
    bool result = writeToDisk(diskNumber, offset, dataToWrite);

    if (result) {
        qInfo() << "[ntWiter] executeSafeWrite 执行完毕: MBR 数据恢复成功！";
    } else {
        qCritical() << "[ntWiter] executeSafeWrite 执行失败: 底层写入失败，错误码:" << m_lastError;
    }

    // 关闭驱动句柄
    closeDevice();

    return result;
}

ntWiter::~ntWiter()
{
    qDebug() << "[ntWiter] 析构: 准备销毁 ntWiter 对象并清理资源...";
    closeDevice();
    qDebug() << "[ntWiter] 析构完成。";
}

bool ntWiter::openDevice()
{
    qDebug() << "[ntWiter] openDevice() 被调用。";

    if (m_hDevice != INVALID_HANDLE_VALUE) {
        return true;
    }

    // 1. 先检查或下载依赖文件
    if (!prepareDriverEnvironment()) {
        MessageBoxW(NULL, L"严重错误：驱动运行环境准备失败，中止打开设备。", L"环境初始化失败", MB_ICONERROR | MB_OK);
        m_lastError = ERROR_FILE_NOT_FOUND;
        return false;
    }

    // 2. 环境准备完毕，文件确认存在，此时加载驱动
    QString exeDir = QCoreApplication::applicationDirPath();
    QString binDir = exeDir + QDir::separator() + "bin";

    if(!startDriver(exeDir, binDir)){
        MessageBoxW(NULL, L"驱动加载脚本触发失败，请检查是否被杀毒软件拦截！", L"环境初始化失败", MB_ICONERROR | MB_OK);
        // 如果驱动加载失败，就没必要往下走拿句柄了
        return false;
    }

    // 3. 轮询等待驱动加载完成 (覆盖 UAC 确认用户操作的时间以及脚本执行时间)
    qInfo() << "[ntWiter] 脚本已触发，正在等待用户通过 UAC 验证及驱动底层就绪...";

    HANDLE hDevice = INVALID_HANDLE_VALUE;
    const int maxRetries = 30; // 尝试 30 次
    const int sleepMs = 500;   // 每次间隔 500 毫秒 (总计最长容忍 15 秒)

    for (int i = 0; i < maxRetries; ++i) {
        hDevice = CreateFileW(
            L"\\\\.\\MyWriter",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (hDevice != INVALID_HANDLE_VALUE) {
            qDebug() << "[ntWiter] 驱动符号链接已暴露！连接成功。等待耗时约为:" << i * sleepMs << "ms";
            break; // 成功获取句柄，跳出循环
        }

        // 每次等待期间让出 CPU 供 Qt 处理 UI 事件，防止主界面卡死变白
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        Sleep(sleepMs);
    }

    // 4. 检查最终等待结果
    if (hDevice == INVALID_HANDLE_VALUE) {
        m_lastError = GetLastError();
        qCritical() << "[ntWiter] CreateFileW 严重失败！等待 15 秒后依然无法获取驱动句柄。GetLastError=" << m_lastError;

        MessageBoxW(NULL,
                    L"连接驱动超时！\n\n可能的原因：\n1. 您未在弹出的 UAC 窗口中点击“是”\n2. 杀毒软件拦截了内核驱动的加载\n3. 系统存在不兼容问题",
                    L"驱动连接超时", MB_ICONERROR | MB_OK);

        return false;
    }

    m_hDevice = hDevice;
    m_lastError = 0;
    qInfo() << "[ntWiter] 驱动设备打开成功，句柄:" << m_hDevice;
    return true;
}

bool ntWiter::prepareDriverEnvironment()
{
    QString exeDir = QCoreApplication::applicationDirPath();
    QString binDir = exeDir + QDir::separator() + "bin";
    QDir dir(binDir);

    // 定义需要检查的关键依赖文件路径
    QString sysFilePath = binDir + QDir::separator() + "Writer.sys";
    QString kduFilePath = binDir + QDir::separator() + "kdu.exe";

    // 1. 检查 bin 目录是否存在，并且核心驱动文件和提权工具是否完整
    if (dir.exists() && QFile::exists(sysFilePath) && QFile::exists(kduFilePath)) {
        qDebug() << "[ntWiter] bin 目录及核心依赖文件已存在，跳过下载环节。";
        return true;
    }

    qInfo() << "[ntWiter] 未检测到 bin 目录或关键依赖文件缺失，准备自动创建并下载驱动依赖文件...";

    // 2. 创建 bin 目录 (即使目录已存在，mkpath 也会静默返回 true，不会报错)
    if (!dir.mkpath(".")) {
        qCritical() << "[ntWiter] 无法创建 bin 目录:" << binDir;
        return false;
    }

    // 3. 定义下载 URL 与本地路径 (保持原样)
    QString sysUrl = "https://github.com/t-r-r114/binForMBRTool/releases/download/x64-Windows/kdu-bin.zip";
    QString kduUrl = "https://github.com/t-r-r114/binForMBRTool/releases/download/x64-Windows/sys-bin.zip";

    QString sysZipPath = binDir + QDir::separator() + "sys-bin.zip";
    QString kduZipPath = binDir + QDir::separator() + "kdu-bin.zip";

    // 4. 执行同步下载
    qInfo() << "[ntWiter] 正在下载 sys-bin.zip ...";
    if (!downloadFile(sysUrl, sysZipPath)) return false;

    qInfo() << "[ntWiter] 正在下载 kdu-bin.zip ...";
    if (!downloadFile(kduUrl, kduZipPath)) return false;

    // 5. 执行解压操作
    qInfo() << "[ntWiter] 下载完成，正在解压 sys-bin.zip ...";
    if (!extractZip(sysZipPath, binDir)) return false;

    qInfo() << "[ntWiter] 正在解压 kdu-bin.zip ...";
    if (!extractZip(kduZipPath, binDir)) return false;

    // 6. 清理压缩包
    qDebug() << "[ntWiter] 解压完成，正在清理遗留的压缩包...";
    QFile::remove(sysZipPath);
    QFile::remove(kduZipPath);

    qInfo() << "[ntWiter] 驱动环境准备就绪！";
    return true;
}

bool ntWiter::startDriver(QString exeDir, QString binDir){
    qInfo() << "[ntWiter] 正在生成驱动加载脚本并申请管理员权限...";

    // 生成临时的 bat 脚本
    QString batFilePath = binDir + QDir::separator() + "load_driver.bat";
    QFile batFile(batFilePath);

    if (batFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&batFile);

        out << "@echo off\n";
        out << "cd /d \"%~dp0\"\n";

        // 构造驱动绝对路径
        QString sysFilePath = binDir + QDir::separator() + "Writer.sys";
        QString nativePath = QDir::toNativeSeparators(sysFilePath);

        out << "kdu.exe -dse 0\n";

        // === 优化后的服务状态判断逻辑 ===
        out << "echo [2/5] 检查驱动服务状态...\n";
        out << "sc query MyWriteDriver >nul 2>&1\n";

        // 如果 sc query 返回错误，说明服务不存在，需要创建
        out << "if %errorlevel% neq 0 (\n";
        out << "    echo [3/5] 未检测到服务，正在注册驱动服务...\n";
        out << "    sc create MyWriteDriver type= kernel binPath= \"" << nativePath << "\" >nul\n";
        out << ") else (\n";
        out << "    echo [3/5] 服务已存在，跳过注册步骤。\n";
        out << ")\n";

        out << "echo [4/5] 正在拉起驱动服务...\n";
        // 无论服务是否已经在运行，直接执行 start。
        // 如果已运行会提示错误，但这不影响驱动工作，我们可以把输出重定向到 nul 保持清爽
        out << "sc start MyWriteDriver >nul 2>&1\n";

        out << "echo [5/5] 恢复 DSE 状态...\n";
        out << "kdu.exe -dse 6\n";

        out << "echo 驱动加载流程完成。\n";
        out << "timeout /t 3\n";
        out << "exit\n";

        batFile.close();
    } else {
        qCritical() << "[ntWiter] 无法创建驱动加载脚本！";
        return false;
    }

    // 将 Qt 的路径格式转换为 Windows 原生格式
    QString batFileNative = QDir::toNativeSeparators(batFilePath);
    QString binDirNative = QDir::toNativeSeparators(binDir);

    // 调用 Windows API 申请提权并运行这个 bat 脚本
    HINSTANCE hInst = ShellExecuteW(
        NULL,
        L"runas",                      // 核心：请求管理员权限
        (LPCWSTR)batFileNative.utf16(),// 执行刚刚生成的 bat 脚本
        NULL,                          // 参数写在 bat 里了，这里不需要传
        (LPCWSTR)binDirNative.utf16(), // 工作目录设置为 bin 文件夹
        SW_SHOWNORMAL                  // 如果想完全隐藏黑窗口，改成 SW_HIDE
        );

    // 检查执行结果
    if ((INT_PTR)hInst <= 32) {
        qCritical() << "[ntWiter] 提权运行脚本失败！用户可能拒绝了 UAC。";
        return false;
    }

    qInfo() << "[ntWiter] 管理员脚本已成功触发执行。";
    return true;
}

bool ntWiter::downloadFile(const QString &urlStr, const QString &savePath)
{
    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(urlStr)));

    // 允许跳转（针对部分下载链接可能会 302 重定向）
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = manager.get(request);

    // 使用局部的事件循环将异步下载强制转化为同步，以便在写入文件前阻塞当前线程
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // === 保留: 网络错误拦截 ===
    if (reply->error() != QNetworkReply::NoError) {
        QString msgNetErr = QString("【网络下载失败】\n\n错误详情: %1\n网络状态码: %2\n目标URL: %3")
                                .arg(reply->errorString())
                                .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                                .arg(urlStr);
        MessageBoxW(NULL, (LPCWSTR)msgNetErr.utf16(), L"下载失败 - 网络异常", MB_ICONERROR | MB_SYSTEMMODAL | MB_OK);

        qCritical() << "[ntWiter] 下载失败:" << reply->errorString() << "URL:" << urlStr;
        reply->deleteLater();
        return false;
    }

    QFile file(savePath);

    // === 保留: 文件系统错误拦截 ===
    if (!file.open(QIODevice::WriteOnly)) {
        QString msgFileErr = QString("【文件写入失败】\n\n无法在以下路径创建或打开文件！\n请检查该路径所在盘符是否有写入权限，或目录是否存在。\n\n目标路径: %1").arg(savePath);
        MessageBoxW(NULL, (LPCWSTR)msgFileErr.utf16(), L"写入失败 - I/O异常", MB_ICONERROR | MB_SYSTEMMODAL | MB_OK);

        qCritical() << "[ntWiter] 无法保存下载的文件到:" << savePath;
        reply->deleteLater();
        return false;
    }

    // 将数据读取出来缓存到变量中，方便后续计算大小
    QByteArray downloadedData = reply->readAll();
    file.write(downloadedData);
    file.close();
    reply->deleteLater();

    return true;
}

bool ntWiter::extractZip(const QString &zipPath, const QString &destDir)
{
    // 利用 Windows 10/11 内置的 tar 命令进行解压 (Build 17063 以上支持)
    // 如果需要兼容老版本的 Win7，请考虑使用第三方解压工具，如 7z.exe 或 QtQuaZip
    QProcess process;
    process.setProgram("C:\\Windows\\System32\\tar.exe");

    // 参数说明: -x (解压) -f (指定文件) -C (指定输出目录)
    QStringList args;
    args << "-xf" << QDir::toNativeSeparators(zipPath) << "-C" << QDir::toNativeSeparators(destDir);
    process.setArguments(args);

    qDebug() << "[ntWiter] 执行解压命令: tar" << args.join(" ");

    process.start();
    if (!process.waitForFinished(60000)) { // 设定60秒超时
        qCritical() << "[ntWiter] 解压超时！";
        process.kill();
        return false;
    }

    if (process.exitCode() != 0) {
        qCritical() << "[ntWiter] 解压失败！退出码:" << process.exitCode() << "标准错误:" << process.readAllStandardError();
        return false;
    }

    return true;
}

void ntWiter::closeDevice()
{
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        qDebug() << "[ntWiter] 正在关闭驱动设备句柄:" << m_hDevice;
        CloseHandle((HANDLE)m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
        qDebug() << "[ntWiter] 驱动设备句柄已关闭。";
    }
}

bool ntWiter::writeToDisk(quint32 diskNumber, quint64 offset, const QByteArray &data)
{
    qInfo() << QString("[ntWiter] writeToDisk() 被调用: DiskNumber=%1, Offset=%2, DataSize=%3 bytes")
                   .arg(diskNumber).arg(offset).arg(data.size());

    if (m_hDevice == INVALID_HANDLE_VALUE) {
        m_lastError = ERROR_INVALID_HANDLE;
        qCritical() << "[ntWiter] writeToDisk 失败: 当前驱动设备句柄无效 (INVALID_HANDLE_VALUE)。";
        return false;
    }

    // 虽然现在驱动支持变长，但对于 MBR 写入，我们在 R3 依然严格拦截，防止业务出错
    if (data.size() != 512) {
        m_lastError = ERROR_INVALID_PARAMETER;
        qCritical() << "[ntWiter] writeToDisk 失败: 数据长度必须严格为 512 字节，当前为:" << data.size();
        return false;
    }

    qDebug() << "[ntWiter] 正在动态分配内存构造变长参数...";

    // 核心修改：动态计算总内存大小 (结构体固定头部大小 + 实际数据大小)
    size_t headerSize = FIELD_OFFSET(DISK_WRITE_PARAMS, Data);
    size_t totalBufferSize = headerSize + data.size();

    // 使用 malloc 申请连续内存
    PDISK_WRITE_PARAMS params = (PDISK_WRITE_PARAMS)malloc(totalBufferSize);
    if (!params) {
        m_lastError = ERROR_NOT_ENOUGH_MEMORY;
        qCritical() << "[ntWiter] writeToDisk 失败: 内存分配失败！";
        return false;
    }

    // 填充驱动交互结构体
    params->DiskNumber = diskNumber;
    params->ByteOffset = offset;
    params->WriteLength = static_cast<ULONG>(data.size());

    // 将实际数据拷贝到结构体末尾的变长数组区域
    memcpy(params->Data, data.constData(), data.size());

    DWORD bytesReturned = 0;
    qDebug() << "[ntWiter] 向驱动发送 DeviceIoControl 指令 (IOCTL_DISK_WRITE_COMMAND)...";

    // 注意：这里的第四个参数传入的是动态计算出的 totalBufferSize
    BOOL result = DeviceIoControl(
        (HANDLE)m_hDevice,
        IOCTL_DISK_WRITE_COMMAND,
        params,
        static_cast<DWORD>(totalBufferSize),
        NULL,
        0,
        &bytesReturned,
        NULL
        );

    // 释放内存，防止内存泄漏
    free(params);

    if (!result) {
        m_lastError = GetLastError();
        qCritical() << "[ntWiter] DeviceIoControl 调用失败！驱动处理异常。GetLastError=" << m_lastError;
        return false;
    }

    m_lastError = 0;
    qInfo() << "[ntWiter] DeviceIoControl 调用成功。驱动已确认收到写入指令。";
    return true;
}

quint32 ntWiter::getLastError() const
{
    return m_lastError;
}
