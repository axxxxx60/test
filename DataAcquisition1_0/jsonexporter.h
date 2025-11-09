#ifndef JSONEXPORTER_H
#define JSONEXPORTER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "databuffer.h"
#include "databasemanager.h"

class JsonExporter : public QObject
{
    Q_OBJECT

public:
    explicit JsonExporter(QObject *parent = nullptr);
    ~JsonExporter();

    // 导出单通道数据到JSON
    bool exportChannelToJson(const QString& filePath,
                             int channel,
                             const QVector<DataPoint>& data);

    // 导出多通道数据到JSON（参照iio_1.py格式）
    bool exportMultiChannelToJson(const QString& filePath,
                                  const QVector<int>& channels,
                                  const QVector<QVector<DataPoint>>& channelData);

    // 从任务信息导出
    bool exportTaskToJson(const QString& filePath,
                          const TaskInfo& taskInfo,
                          const QVector<QVector<DataPoint>>& channelData);

    QString getLastError() const { return m_lastError; }

signals:
    void exportProgress(int percentage, const QString& message);
    void exportCompleted(bool success, const QString& message);

private:
    QString m_lastError;

    // 将DataPoint数组转换为JSON数组（只保存幅值）
    QJsonArray convertToJsonArray(const QVector<DataPoint>& data);
};

#endif // JSONEXPORTER_H
