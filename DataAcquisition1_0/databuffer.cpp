#include "databuffer.h"
#include <QDebug>

DataBuffer::DataBuffer(QObject *parent)
    : QObject(parent)
    , m_maxCapacity(100000)  // 默认最大容量10万个数据点
{
}

DataBuffer::~DataBuffer()
{
}

void DataBuffer::addDataPoint(int channel, const DataPoint& point)
{
    if (channel < 0 || channel >= MAX_CHANNELS) {
        qWarning() << "无效的通道号:" << channel;
        return;
    }

    QMutexLocker locker(&m_mutex);

    m_channelData[channel].append(point);

    // 检查是否超过最大容量
    if (m_channelData[channel].size() > m_maxCapacity) {
        // 移除最旧的数据点
        m_channelData[channel].remove(0, m_channelData[channel].size() - m_maxCapacity);
        emit bufferFull(channel);
    }

    emit dataAdded(channel);
}

void DataBuffer::addDataPoints(int channel, const QVector<DataPoint>& points)
{
    if (points.isEmpty()) {
        return;
    }

    if (channel < 0 || channel >= MAX_CHANNELS) {
        qWarning() << "无效的通道号:" << channel;
        return;
    }

    QMutexLocker locker(&m_mutex);

    m_channelData[channel].append(points);

    // 检查是否超过最大容量
    if (m_channelData[channel].size() > m_maxCapacity) {
        // 移除最旧的数据点
        int removeCount = m_channelData[channel].size() - m_maxCapacity;
        m_channelData[channel].remove(0, removeCount);
        emit bufferFull(channel);
    }

    emit dataAdded(channel);
}

QVector<DataPoint> DataBuffer::getChannelData(int channel, int maxPoints)
{
    if (channel < 0 || channel >= MAX_CHANNELS) {
        qWarning() << "无效的通道号:" << channel;
        return QVector<DataPoint>();
    }

    QMutexLocker locker(&m_mutex);

    QVector<DataPoint> data;

    if (maxPoints < 0 || maxPoints >= m_channelData[channel].size()) {
        data = m_channelData[channel];
    } else {
        // 返回最新的maxPoints个数据点
        int startIndex = m_channelData[channel].size() - maxPoints;
        data = m_channelData[channel].mid(startIndex, maxPoints);
    }

    return data;
}

QVector<DataPoint> DataBuffer::getAllChannelData(int channel)
{
    return getChannelData(channel, -1);
}

void DataBuffer::clear()
{
    QMutexLocker locker(&m_mutex);

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        m_channelData[i].clear();
    }

    qDebug() << "数据缓冲区已清空";
}

void DataBuffer::clearChannel(int channel)
{
    if (channel < 0 || channel >= MAX_CHANNELS) {
        qWarning() << "无效的通道号:" << channel;
        return;
    }

    QMutexLocker locker(&m_mutex);

    m_channelData[channel].clear();
    qDebug() << "通道" << channel << "数据已清空";
}

int DataBuffer::getDataCount(int channel) const
{
    if (channel < 0 || channel >= MAX_CHANNELS) {
        qWarning() << "无效的通道号:" << channel;
        return 0;
    }

    QMutexLocker locker(&m_mutex);
    return m_channelData[channel].size();
}

void DataBuffer::setMaxCapacity(int capacity)
{
    QMutexLocker locker(&m_mutex);

    if (capacity <= 0) {
        qWarning() << "无效的缓冲区容量:" << capacity;
        return;
    }

    m_maxCapacity = capacity;

    // 如果当前数据量超过新容量，进行裁剪
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        if (m_channelData[i].size() > m_maxCapacity) {
            int removeCount = m_channelData[i].size() - m_maxCapacity;
            m_channelData[i].remove(0, removeCount);
        }
    }

    qDebug() << "缓冲区最大容量设置为:" << m_maxCapacity;
}

QVector<QVector<DataPoint>> DataBuffer::getAllChannelsData()
{
    QMutexLocker locker(&m_mutex);

    QVector<QVector<DataPoint>> allData;
    allData.reserve(MAX_CHANNELS);

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        allData.append(m_channelData[i]);
    }

    return allData;
}
