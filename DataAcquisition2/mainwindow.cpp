#include "mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QProgressDialog>
#include <QStatusBar>

// ==================== Worker Implementations ====================

void DatabaseWorker::saveTaskData(const TaskInfo& taskInfo,
                                  const QVector<QVector<DataPoint>>& channelData)
{
    emit progressUpdated(10, "正在创建任务记录...");

    int taskId = m_dbManager->createTask(taskInfo);
    if (taskId < 0) {
        emit saveCompleted(false, "创建任务失败");
        return;
    }

    emit progressUpdated(20, "正在保存通道数据...");

    // 保存各个通道的数据
    int totalChannels = channelData.size();
    for (int i = 0; i < totalChannels; ++i) {
        if (!channelData[i].isEmpty()) {
            int channel = taskInfo.enabledChannels.value(i, i);

            emit progressUpdated(20 + (i * 60 / totalChannels),
                                 QString("正在保存通道 %1 数据...").arg(channel));

            if (!m_dbManager->saveRawData(taskId, channel, channelData[i])) {
                emit saveCompleted(false, QString("保存通道 %1 数据失败").arg(channel));
                return;
            }
        }
    }

    emit progressUpdated(100, "数据保存完成");
    emit saveCompleted(true, "数据已成功保存到数据库");
}

void DatabaseWorker::saveAnalysisResults(const QVector<AnalysisResult>& results)
{
    for (const auto& result : results) {
        if (!m_dbManager->saveAnalysisResult(result)) {
            emit saveCompleted(false, "保存分析结果失败");
            return;
        }
    }
    emit saveCompleted(true, "分析结果已保存");
}

void AnalysisWorker::analyzeData(const QVector<QVector<DataPoint>>& channelData,
                                 int taskId, double sampleRate)
{
    QVector<AnalysisResult> results;

    int totalChannels = channelData.size();
    for (int i = 0; i < totalChannels; ++i) {
        if (!channelData[i].isEmpty()) {
            emit progressUpdated((i * 100) / totalChannels,
                                 QString("正在分析通道 %1...").arg(i));

            AnalysisResult result = m_analyzer->performFullAnalysis(
                channelData[i], taskId, i, sampleRate);
            results.append(result);
        }
    }

    emit progressUpdated(100, "分析完成");
    emit analysisCompleted(results);
}

// ==================== MainWindow Implementation ====================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new MainWindowUI())
    , m_dataBuffer(new DataBuffer(this))
    , m_iioReceiver(new IioReceiver(m_dataBuffer, this))
    , m_waveformWidget(new WaveformWidget(this))
    , m_dbManager(new DatabaseManager(this))
    , m_dataProcessor(new DataProcessor(this))
    , m_dataAnalyzer(new DataAnalyzer(this))
    , m_statusUpdateTimer(new QTimer(this))
    , m_isAcquiring(false)
    , m_isDatabaseConnected(false)
    , m_currentTaskId(-1)
    , m_startTime(0.0)
{
    setupUi();
    connectSignals();

    setWindowTitle("数据采集与分析系统 - IIO版");
    resize(1400, 900);

    // 初始化多线程
    m_databaseThread = new QThread(this);
    m_databaseWorker = new DatabaseWorker(m_dbManager);
    m_databaseWorker->moveToThread(m_databaseThread);
    connect(m_databaseThread, &QThread::finished, m_databaseWorker, &QObject::deleteLater);
    connect(m_databaseWorker, &DatabaseWorker::saveCompleted,
            this, &MainWindow::onSaveCompleted);
    connect(m_databaseWorker, &DatabaseWorker::progressUpdated,
            [this](int p, const QString& m) {
                m_statusLabel->setText(QString("%1 (%2%)").arg(m).arg(p));
            });
    m_databaseThread->start();

    m_analysisThread = new QThread(this);
    m_analysisWorker = new AnalysisWorker(m_dataAnalyzer);
    m_analysisWorker->moveToThread(m_analysisThread);
    connect(m_analysisThread, &QThread::finished, m_analysisWorker, &QObject::deleteLater);
    connect(m_analysisWorker, &AnalysisWorker::analysisCompleted,
            this, &MainWindow::onAnalysisCompleted);
    connect(m_analysisWorker, &AnalysisWorker::progressUpdated,
            [this](int p, const QString& m) {
                m_statusLabel->setText(QString("%1 (%2%)").arg(m).arg(p));
            });
    m_analysisThread->start();

    // 启动状态更新定时器
    m_statusUpdateTimer->start(1000);

    // 初始化通道启用状态
    m_enabledChannels.resize(MAX_CHANNELS);
    m_enabledChannels.fill(false);
    m_enabledChannels.setBit(0, true);
    m_enabledChannels.setBit(1, true);
}

MainWindow::~MainWindow()
{
    if (m_isAcquiring) {
        onStopAcquisitionClicked();
    }

    m_databaseThread->quit();
    m_databaseThread->wait();

    m_analysisThread->quit();
    m_analysisThread->wait();

    delete ui;
}

void MainWindow::setupUi()
{
    // 创建UI
    ui->setupUi(this);
    setCentralWidget(ui->centralWidget);

    // 设置波形显示控件
    m_waveformWidget->setDataBuffer(m_dataBuffer);
    ui->waveformLayout->addWidget(m_waveformWidget);

    // 创建状态栏标签
    m_statusLabel = new QLabel("就绪", this);
    m_connectionLabel = new QLabel("未连接", this);
    m_dataCountLabel = new QLabel("数据点: 0", this);

    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_connectionLabel);
    statusBar()->addPermanentWidget(m_dataCountLabel);

    // 设置默认任务名称
    ui->taskNameEdit->setText(QString("Task_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")));
}

void MainWindow::connectSignals()
{
    // IIO连接信号
    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    connect(ui->disconnectButton, &QPushButton::clicked,
            this, &MainWindow::onDisconnectClicked);
    connect(m_iioReceiver, &IioReceiver::connected,
            this, &MainWindow::onIioConnected);
    connect(m_iioReceiver, &IioReceiver::disconnected,
            this, &MainWindow::onIioDisconnected);
    connect(m_iioReceiver, &IioReceiver::errorOccurred,
            this, &MainWindow::onIioError);

    // 数据采集信号
    connect(ui->startAcquisitionButton, &QPushButton::clicked,
            this, &MainWindow::onStartAcquisitionClicked);
    connect(ui->stopAcquisitionButton, &QPushButton::clicked,
            this, &MainWindow::onStopAcquisitionClicked);
    connect(ui->saveDataButton, &QPushButton::clicked,
            this, &MainWindow::onSaveDataClicked);
    connect(ui->clearDataButton, &QPushButton::clicked,
            this, &MainWindow::onClearDataClicked);

    // 数据库连接
    connect(ui->connectDbButton, &QPushButton::clicked,
            this, &MainWindow::onConnectDatabaseClicked);

    // 通道选择
    for (int i = 0; i < 13; ++i) {
        connect(ui->channelCheckBoxes[i], &QCheckBox::stateChanged,
                [this, i](int state) { onChannelVisibilityChanged(i, state); });
    }

    // 历史数据
    connect(ui->viewHistoryButton, &QPushButton::clicked,
            this, &MainWindow::onViewHistoryClicked);

    // 数据分析
    connect(ui->analyzeButton, &QPushButton::clicked,
            this, &MainWindow::onAnalyzeDataClicked);

    // 状态更新
    connect(m_statusUpdateTimer, &QTimer::timeout,
            this, &MainWindow::updateStatusBar);
}

void MainWindow::onConnectClicked()
{
    QString ipAddress;

    if (ui->clientModeRadio->isChecked()) {
        ipAddress = ui->clientHostEdit->text().trimmed();
        if (ipAddress.isEmpty()) {
            QMessageBox::warning(this, "警告", "请输入主机地址");
            return;
        }
    } else {
        ipAddress = "192.168.10.131"; // 默认地址，可从配置读取
    }

    // 更新启用的通道
    QVector<int> enabledChannels = getSelectedChannels();
    m_iioReceiver->setEnabledChannels(enabledChannels);
    m_iioReceiver->setSampleRate(ui->sampleRateSpinBox->value());

    if (m_iioReceiver->connectToDevice(ipAddress)) {
        ui->connectButton->setEnabled(false);
        ui->disconnectButton->setEnabled(true);
        ui->connectionGroupBox->setEnabled(false);
        m_statusLabel->setText("正在连接IIO设备...");
    } else {
        QMessageBox::warning(this, "连接失败", "无法连接到IIO设备");
    }
}

void MainWindow::onDisconnectClicked()
{
    if (m_isAcquiring) {
        onStopAcquisitionClicked();
    }

    m_iioReceiver->disconnectFromDevice();

    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->connectionGroupBox->setEnabled(true);
    ui->startAcquisitionButton->setEnabled(false);

    m_statusLabel->setText("已断开连接");
}

void MainWindow::onIioConnected()
{
    m_statusLabel->setText("IIO设备已连接");
    m_connectionLabel->setText(QString("已连接: %1").arg(
        m_iioReceiver->getConnectionInfo()));
    ui->startAcquisitionButton->setEnabled(true);

    QMessageBox::information(this, "连接成功", "IIO设备连接已建立");
}

void MainWindow::onIioDisconnected()
{
    m_statusLabel->setText("IIO连接已断开");
    m_connectionLabel->setText("未连接");
    ui->startAcquisitionButton->setEnabled(false);

    if (m_isAcquiring) {
        onStopAcquisitionClicked();
    }
}

void MainWindow::onIioError(const QString& error)
{
    QMessageBox::critical(this, "IIO错误", error);
    m_statusLabel->setText("错误: " + error);
}

void MainWindow::onStartAcquisitionClicked()
{
    if (!validateTaskInfo()) {
        return;
    }

    // 清空缓冲区
    m_dataBuffer->clear();

    // 记录开始时间
    m_startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;

    // 启动显示
    m_waveformWidget->startDisplay();

    m_isAcquiring = true;
    ui->startAcquisitionButton->setEnabled(false);
    ui->stopAcquisitionButton->setEnabled(true);
    ui->saveDataButton->setEnabled(false);
    ui->taskInfoGroupBox->setEnabled(false);
    ui->channelSelectGroupBox->setEnabled(false);

    m_statusLabel->setText("正在采集数据...");
}

void MainWindow::onStopAcquisitionClicked()
{
    m_waveformWidget->stopDisplay();

    m_isAcquiring = false;
    ui->startAcquisitionButton->setEnabled(true);
    ui->stopAcquisitionButton->setEnabled(false);
    ui->saveDataButton->setEnabled(true);
    ui->analyzeButton->setEnabled(true);
    ui->taskInfoGroupBox->setEnabled(true);
    ui->channelSelectGroupBox->setEnabled(true);

    m_statusLabel->setText("采集已停止");
}

void MainWindow::onSaveDataClicked()
{
    if (!m_isDatabaseConnected) {
        QMessageBox::warning(this, "警告", "请先连接数据库");
        return;
    }

    if (!validateTaskInfo()) {
        return;
    }

    // 准备任务信息
    TaskInfo taskInfo;
    taskInfo.taskName = ui->taskNameEdit->text();
    taskInfo.sampleRate = ui->sampleRateSpinBox->value();
    taskInfo.description = ui->taskDescEdit->toPlainText();
    taskInfo.channelCount = 13;
    taskInfo.enabledChannels = getSelectedChannels();

    // 计算时长
    QVector<int> channels = getSelectedChannels();
    if (!channels.isEmpty()) {
        QVector<DataPoint> data = m_dataBuffer->getAllChannelData(channels[0]);
        if (!data.isEmpty()) {
            taskInfo.duration = data.last().time - data.first().time;
        }
    }

    // 获取所有通道数据
    QVector<QVector<DataPoint>> channelData;
    for (int ch : channels) {
        channelData.append(m_dataBuffer->getAllChannelData(ch));
    }

    // 在工作线程中保存
    QMetaObject::invokeMethod(m_databaseWorker, "saveTaskData",
                              Qt::QueuedConnection,
                              Q_ARG(TaskInfo, taskInfo),
                              Q_ARG(QVector<QVector<DataPoint>>, channelData));

    m_statusLabel->setText("正在保存数据...");
}

void MainWindow::onClearDataClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要清空当前数据吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_dataBuffer->clear();
        m_waveformWidget->clearDisplay();
        m_statusLabel->setText("数据已清空");
    }
}

void MainWindow::onConnectDatabaseClicked()
{
    QString host = ui->dbHostEdit->text();
    int port = ui->dbPortSpinBox->value();
    QString dbName = ui->dbNameEdit->text();
    QString user = ui->dbUserEdit->text();
    QString password = ui->dbPasswordEdit->text();

    if (m_dbManager->connectToDatabase(host, port, dbName, user, password)) {
        m_isDatabaseConnected = true;
        ui->connectDbButton->setEnabled(false);
        ui->dbConfigGroupBox->setEnabled(false);
        ui->viewHistoryButton->setEnabled(true);

        m_statusLabel->setText("数据库已连接");
        QMessageBox::information(this, "成功", "数据库连接成功");
    } else {
        QMessageBox::critical(this, "错误",
                              QString("数据库连接失败: %1").arg(m_dbManager->getLastError()));
    }
}

void MainWindow::onChannelVisibilityChanged(int channel, int state)
{
    bool visible = (state == Qt::Checked);
    m_enabledChannels.setBit(channel, visible);
    m_waveformWidget->setChannelVisible(channel, visible);

    // 如果正在连接，更新接收器的通道配置
    if (m_iioReceiver->isConnected()) {
        m_iioReceiver->setEnabledChannels(getSelectedChannels());
    }
}

void MainWindow::updateStatusBar()
{
    QString countText = "通道数据: ";
    QVector<int> channels = getSelectedChannels();
    for (int ch : channels) {
        int count = m_dataBuffer->getDataCount(ch);
        countText += QString("CH%1:%2 ").arg(ch).arg(count);
    }
    m_dataCountLabel->setText(countText);
}

void MainWindow::onViewHistoryClicked()
{
    if (!m_isDatabaseConnected) {
        QMessageBox::warning(this, "警告", "请先连接数据库");
        return;
    }

    HistoryViewer* viewer = new HistoryViewer(m_dbManager, this);
    connect(viewer, &HistoryViewer::replayRequested,
            this, &MainWindow::onReplayTask);
    viewer->exec();
    delete viewer;
}

void MainWindow::onReplayTask(int taskId)
{
    m_statusLabel->setText("正在加载历史数据...");

    // 停止当前采集
    if (m_isAcquiring) {
        onStopAcquisitionClicked();
    }

    // 获取任务信息
    TaskInfo task = m_dbManager->getTask(taskId);

    // 加载多通道数据
    m_waveformWidget->clearDisplayData();

    for (int ch : task.enabledChannels) {
        QVector<DataPoint> data = m_dbManager->loadRawData(taskId, ch);
        if (!data.isEmpty()) {
            m_waveformWidget->setDisplayData(ch, data);
        }
    }

    m_currentTaskId = taskId;
    m_statusLabel->setText("历史数据回放中");
}

void MainWindow::onAnalyzeDataClicked()
{
    QVector<int> channels = getSelectedChannels();
    QVector<QVector<DataPoint>> channelData;

    for (int ch : channels) {
        channelData.append(m_dataBuffer->getAllChannelData(ch));
    }

    if (channelData.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有可分析的数据");
        return;
    }

    double sampleRate = ui->sampleRateSpinBox->value();

    // 在工作线程中分析
    QMetaObject::invokeMethod(m_analysisWorker, "analyzeData",
                              Qt::QueuedConnection,
                              Q_ARG(QVector<QVector<DataPoint>>, channelData),
                              Q_ARG(int, m_currentTaskId),
                              Q_ARG(double, sampleRate));

    m_statusLabel->setText("正在分析数据...");
}

void MainWindow::onAnalysisCompleted(const QVector<AnalysisResult>& results)
{
    QString message = "=== 分析结果 ===\n\n";

    for (const auto& result : results) {
        message += QString("通道 %1:\n").arg(result.channel);
        message += QString("  最大幅值: %1\n").arg(result.maxAmplitude);
        message += QString("  最小幅值: %1\n").arg(result.minAmplitude);
        message += QString("  平均幅值: %1\n").arg(result.avgAmplitude);
        message += QString("  均方根值: %1\n").arg(result.rmsValue);
        message += QString("  主频率: %1 Hz\n\n").arg(result.frequency);
    }

    QMessageBox::information(this, "分析结果", message);

    // 如果已连接数据库，保存分析结果
    if (m_isDatabaseConnected) {
        QMetaObject::invokeMethod(m_databaseWorker, "saveAnalysisResults",
                                  Qt::QueuedConnection,
                                  Q_ARG(QVector<AnalysisResult>, results));
    }

    m_statusLabel->setText("分析完成");
}

void MainWindow::onSaveCompleted(bool success, const QString& message)
{
    if (success) {
        QMessageBox::information(this, "成功", message);
    } else {
        QMessageBox::critical(this, "错误", message);
    }
    m_statusLabel->setText(message);
}

void MainWindow::updateConnectionStatus()
{
    if (m_iioReceiver->isConnected()) {
        m_connectionLabel->setText(QString("已连接: %1").arg(
            m_iioReceiver->getConnectionInfo()));
    } else {
        m_connectionLabel->setText("未连接");
    }
}

bool MainWindow::validateTaskInfo()
{
    if (ui->taskNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入任务名称");
        return false;
    }

    if (ui->sampleRateSpinBox->value() <= 0) {
        QMessageBox::warning(this, "警告", "采样率必须大于0");
        return false;
    }

    if (getSelectedChannels().isEmpty()) {
        QMessageBox::warning(this, "警告", "请至少选择一个通道");
        return false;
    }

    return true;
}

QVector<int> MainWindow::getSelectedChannels()
{
    QVector<int> channels;
    for (int i = 0; i < 13; ++i) {
        if (ui->channelCheckBoxes[i]->isChecked()) {
            channels.append(i);
        }
    }
    return channels;
}
