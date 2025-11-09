#ifndef TCPRECEIVER_H
#define TCPRECEIVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QByteArray>
#include "databuffer.h"
#include <iio.h>

// 数据包头结构
#pragma pack(push, 1)
struct DataPacketHeader {
    quint32 magic;        // 魔数，用于验证数据包：0x44415441 ("DATA")
    quint32 channel;      // 通道号：1或2
    quint32 pointCount;   // 本包中的数据点数量
    double sampleRate;    // 采样率
    double startTime;     // 起始时间
};
#pragma pack(pop)

class TcpReceiver : public QObject
{
    Q_OBJECT
    
public:
    explicit TcpReceiver(DataBuffer* buffer, QObject *parent = nullptr);
    ~TcpReceiver();
    
    // 作为服务器监听连接
    bool startServer(quint16 port);
    void stopServer();
    
    // 作为客户端连接到服务器
    bool connectToHost(const QString& host, quint16 port);
    void disconnectFromHost();
    
    bool isConnected() const;
    QString getConnectionInfo() const;
    
    // 设置采样率（用于计算时间）
    void setSampleRate(double rate) { m_sampleRate = rate; }
    double getSampleRate() const { return m_sampleRate; }
    
signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void dataReceived(int channel, int pointCount);
    void statusChanged(const QString& status);
    
private slots:
    void onNewConnection();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    
private:
    void processReceivedData();
    void parseDataPacket(const QByteArray& data);
    
    QTcpServer* m_server;
    QTcpSocket* m_socket;
    DataBuffer* m_dataBuffer;
    QByteArray m_receiveBuffer;
    
    double m_sampleRate;
    bool m_isServer;
};

#endif // TCPRECEIVER_H
