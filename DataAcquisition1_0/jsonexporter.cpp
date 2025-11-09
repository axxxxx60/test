#include "jsonexporter.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

JsonExporter::JsonExporter(QObject *parent)
    : QObject(parent)
{
}

JsonExporter::~JsonExporter()
{
}

bool JsonExporter::exportChannelToJson(const QString& filePath,
                                       int channel,
                                       const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        m_lastError = "数据为空";
        return false;
    }

    emit exportProgress(10, "准备导出单通道数据...");

    QJsonObject root;
    root[QString::number(channel)] = convertToJsonArray(data);

    emit exportProgress(50, "生成JSON文档...");

    QJsonDocument doc(root);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    emit exportProgress(70, "写入文件...");

    // 确保目录存在
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开文件: %1").arg(file.errorString());
        return false;
    }

    qint64 written = file.write(jsonData);
    file.close();

    if (written < 0) {
        m_lastError = "写入文件失败";
        return false;
    }

    emit exportProgress(100, "导出完成");
    qDebug() << "JSON文件已保存到:" << filePath;
    return true;
}

bool JsonExporter::exportMultiChannelToJson(const QString& filePath,
                                            const QVector<int>& channels,
                                            const QVector<QVector<DataPoint>>& channelData)
{
    if (channels.size() != channelData.size()) {
        m_lastError = "通道数量与数据数量不匹配";
        return false;
    }

    if (channels.isEmpty()) {
        m_lastError = "没有数据要导出";
        return false;
    }

    emit exportProgress(10, "准备导出多通道数据...");

    // 创建JSON对象，格式参照iio_1.py
    QJsonObject root;

    int totalChannels = channels.size();
    for (int i = 0; i < totalChannels; ++i) {
        int channelIndex = channels[i];
        const QVector<DataPoint>& data = channelData[i];

        if (!data.isEmpty()) {
            // 使用通道索引作为键（字符串格式）
            root[QString::number(channelIndex)] = convertToJsonArray(data);
        }

        int progress = 10 + (i * 60 / totalChannels);
        emit exportProgress(progress,
                            QString("处理通道 %1...").arg(channelIndex));
    }

    emit exportProgress(70, "生成JSON文档...");

    QJsonDocument doc(root);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    emit exportProgress(80, "写入文件...");

    // 确保目录存在
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开文件: %1").arg(file.errorString());
        return false;
    }

    qint64 written = file.write(jsonData);
    file.close();

    if (written < 0) {
        m_lastError = "写入文件失败";
        return false;
    }

    emit exportProgress(100, "导出完成");

    qDebug() << "多通道JSON文件已保存到:" << filePath;
    qDebug() << "文件大小:" << jsonData.size() << "字节";
    qDebug() << "包含通道数:" << channels.size();

    emit exportCompleted(true, QString("成功导出 %1 个通道的数据").arg(channels.size()));
    return true;
}

bool JsonExporter::exportTaskToJson(const QString& filePath,
                                    const TaskInfo& taskInfo,
                                    const QVector<QVector<DataPoint>>& channelData)
{
    return exportMultiChannelToJson(filePath, taskInfo.enabledChannels, channelData);
}

QJsonArray JsonExporter::convertToJsonArray(const QVector<DataPoint>& data)
{
    QJsonArray array;

    // 只保存幅值数据，与iio_1.py的格式保持一致
    for (const auto& point : data) {
        array.append(point.amplitude);
    }

    return array;
}
