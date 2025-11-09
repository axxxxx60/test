#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVector>
#include "databuffer.h"

// 任务信息结构
struct TaskInfo {
    int taskId;
    QString taskName;
    double sampleRate;
    double duration;
    int channelCount;      // 支持最多13个通道
    QString createTime;
    QString description;
    QVector<int> enabledChannels;  // 启用的通道列表

    TaskInfo() : taskId(-1), sampleRate(0), duration(0), channelCount(13) {}
};

// 分析结果结构
struct AnalysisResult {
    int analysisId;
    int taskId;
    int channel;
    double maxAmplitude;
    double minAmplitude;
    double avgAmplitude;
    double rmsValue;
    double frequency;
    QString analysisTime;

    AnalysisResult() : analysisId(-1), taskId(-1), channel(0),
        maxAmplitude(0), minAmplitude(0), avgAmplitude(0),
        rmsValue(0), frequency(0) {}
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    // 连接数据库
    bool connectToDatabase(const QString& host, int port,
                           const QString& dbName,
                           const QString& user,
                           const QString& password);
    void disconnectFromDatabase();
    bool isConnected() const;

    // 任务操作
    int createTask(const TaskInfo& info);
    bool updateTask(const TaskInfo& info);
    bool deleteTask(int taskId);
    TaskInfo getTask(int taskId);
    QVector<TaskInfo> getAllTasks();
    QVector<TaskInfo> searchTasks(const QString& keyword);

    // 数据保存 - 支持多通道
    bool saveRawData(int taskId, int channel, const QVector<DataPoint>& data);
    bool saveProcessedCoordinates(int taskId, int channel,
                                  const QVector<DataPoint>& data);

    // 批量保存多通道数据
    bool saveMultiChannelData(int taskId, const QVector<QVector<DataPoint>>& channelData,
                              const QVector<int>& channelIndices);

    // 数据读取
    QVector<DataPoint> loadRawData(int taskId, int channel);
    QVector<DataPoint> loadProcessedData(int taskId, int channel);

    // 加载多通道数据
    QVector<QVector<DataPoint>> loadMultiChannelData(int taskId,
                                                     const QVector<int>& channels);

    // 分析结果
    bool saveAnalysisResult(const AnalysisResult& result);
    QVector<AnalysisResult> getAnalysisResults(int taskId);
    QVector<AnalysisResult> getChannelAnalysisResults(int taskId, int channel);

    // 批量操作
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    QString getLastError() const { return m_lastError; }

signals:
    void progressUpdated(int percentage, const QString& message);
    void operationCompleted(bool success, const QString& message);

private:
    bool executeQuery(QSqlQuery& query);
    bool createTables();

    QSqlDatabase m_database;
    QString m_lastError;
    bool m_isConnected;
};

#endif // DATABASEMANAGER_H
