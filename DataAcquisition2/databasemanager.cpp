#include "databasemanager.h"
#include <QSqlRecord>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnectFromDatabase();
}

bool DatabaseManager::connectToDatabase(const QString& host, int port,
                                        const QString& dbName,
                                        const QString& user,
                                        const QString& password)
{
    if (m_isConnected) {
        disconnectFromDatabase();
    }

    m_database = QSqlDatabase::addDatabase("QMYSQL");
    m_database.setHostName(host);
    m_database.setPort(port);
    m_database.setDatabaseName(dbName);
    m_database.setUserName(user);
    m_database.setPassword(password);

    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "数据库连接失败:" << m_lastError;
        return false;
    }

    m_isConnected = true;
    qDebug() << "数据库连接成功";

    // 创建表
    if (!createTables()) {
        m_lastError = "创建表失败";
        disconnectFromDatabase();
        return false;
    }

    return true;
}

void DatabaseManager::disconnectFromDatabase()
{
    if (m_isConnected) {
        m_database.close();
        m_isConnected = false;
        qDebug() << "数据库已断开连接";
    }
}

bool DatabaseManager::isConnected() const
{
    return m_isConnected && m_database.isOpen();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // 创建任务表
    QString createTaskTable = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            task_id INT AUTO_INCREMENT PRIMARY KEY,
            task_name VARCHAR(255) NOT NULL,
            sample_rate DOUBLE NOT NULL,
            duration DOUBLE NOT NULL,
            channel_count INT DEFAULT 2,
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            description TEXT,
            INDEX idx_task_name (task_name),
            INDEX idx_create_time (create_time)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!query.exec(createTaskTable)) {
        m_lastError = query.lastError().text();
        qWarning() << "创建任务表失败:" << m_lastError;
        return false;
    }

    // 创建原始数据表
    QString createRawDataTable = R"(
        CREATE TABLE IF NOT EXISTS raw_data (
            data_id BIGINT AUTO_INCREMENT PRIMARY KEY,
            task_id INT NOT NULL,
            channel INT NOT NULL,
            time_value DOUBLE NOT NULL,
            amplitude DOUBLE NOT NULL,
            INDEX idx_task_channel (task_id, channel),
            INDEX idx_time (time_value),
            FOREIGN KEY (task_id) REFERENCES tasks(task_id) ON DELETE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!query.exec(createRawDataTable)) {
        m_lastError = query.lastError().text();
        qWarning() << "创建原始数据表失败:" << m_lastError;
        return false;
    }

    // 创建处理后坐标数据表
    QString createProcessedTable = R"(
        CREATE TABLE IF NOT EXISTS processed_coordinates (
            coord_id BIGINT AUTO_INCREMENT PRIMARY KEY,
            task_id INT NOT NULL,
            channel INT NOT NULL,
            time_value DOUBLE NOT NULL,
            amplitude DOUBLE NOT NULL,
            INDEX idx_task_channel (task_id, channel),
            FOREIGN KEY (task_id) REFERENCES tasks(task_id) ON DELETE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!query.exec(createProcessedTable)) {
        m_lastError = query.lastError().text();
        qWarning() << "创建处理数据表失败:" << m_lastError;
        return false;
    }

    // 创建分析结果表
    QString createAnalysisTable = R"(
        CREATE TABLE IF NOT EXISTS analysis_results (
            analysis_id INT AUTO_INCREMENT PRIMARY KEY,
            task_id INT NOT NULL,
            channel INT NOT NULL,
            max_amplitude DOUBLE,
            min_amplitude DOUBLE,
            avg_amplitude DOUBLE,
            rms_value DOUBLE,
            frequency DOUBLE,
            analysis_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            INDEX idx_task (task_id),
            FOREIGN KEY (task_id) REFERENCES tasks(task_id) ON DELETE CASCADE
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
    )";

    if (!query.exec(createAnalysisTable)) {
        m_lastError = query.lastError().text();
        qWarning() << "创建分析结果表失败:" << m_lastError;
        return false;
    }

    qDebug() << "数据库表创建成功";
    return true;
}

int DatabaseManager::createTask(const TaskInfo& info)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO tasks (task_name, sample_rate, duration, "
                 "channel_count, description) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(info.taskName);
    query.addBindValue(info.sampleRate);
    query.addBindValue(info.duration);
    query.addBindValue(info.channelCount);
    query.addBindValue(info.description);

    if (!executeQuery(query)) {
        return -1;
    }

    int taskId = query.lastInsertId().toInt();
    qDebug() << "任务创建成功，ID:" << taskId;
    return taskId;
}

bool DatabaseManager::updateTask(const TaskInfo& info)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE tasks SET task_name=?, sample_rate=?, duration=?, "
                 "channel_count=?, description=? WHERE task_id=?");
    query.addBindValue(info.taskName);
    query.addBindValue(info.sampleRate);
    query.addBindValue(info.duration);
    query.addBindValue(info.channelCount);
    query.addBindValue(info.description);
    query.addBindValue(info.taskId);

    return executeQuery(query);
}

bool DatabaseManager::deleteTask(int taskId)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM tasks WHERE task_id=?");
    query.addBindValue(taskId);

    bool success = executeQuery(query);
    if (success) {
        qDebug() << "任务删除成功，ID:" << taskId;
    }
    return success;
}

TaskInfo DatabaseManager::getTask(int taskId)
{
    TaskInfo info;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE task_id=?");
    query.addBindValue(taskId);

    if (!executeQuery(query)) {
        return info;
    }

    if (query.next()) {
        info.taskId = query.value("task_id").toInt();
        info.taskName = query.value("task_name").toString();
        info.sampleRate = query.value("sample_rate").toDouble();
        info.duration = query.value("duration").toDouble();
        info.channelCount = query.value("channel_count").toInt();
        info.createTime = query.value("create_time").toString();
        info.description = query.value("description").toString();
    }

    return info;
}

QVector<TaskInfo> DatabaseManager::getAllTasks()
{
    QVector<TaskInfo> tasks;

    QSqlQuery query(m_database);
    if (!query.exec("SELECT * FROM tasks ORDER BY create_time DESC")) {
        m_lastError = query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        TaskInfo info;
        info.taskId = query.value("task_id").toInt();
        info.taskName = query.value("task_name").toString();
        info.sampleRate = query.value("sample_rate").toDouble();
        info.duration = query.value("duration").toDouble();
        info.channelCount = query.value("channel_count").toInt();
        info.createTime = query.value("create_time").toString();
        info.description = query.value("description").toString();
        tasks.append(info);
    }

    return tasks;
}

QVector<TaskInfo> DatabaseManager::searchTasks(const QString& keyword)
{
    QVector<TaskInfo> tasks;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE task_name LIKE ? OR description LIKE ? "
                 "ORDER BY create_time DESC");
    QString searchPattern = "%" + keyword + "%";
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);

    if (!executeQuery(query)) {
        return tasks;
    }

    while (query.next()) {
        TaskInfo info;
        info.taskId = query.value("task_id").toInt();
        info.taskName = query.value("task_name").toString();
        info.sampleRate = query.value("sample_rate").toDouble();
        info.duration = query.value("duration").toDouble();
        info.channelCount = query.value("channel_count").toInt();
        info.createTime = query.value("create_time").toString();
        info.description = query.value("description").toString();
        tasks.append(info);
    }

    return tasks;
}

bool DatabaseManager::saveRawData(int taskId, int channel, const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return true;
    }

    if (!beginTransaction()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO raw_data (task_id, channel, time_value, amplitude) "
                 "VALUES (?, ?, ?, ?)");

    int totalPoints = data.size();
    int batchSize = 1000;

    for (int i = 0; i < totalPoints; i += batchSize) {
        int endIndex = qMin(i + batchSize, totalPoints);

        for (int j = i; j < endIndex; ++j) {
            query.addBindValue(taskId);
            query.addBindValue(channel);
            query.addBindValue(data[j].time);
            query.addBindValue(data[j].amplitude);

            if (!query.exec()) {
                m_lastError = query.lastError().text();
                rollbackTransaction();
                return false;
            }
        }

        int percentage = (endIndex * 100) / totalPoints;
        emit progressUpdated(percentage, QString("保存原始数据: %1/%2").arg(endIndex).arg(totalPoints));
    }

    if (!commitTransaction()) {
        return false;
    }

    qDebug() << "原始数据保存成功 - 任务:" << taskId << "通道:" << channel << "点数:" << data.size();
    return true;
}

bool DatabaseManager::saveProcessedCoordinates(int taskId, int channel,
                                               const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return true;
    }

    if (!beginTransaction()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO processed_coordinates (task_id, channel, time_value, amplitude) "
                 "VALUES (?, ?, ?, ?)");

    int totalPoints = data.size();
    int batchSize = 1000;

    for (int i = 0; i < totalPoints; i += batchSize) {
        int endIndex = qMin(i + batchSize, totalPoints);

        for (int j = i; j < endIndex; ++j) {
            query.addBindValue(taskId);
            query.addBindValue(channel);
            query.addBindValue(data[j].time);
            query.addBindValue(data[j].amplitude);

            if (!query.exec()) {
                m_lastError = query.lastError().text();
                rollbackTransaction();
                return false;
            }
        }

        int percentage = (endIndex * 100) / totalPoints;
        emit progressUpdated(percentage, QString("保存处理数据: %1/%2").arg(endIndex).arg(totalPoints));
    }

    if (!commitTransaction()) {
        return false;
    }

    qDebug() << "处理数据保存成功 - 任务:" << taskId << "通道:" << channel << "点数:" << data.size();
    return true;
}

QVector<DataPoint> DatabaseManager::loadRawData(int taskId, int channel)
{
    QVector<DataPoint> data;

    QSqlQuery query(m_database);
    query.prepare("SELECT time_value, amplitude FROM raw_data "
                 "WHERE task_id=? AND channel=? ORDER BY time_value ASC");
    query.addBindValue(taskId);
    query.addBindValue(channel);

    if (!executeQuery(query)) {
        return data;
    }

    while (query.next()) {
        DataPoint point;
        point.time = query.value("time_value").toDouble();
        point.amplitude = query.value("amplitude").toDouble();
        data.append(point);
    }

    qDebug() << "原始数据加载成功 - 任务:" << taskId << "通道:" << channel << "点数:" << data.size();
    return data;
}

QVector<DataPoint> DatabaseManager::loadProcessedData(int taskId, int channel)
{
    QVector<DataPoint> data;

    QSqlQuery query(m_database);
    query.prepare("SELECT time_value, amplitude FROM processed_coordinates "
                 "WHERE task_id=? AND channel=? ORDER BY time_value ASC");
    query.addBindValue(taskId);
    query.addBindValue(channel);

    if (!executeQuery(query)) {
        return data;
    }

    while (query.next()) {
        DataPoint point;
        point.time = query.value("time_value").toDouble();
        point.amplitude = query.value("amplitude").toDouble();
        data.append(point);
    }

    qDebug() << "处理数据加载成功 - 任务:" << taskId << "通道:" << channel << "点数:" << data.size();
    return data;
}

bool DatabaseManager::saveAnalysisResult(const AnalysisResult& result)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO analysis_results "
                 "(task_id, channel, max_amplitude, min_amplitude, avg_amplitude, "
                 "rms_value, frequency) VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(result.taskId);
    query.addBindValue(result.channel);
    query.addBindValue(result.maxAmplitude);
    query.addBindValue(result.minAmplitude);
    query.addBindValue(result.avgAmplitude);
    query.addBindValue(result.rmsValue);
    query.addBindValue(result.frequency);

    bool success = executeQuery(query);
    if (success) {
        qDebug() << "分析结果保存成功 - 任务:" << result.taskId << "通道:" << result.channel;
    }
    return success;
}

QVector<AnalysisResult> DatabaseManager::getAnalysisResults(int taskId)
{
    QVector<AnalysisResult> results;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM analysis_results WHERE task_id=? "
                 "ORDER BY analysis_time DESC");
    query.addBindValue(taskId);

    if (!executeQuery(query)) {
        return results;
    }

    while (query.next()) {
        AnalysisResult result;
        result.analysisId = query.value("analysis_id").toInt();
        result.taskId = query.value("task_id").toInt();
        result.channel = query.value("channel").toInt();
        result.maxAmplitude = query.value("max_amplitude").toDouble();
        result.minAmplitude = query.value("min_amplitude").toDouble();
        result.avgAmplitude = query.value("avg_amplitude").toDouble();
        result.rmsValue = query.value("rms_value").toDouble();
        result.frequency = query.value("frequency").toDouble();
        result.analysisTime = query.value("analysis_time").toString();
        results.append(result);
    }

    return results;
}

bool DatabaseManager::beginTransaction()
{
    if (!m_database.transaction()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "开始事务失败:" << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseManager::commitTransaction()
{
    if (!m_database.commit()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "提交事务失败:" << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseManager::rollbackTransaction()
{
    if (!m_database.rollback()) {
        m_lastError = m_database.lastError().text();
        qWarning() << "回滚事务失败:" << m_lastError;
        return false;
    }
    return true;
}

bool DatabaseManager::executeQuery(QSqlQuery& query)
{
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qWarning() << "SQL执行失败:" << m_lastError;
        qWarning() << "SQL语句:" << query.lastQuery();
        return false;
    }
    return true;
}

// 在原有 databasemanager.cpp 的基础上添加以下方法

// 在文件末尾添加这些新方法：

bool DatabaseManager::saveMultiChannelData(int taskId,
                                           const QVector<QVector<DataPoint>>& channelData,
                                           const QVector<int>& channelIndices)
{
    if (channelData.size() != channelIndices.size()) {
        qWarning() << "通道数据和索引数量不匹配";
        return false;
    }

    if (!beginTransaction()) {
        return false;
    }

    for (int i = 0; i < channelData.size(); ++i) {
        int channel = channelIndices[i];
        const QVector<DataPoint>& data = channelData[i];

        if (data.isEmpty()) continue;

        QSqlQuery query(m_database);
        query.prepare("INSERT INTO raw_data (task_id, channel, time_value, amplitude) "
                      "VALUES (?, ?, ?, ?)");

        int totalPoints = data.size();
        int batchSize = 1000;

        for (int j = 0; j < totalPoints; j += batchSize) {
            int endIndex = qMin(j + batchSize, totalPoints);

            for (int k = j; k < endIndex; ++k) {
                query.addBindValue(taskId);
                query.addBindValue(channel);
                query.addBindValue(data[k].time);
                query.addBindValue(data[k].amplitude);

                if (!query.exec()) {
                    m_lastError = query.lastError().text();
                    rollbackTransaction();
                    return false;
                }
            }

            int percentage = ((i * totalPoints + endIndex) * 100) /
                             (channelData.size() * totalPoints);
            emit progressUpdated(percentage,
                                 QString("保存通道 %1 数据: %2/%3").arg(channel).arg(endIndex).arg(totalPoints));
        }
    }

    if (!commitTransaction()) {
        return false;
    }

    qDebug() << "多通道数据保存成功";
    return true;
}

QVector<QVector<DataPoint>> DatabaseManager::loadMultiChannelData(int taskId,
                                                                  const QVector<int>& channels)
{
    QVector<QVector<DataPoint>> allData;
    allData.reserve(channels.size());

    for (int channel : channels) {
        QVector<DataPoint> data = loadRawData(taskId, channel);
        allData.append(data);
    }

    return allData;
}

QVector<AnalysisResult> DatabaseManager::getChannelAnalysisResults(int taskId, int channel)
{
    QVector<AnalysisResult> results;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM analysis_results WHERE task_id=? AND channel=? "
                  "ORDER BY analysis_time DESC");
    query.addBindValue(taskId);
    query.addBindValue(channel);

    if (!executeQuery(query)) {
        return results;
    }

    while (query.next()) {
        AnalysisResult result;
        result.analysisId = query.value("analysis_id").toInt();
        result.taskId = query.value("task_id").toInt();
        result.channel = query.value("channel").toInt();
        result.maxAmplitude = query.value("max_amplitude").toDouble();
        result.minAmplitude = query.value("min_amplitude").toDouble();
        result.avgAmplitude = query.value("avg_amplitude").toDouble();
        result.rmsValue = query.value("rms_value").toDouble();
        result.frequency = query.value("frequency").toDouble();
        result.analysisTime = query.value("analysis_time").toString();
        results.append(result);
    }

    return results;
}
