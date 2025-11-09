#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QThread>
#include <QVector>
#include <QBitArray>
#include "databuffer.h"
#include "iioreceiver.h"
#include "waveformwidget.h"
#include "databasemanager.h"
#include "dataprocessor.h"
#include "dataanalyzer.h"
#include "historyviewer.h"
#include "mainwindow_ui.h"

// 数据库工作线程
class DatabaseWorker : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseWorker(DatabaseManager* dbManager, QObject *parent = nullptr)
        : QObject(parent), m_dbManager(dbManager) {}

public slots:
    void saveTaskData(const TaskInfo& taskInfo,
                      const QVector<QVector<DataPoint>>& channelData);
    void saveAnalysisResults(const QVector<AnalysisResult>& results);

signals:
    void saveCompleted(bool success, const QString& message);
    void progressUpdated(int percentage, const QString& message);

private:
    DatabaseManager* m_dbManager;
};

// 分析工作线程
class AnalysisWorker : public QObject
{
    Q_OBJECT
public:
    explicit AnalysisWorker(DataAnalyzer* analyzer, QObject *parent = nullptr)
        : QObject(parent), m_analyzer(analyzer) {}

public slots:
    void analyzeData(const QVector<QVector<DataPoint>>& channelData,
                     int taskId, double sampleRate);

signals:
    void analysisCompleted(const QVector<AnalysisResult>& results);
    void progressUpdated(int percentage, const QString& message);

private:
    DataAnalyzer* m_analyzer;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 网络连接
    void onConnectClicked();
    void onDisconnectClicked();
    void onIioConnected();
    void onIioDisconnected();
    void onIioError(const QString& error);

    // 数据采集
    void onStartAcquisitionClicked();
    void onStopAcquisitionClicked();
    void onSaveDataClicked();
    void onClearDataClicked();

    // 数据库连接
    void onConnectDatabaseClicked();

    // 通道显示
    void onChannelVisibilityChanged(int channel, int state);
    void updateStatusBar();

    // 历史数据
    void onViewHistoryClicked();
    void onReplayTask(int taskId);

    // 数据分析
    void onAnalyzeDataClicked();
    void onAnalysisCompleted(const QVector<AnalysisResult>& results);

    // 数据保存完成
    void onSaveCompleted(bool success, const QString& message);

private:
    void setupUi();
    void connectSignals();
    void updateConnectionStatus();
    bool validateTaskInfo();
    QVector<int> getSelectedChannels();

    MainWindowUI *ui;

    // 核心组件
    DataBuffer* m_dataBuffer;
    IioReceiver* m_iioReceiver;
    WaveformWidget* m_waveformWidget;
    DatabaseManager* m_dbManager;
    DataProcessor* m_dataProcessor;
    DataAnalyzer* m_dataAnalyzer;

    // 多线程
    QThread* m_databaseThread;
    DatabaseWorker* m_databaseWorker;
    QThread* m_analysisThread;
    AnalysisWorker* m_analysisWorker;

    // 状态标签
    QLabel* m_statusLabel;
    QLabel* m_connectionLabel;
    QLabel* m_dataCountLabel;

    // 定时器
    QTimer* m_statusUpdateTimer;

    // 状态标志
    bool m_isAcquiring;
    bool m_isDatabaseConnected;
    int m_currentTaskId;

    // 采集参数
    double m_startTime;
    QBitArray m_enabledChannels;
};

#endif // MAINWINDOW_H
