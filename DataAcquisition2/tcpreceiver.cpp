#include "tcpreceiver.h"
#include <QDebug>
#include <QHostAddress>

TcpReceiver::TcpReceiver(DataBuffer* buffer, QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_socket(nullptr)
    , m_dataBuffer(buffer)
    , m_sampleRate(1000.0)
    , m_isServer(false)
{
}

TcpReceiver::~TcpReceiver()
{
    stopServer();
    disconnectFromHost();
}

bool TcpReceiver::startServer(quint16 port)
{
    if (m_server) {
        stopServer();
    }

    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection,
            this, &TcpReceiver::onNewConnection);

    if (!m_server->listen(QHostAddress::Any, port)) {
        QString error = QString("无法启动服务器: %1").arg(m_server->errorString());
        emit errorOccurred(error);
        delete m_server;
        m_server = nullptr;
        return false;
    }

    m_isServer = true;
    emit statusChanged(QString("服务器监听端口 %1").arg(port));
    qDebug() << "TCP服务器启动，监听端口:" << port;
    return true;
}

void TcpReceiver::stopServer()
{
    if (m_server) {
        if (m_socket) {
            m_socket->disconnectFromHost();
            m_socket->deleteLater();
            m_socket = nullptr;
        }
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
        emit statusChanged("服务器已停止");
    }
}

bool TcpReceiver::connectToHost(const QString& host, quint16 port)
{
    if (m_socket) {
        disconnectFromHost();
    }

    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected,
            this, &TcpReceiver::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TcpReceiver::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TcpReceiver::onReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &TcpReceiver::onError);
#else
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &TcpReceiver::onError);
#endif

    m_isServer = false;
    m_socket->connectToHost(host, port);

    emit statusChanged(QString("正在连接到 %1:%2").arg(host).arg(port));
    qDebug() << "正在连接到:" << host << ":" << port;
    return true;
}

void TcpReceiver::disconnectFromHost()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
        emit statusChanged("已断开连接");
    }
}

bool TcpReceiver::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

QString TcpReceiver::getConnectionInfo() const
{
    if (!m_socket || !isConnected()) {
        return "未连接";
    }

    if (m_isServer) {
        return QString("%1:%2").arg(m_socket->peerAddress().toString())
        .arg(m_socket->peerPort());
    } else {
        return QString("%1:%2").arg(m_socket->peerAddress().toString())
        .arg(m_socket->peerPort());
    }
}

void TcpReceiver::onNewConnection()
{
    if (!m_server) return;

    QTcpSocket* clientSocket = m_server->nextPendingConnection();

    if (m_socket) {
        // 已有连接，拒绝新连接
        qDebug() << "拒绝新连接，已有活动连接";
        clientSocket->disconnectFromHost();
        clientSocket->deleteLater();
        return;
    }

    m_socket = clientSocket;

    connect(m_socket, &QTcpSocket::connected,
            this, &TcpReceiver::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TcpReceiver::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TcpReceiver::onReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &TcpReceiver::onError);
#else
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &TcpReceiver::onError);
#endif

    qDebug() << "客户端已连接:" << m_socket->peerAddress().toString()
             << ":" << m_socket->peerPort();
    emit connected();
}

void TcpReceiver::onConnected()
{
    qDebug() << "TCP连接已建立";
    emit connected();
    emit statusChanged("已连接");
}

void TcpReceiver::onDisconnected()
{
    qDebug() << "TCP连接已断开";
    m_receiveBuffer.clear();

    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    emit disconnected();
    emit statusChanged("连接已断开");
}

void TcpReceiver::onReadyRead()
{
    if (!m_socket) return;

    // 读取所有可用数据
    QByteArray data = m_socket->readAll();
    m_receiveBuffer.append(data);

    // 处理接收到的数据
    processReceivedData();
}

void TcpReceiver::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);

    if (!m_socket) return;

    QString errorString = m_socket->errorString();
    qDebug() << "TCP错误:" << errorString;
    emit errorOccurred(errorString);
}

void TcpReceiver::processReceivedData()
{
    const int headerSize = sizeof(DataPacketHeader);

    while (m_receiveBuffer.size() >= headerSize) {
        // 解析包头
        DataPacketHeader* header = reinterpret_cast<DataPacketHeader*>(
            m_receiveBuffer.data());

        // 验证魔数
        if (header->magic != 0x44415441) { // "DATA"
            qWarning() << "无效的数据包魔数";
            // 尝试查找下一个有效的魔数
            bool found = false;
            for (int i = 1; i < m_receiveBuffer.size() - 3; ++i) {
                quint32* magic = reinterpret_cast<quint32*>(
                    m_receiveBuffer.data() + i);
                if (*magic == 0x44415441) {
                    m_receiveBuffer.remove(0, i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                m_receiveBuffer.clear();
            }
            continue;
        }

        // 计算完整数据包大小
        int dataSize = header->pointCount * sizeof(double);
        int packetSize = headerSize + dataSize;

        // 检查是否接收到完整数据包
        if (m_receiveBuffer.size() < packetSize) {
            break; // 等待更多数据
        }

        // 提取完整数据包
        QByteArray packet = m_receiveBuffer.left(packetSize);
        m_receiveBuffer.remove(0, packetSize);

        // 解析数据包
        parseDataPacket(packet);
    }
}

void TcpReceiver::parseDataPacket(const QByteArray& data)
{
    if (data.size() < static_cast<int>(sizeof(DataPacketHeader))) {
        return;
    }

    const DataPacketHeader* header = reinterpret_cast<const DataPacketHeader*>(
        data.constData());

    int channel = header->channel;
    quint32 pointCount = header->pointCount;
    double startTime = header->startTime;
    double sampleRate = header->sampleRate;

    // 验证通道号
    if (channel < 1 || channel > 2) {
        qWarning() << "无效的通道号:" << channel;
        return;
    }

    // 解析数据点
    const double* amplitudes = reinterpret_cast<const double*>(
        data.constData() + sizeof(DataPacketHeader));

    QVector<DataPoint> points;
    points.reserve(pointCount);

    for (quint32 i = 0; i < pointCount; ++i) {
        DataPoint point;
        point.time = startTime + i / sampleRate;
        point.amplitude = amplitudes[i];
        points.append(point);
    }

    // 添加到数据缓冲区
    if (m_dataBuffer) {
        m_dataBuffer->addDataPoints(channel, points);
    }

    emit dataReceived(channel, pointCount);

    qDebug() << "接收数据包 - 通道:" << channel
             << "点数:" << pointCount
             << "起始时间:" << startTime;
}
