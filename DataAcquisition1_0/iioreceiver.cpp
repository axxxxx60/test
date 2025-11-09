#include "iioreceiver.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>

// ==================== IioWorker Implementation ====================

IioWorker::IioWorker(QObject *parent)
    : QObject(parent)
    , m_bufferSize(2560)
    , m_rounds(5)  // 这个参数在持续接收模式下不再使用
    , m_sampleRate(1000.0)
    , m_running(false)
    , m_ctx(nullptr)
    , m_adc0(nullptr)
    , m_adc1(nullptr)
    , m_buf0(nullptr)
    , m_buf1(nullptr)
{
}

IioWorker::~IioWorker()
{
    stopAcquisition();
    cleanupIio();
}

void IioWorker::setConnectionParams(const QString& ipAddress, int bufferSize, int rounds)
{
    QMutexLocker locker(&m_mutex);
    m_ipAddress = ipAddress;
    m_bufferSize = bufferSize;
    m_rounds = rounds;  // 保留参数但不使用
}

void IioWorker::setEnabledChannels(const QVector<int>& channels)
{
    QMutexLocker locker(&m_mutex);
    m_enabledChannels = channels;
}

void IioWorker::startAcquisition()
{
    if (m_running) {
        qWarning() << "采集已在运行中";
        return;
    }

    if (!initializeIio()) {
        emit errorOccurred("初始化IIO失败");
        return;
    }

    if (!configureChannels()) {
        emit errorOccurred("配置通道失败");
        cleanupIio();
        return;
    }

    m_running = true;
    emit connected();
    emit statusChanged("IIO设备已连接,开始持续采集");

    // 开始持续采集循环
    acquisitionLoop();
}

void IioWorker::stopAcquisition()
{
    m_running = false;
    emit statusChanged("停止采集");
}

bool IioWorker::initializeIio()
{
    QString uri = "ip:" + m_ipAddress;
    qDebug() << "正在连接到:" << uri;

    m_ctx = iio_create_context_from_uri(uri.toUtf8().constData());
    if (!m_ctx) {
        qWarning() << "无法创建IIO上下文";
        return false;
    }

    // 查找ADC设备
    m_adc0 = iio_context_find_device(m_ctx, "cf_axi_adc");
    m_adc1 = iio_context_find_device(m_ctx, "cf_axi_adc_1");

    if (!m_adc0 || !m_adc1) {
        qWarning() << "无法找到ADC设备";
        return false;
    }

    qDebug() << "IIO设备初始化成功";
    return true;
}

void IioWorker::cleanupIio()
{
    if (m_buf0) {
        iio_buffer_destroy(m_buf0);
        m_buf0 = nullptr;
    }

    if (m_buf1) {
        iio_buffer_destroy(m_buf1);
        m_buf1 = nullptr;
    }

    if (m_ctx) {
        iio_context_destroy(m_ctx);
        m_ctx = nullptr;
    }

    m_adc0 = nullptr;
    m_adc1 = nullptr;

    qDebug() << "IIO资源已清理";
}

bool IioWorker::configureChannels()
{
    QMutexLocker locker(&m_mutex);

    // 配置ADC0的通道 (通道0-7)
    unsigned int adc0_count = iio_device_get_channels_count(m_adc0);
    for (unsigned int i = 0; i < adc0_count && i < 8; ++i) {
        struct iio_channel *chn = iio_device_get_channel(m_adc0, i);
        if (chn && iio_channel_is_scan_element(chn)) {
            bool enable = m_enabledChannels.contains(i);
            if (enable) {
                iio_channel_enable(chn);
                qDebug() << "启用ADC0通道" << i;
            } else {
                iio_channel_disable(chn);
            }
        }
    }

    // 配置ADC1的通道 (通道8-12, 对应IIO通道0-4)
    unsigned int adc1_count = iio_device_get_channels_count(m_adc1);
    for (unsigned int i = 0; i < adc1_count && i < 5; ++i) {
        struct iio_channel *chn = iio_device_get_channel(m_adc1, i);
        if (chn && iio_channel_is_scan_element(chn)) {
            bool enable = m_enabledChannels.contains(i + 8);
            if (enable) {
                iio_channel_enable(chn);
                qDebug() << "启用ADC1通道" << (i + 8);
            } else {
                iio_channel_disable(chn);
            }
        }
    }

    // 创建缓冲区
    m_buf0 = iio_device_create_buffer(m_adc0, m_bufferSize, false);
    m_buf1 = iio_device_create_buffer(m_adc1, m_bufferSize, false);

    if (!m_buf0 || !m_buf1) {
        qWarning() << "无法创建IIO缓冲区";
        return false;
    }

    qDebug() << "通道配置完成";
    return true;
}

void IioWorker::acquisitionLoop()
{
    qDebug() << "开始持续采集循环";

    // 获取scale值用于数据转换
    struct iio_channel *ch0 = iio_device_get_channel(m_adc0, 0);
    const char *scale_str = nullptr;
    double scale = 1.0;

    if (ch0) {
        iio_channel_attr_read(ch0, "scale", (char*)&scale_str, 32);
        if (scale_str) {
            scale = QString(scale_str).toDouble();
        }
    }

    double timeStep = 1.0 / m_sampleRate;
    double currentTime = 0.0;  // 累计时间
    int cycleCount = 0;  // 循环计数器

    // 持续采集循环 - 只要m_running为true就一直运行
    while (m_running) {
        QVector<QVector<int32_t>> allChannelData(13);

        // 采集ADC0数据
        ssize_t ret0 = iio_buffer_refill(m_buf0);
        if (ret0 < 0) {
            qWarning() << "ADC0缓冲区刷新失败:" << ret0;
            emit errorOccurred("ADC0数据读取失败");
            m_running = false;
            break;
        }

        // 采集ADC1数据
        ssize_t ret1 = iio_buffer_refill(m_buf1);
        if (ret1 < 0) {
            qWarning() << "ADC1缓冲区刷新失败:" << ret1;
            emit errorOccurred("ADC1数据读取失败");
            m_running = false;
            break;
        }

        // 解析ADC0数据 (通道0-7)
        void *buf0_start = iio_buffer_start(m_buf0);
        ptrdiff_t buf0_step = iio_buffer_step(m_buf0);

        for (int i = 0; i < m_bufferSize; ++i) {
            uint8_t *sample = (uint8_t*)buf0_start + i * buf0_step;

            for (int ch = 0; ch < 8; ++ch) {
                if (m_enabledChannels.contains(ch)) {
                    struct iio_channel *chn = iio_device_get_channel(m_adc0, ch);
                    if (chn && iio_channel_is_enabled(chn)) {
                        uint32_t raw_value = *(uint32_t*)(sample + ch * 4);
                        int32_t value = (int32_t)(raw_value << 8) >> 8; // 符号扩展
                        allChannelData[ch].append(value);
                    }
                }
            }
        }

        // 解析ADC1数据 (通道8-12)
        void *buf1_start = iio_buffer_start(m_buf1);
        ptrdiff_t buf1_step = iio_buffer_step(m_buf1);

        for (int i = 0; i < m_bufferSize; ++i) {
            uint8_t *sample = (uint8_t*)buf1_start + i * buf1_step;

            for (int ch = 0; ch < 5; ++ch) {
                int globalCh = ch + 8;
                if (m_enabledChannels.contains(globalCh)) {
                    struct iio_channel *chn = iio_device_get_channel(m_adc1, ch);
                    if (chn && iio_channel_is_enabled(chn)) {
                        uint32_t raw_value = *(uint32_t*)(sample + ch * 4);
                        int32_t value = (int32_t)(raw_value << 8) >> 8;
                        allChannelData[globalCh].append(value);
                    }
                }
            }
        }

        // 转换并发送数据
        if (m_running) {
            for (int ch : m_enabledChannels) {
                if (!allChannelData[ch].isEmpty()) {
                    QVector<DataPoint> points;
                    points.reserve(allChannelData[ch].size());

                    for (int i = 0; i < allChannelData[ch].size(); ++i) {
                        DataPoint point;
                        point.time = currentTime + i * timeStep;
                        point.amplitude = allChannelData[ch][i] * scale;
                        points.append(point);
                    }

                    emit dataReceived(ch, points);
                }
            }

            // 更新累计时间
            currentTime += m_bufferSize * timeStep;
            cycleCount++;

            // 每100个周期更新一次状态
            if (cycleCount % 100 == 0) {
                emit statusChanged(QString("持续采集中 - 已采集 %1 秒数据").arg(currentTime, 0, 'f', 2));
            }
        }

        // 短暂休息避免CPU占用过高
        QThread::msleep(1);
    }

    emit disconnected();
    emit statusChanged("采集循环已停止");
}

// ==================== IioReceiver Implementation ====================

IioReceiver::IioReceiver(DataBuffer* buffer, QObject *parent)
    : QObject(parent)
    , m_dataBuffer(buffer)
    , m_worker(nullptr)
    , m_workerThread(nullptr)
    , m_isConnected(false)
    , m_sampleRate(1000.0)
    , m_bufferSize(2560)
    , m_rounds(5)  // 这个参数在持续接收模式下不再使用
{
    m_enabledChannels << 0 << 1; // 默认启用前两个通道
}

IioReceiver::~IioReceiver()
{
    disconnectFromDevice();
}

bool IioReceiver::connectToDevice(const QString& ipAddress)
{
    if (m_isConnected) {
        qWarning() << "已经连接到设备";
        return false;
    }

    // 创建工作线程
    m_workerThread = new QThread(this);
    m_worker = new IioWorker();
    m_worker->moveToThread(m_workerThread);

    // 连接信号
    connect(m_workerThread, &QThread::started, m_worker, &IioWorker::startAcquisition);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    connect(m_worker, &IioWorker::connected, this, &IioReceiver::onWorkerConnected);
    connect(m_worker, &IioWorker::disconnected, this, &IioReceiver::onWorkerDisconnected);
    connect(m_worker, &IioWorker::errorOccurred, this, &IioReceiver::onWorkerError);
    connect(m_worker, &IioWorker::dataReceived, this, &IioReceiver::onWorkerDataReceived);
    connect(m_worker, &IioWorker::statusChanged, this, &IioReceiver::statusChanged);

    // 设置参数
    m_worker->setConnectionParams(ipAddress, m_bufferSize, m_rounds);
    m_worker->setEnabledChannels(m_enabledChannels);
    m_worker->setSampleRate(m_sampleRate);

    m_connectionInfo = ipAddress;

    // 启动线程
    m_workerThread->start();

    qDebug() << "开始连接到IIO设备:" << ipAddress;
    return true;
}

void IioReceiver::disconnectFromDevice()
{
    if (!m_isConnected && !m_workerThread) {
        return;
    }

    if (m_worker) {
        m_worker->stopAcquisition();
    }

    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait(3000);

        if (m_workerThread->isRunning()) {
            m_workerThread->terminate();
            m_workerThread->wait();
        }

        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }

    m_worker = nullptr;
    m_isConnected = false;
    m_connectionInfo.clear();

    qDebug() << "IIO设备已断开";
}

void IioReceiver::setSampleRate(double rate)
{
    m_sampleRate = rate;
    if (m_worker) {
        m_worker->setSampleRate(rate);
    }
}

void IioReceiver::setEnabledChannels(const QVector<int>& channels)
{
    m_enabledChannels = channels;
    if (m_worker) {
        m_worker->setEnabledChannels(channels);
    }

    qDebug() << "已设置启用通道:" << m_enabledChannels;
}

void IioReceiver::onWorkerConnected()
{
    m_isConnected = true;
    emit connected();
    qDebug() << "IIO工作线程已连接";
}

void IioReceiver::onWorkerDisconnected()
{
    m_isConnected = false;
    emit disconnected();
    qDebug() << "IIO工作线程已断开";
}

void IioReceiver::onWorkerError(const QString& error)
{
    qWarning() << "IIO错误:" << error;
    emit errorOccurred(error);
}

void IioReceiver::onWorkerDataReceived(int channel, const QVector<DataPoint>& data)
{
    if (m_dataBuffer) {
        m_dataBuffer->addDataPoints(channel, data);
        emit dataReceived(channel, data.size());
    }
}
