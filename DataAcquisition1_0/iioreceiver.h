#ifndef IIORECEIVER_H
#define IIORECEIVER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QVector>
#include <iio.h>
#include "databuffer.h"

// IIO采集工作线程
class IioWorker : public QObject
{
    Q_OBJECT
public:
    explicit IioWorker(QObject *parent = nullptr);
    ~IioWorker();

    void setConnectionParams(const QString& ipAddress, int bufferSize, int rounds);
    void setEnabledChannels(const QVector<int>& channels);
    void setSampleRate(double rate) { m_sampleRate = rate; }

public slots:
    void startAcquisition();
    void stopAcquisition();

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void dataReceived(int channel, const QVector<DataPoint>& data);
    void statusChanged(const QString& status);

private:
    bool initializeIio();
    void cleanupIio();
    bool configureChannels();
    void acquisitionLoop();

    QString m_ipAddress;
    int m_bufferSize;
    int m_rounds;
    QVector<int> m_enabledChannels;
    double m_sampleRate;
    bool m_running;

    struct iio_context* m_ctx;
    struct iio_device* m_adc0;
    struct iio_device* m_adc1;
    struct iio_buffer* m_buf0;
    struct iio_buffer* m_buf1;

    QMutex m_mutex;
};

// IIO接收器主类
class IioReceiver : public QObject
{
    Q_OBJECT

public:
    explicit IioReceiver(DataBuffer* buffer, QObject *parent = nullptr);
    ~IioReceiver();

    bool connectToDevice(const QString& ipAddress);
    void disconnectFromDevice();

    bool isConnected() const { return m_isConnected; }
    QString getConnectionInfo() const { return m_connectionInfo; }

    void setSampleRate(double rate);
    double getSampleRate() const { return m_sampleRate; }

    void setEnabledChannels(const QVector<int>& channels);

    // 配置参数
    void setBufferSize(int size) { m_bufferSize = size; }
    void setRounds(int rounds) { m_rounds = rounds; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void dataReceived(int channel, int pointCount);
    void statusChanged(const QString& status);

private slots:
    void onWorkerConnected();
    void onWorkerDisconnected();
    void onWorkerError(const QString& error);
    void onWorkerDataReceived(int channel, const QVector<DataPoint>& data);

private:
    DataBuffer* m_dataBuffer;
    IioWorker* m_worker;
    QThread* m_workerThread;

    bool m_isConnected;
    QString m_connectionInfo;
    double m_sampleRate;
    int m_bufferSize;
    int m_rounds;
    QVector<int> m_enabledChannels;
};

#endif // IIORECEIVER_H
