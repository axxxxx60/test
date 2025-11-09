#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QVector>
#include <QThread>
#include "databuffer.h"
#include "databasemanager.h"

class DataProcessor : public QObject
{
    Q_OBJECT
    
public:
    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor();
    
    // 数据滤波
    QVector<DataPoint> applyLowPassFilter(const QVector<DataPoint>& data, 
                                          double cutoffFreq, double sampleRate);
    QVector<DataPoint> applyHighPassFilter(const QVector<DataPoint>& data,
                                           double cutoffFreq, double sampleRate);
    
    // 数据降采样
    QVector<DataPoint> downsample(const QVector<DataPoint>& data, int factor);
    
    // 数据平滑
    QVector<DataPoint> movingAverageSmooth(const QVector<DataPoint>& data, 
                                           int windowSize);
    
    // 坐标转换（归一化等）
    QVector<DataPoint> normalizeData(const QVector<DataPoint>& data);
    QVector<DataPoint> scaleData(const QVector<DataPoint>& data, 
                                 double scaleX, double scaleY);
    
signals:
    void processingProgress(int percentage, const QString& message);
    void processingCompleted(bool success);
    
private:
    double calculateAlpha(double cutoffFreq, double sampleRate);
};

#endif // DATAPROCESSOR_H
