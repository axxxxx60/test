#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <QObject>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

#define MAX_CHANNELS 13

// 单个数据点结构
struct DataPoint {
    double time;      // 时间值（秒）
    double amplitude; // 幅值

    DataPoint() : time(0.0), amplitude(0.0) {}
    DataPoint(double t, double a) : time(t), amplitude(a) {}
};

// 数据缓冲区类 - 支持13个通道
class DataBuffer : public QObject
{
    Q_OBJECT

public:
    explicit DataBuffer(QObject *parent = nullptr);
    ~DataBuffer();

    // 添加数据点
    void addDataPoint(int channel, const DataPoint& point);
    void addDataPoints(int channel, const QVector<DataPoint>& points);

    // 获取数据
    QVector<DataPoint> getChannelData(int channel, int maxPoints = -1);
    QVector<DataPoint> getAllChannelData(int channel);

    // 清空缓冲区
    void clear();
    void clearChannel(int channel);

    // 获取数据点数量
    int getDataCount(int channel) const;

    // 设置缓冲区最大容量
    void setMaxCapacity(int capacity);
    int getMaxCapacity() const { return m_maxCapacity; }

    // 获取所有通道的数据
    QVector<QVector<DataPoint>> getAllChannelsData();

signals:
    void dataAdded(int channel);
    void bufferFull(int channel);

private:
    QVector<DataPoint> m_channelData[MAX_CHANNELS];
    mutable QMutex m_mutex;
    int m_maxCapacity;  // 最大缓冲容量
};

#endif // DATABUFFER_H
