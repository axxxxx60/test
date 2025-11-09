#include "historyviewer.h"
#include <QMessageBox>
#include <QDebug>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

HistoryViewer::HistoryViewer(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setWindowTitle("历史数据查看");
    resize(900, 600);

    // 创建UI控件
    createWidgets();

    // 设置布局
    setupLayout();

    // 设置表格
    setupTableWidget();

    // 连接信号槽
    connectSignals();

    // 加载任务列表
    loadTasks();
}

HistoryViewer::~HistoryViewer()
{
}

void HistoryViewer::createWidgets()
{
    // 搜索区域控件
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("输入任务名称或描述关键字...");

    m_searchButton = new QPushButton("搜索", this);
    m_refreshButton = new QPushButton("刷新", this);

    // 表格控件
    m_taskTableWidget = new QTableWidget(this);

    // 操作按钮
    m_viewButton = new QPushButton("查看/回放", this);
    m_deleteButton = new QPushButton("删除", this);
    m_analyzeButton = new QPushButton("分析结果", this);
    m_closeButton = new QPushButton("关闭", this);

    // 状态标签
    m_statusLabel = new QLabel("就绪", this);

    // 初始状态
    m_viewButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_analyzeButton->setEnabled(false);
}

void HistoryViewer::setupLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 搜索区域
    QGroupBox* searchGroup = new QGroupBox("搜索", this);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchGroup);
    searchLayout->addWidget(m_searchLineEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_refreshButton);

    mainLayout->addWidget(searchGroup);

    // 表格区域
    QGroupBox* tableGroup = new QGroupBox("任务列表", this);
    QVBoxLayout* tableLayout = new QVBoxLayout(tableGroup);
    tableLayout->addWidget(m_taskTableWidget);

    mainLayout->addWidget(tableGroup);

    // 操作按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_viewButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_analyzeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    // 状态栏
    mainLayout->addWidget(m_statusLabel);

    setLayout(mainLayout);
}

void HistoryViewer::setupTableWidget()
{
    // 设置表格列
    m_taskTableWidget->setColumnCount(7);
    QStringList headers;
    headers << "任务ID" << "任务名称" << "采样率(Hz)"
            << "时长(s)" << "通道数" << "创建时间" << "描述";
    m_taskTableWidget->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    m_taskTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTableWidget->setAlternatingRowColors(true);

    // 设置列宽
    m_taskTableWidget->setColumnWidth(0, 80);
    m_taskTableWidget->setColumnWidth(1, 150);
    m_taskTableWidget->setColumnWidth(2, 100);
    m_taskTableWidget->setColumnWidth(3, 80);
    m_taskTableWidget->setColumnWidth(4, 80);
    m_taskTableWidget->setColumnWidth(5, 150);
    m_taskTableWidget->horizontalHeader()->setStretchLastSection(true);
}

void HistoryViewer::connectSignals()
{
    connect(m_searchButton, &QPushButton::clicked,
            this, &HistoryViewer::onSearchClicked);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &HistoryViewer::onRefreshClicked);
    connect(m_viewButton, &QPushButton::clicked,
            this, &HistoryViewer::onViewClicked);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &HistoryViewer::onDeleteClicked);
    connect(m_analyzeButton, &QPushButton::clicked,
            this, &HistoryViewer::onAnalyzeClicked);
    connect(m_closeButton, &QPushButton::clicked,
            this, &QDialog::reject);
    connect(m_taskTableWidget, &QTableWidget::itemSelectionChanged,
            this, &HistoryViewer::onTableSelectionChanged);
}

void HistoryViewer::loadTasks()
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        QMessageBox::warning(this, "警告", "数据库未连接");
        return;
    }

    m_currentTasks = m_dbManager->getAllTasks();
    updateTaskTable(m_currentTasks);
}

void HistoryViewer::updateTaskTable(const QVector<TaskInfo>& tasks)
{
    m_taskTableWidget->setRowCount(0);

    for (int i = 0; i < tasks.size(); ++i) {
        const TaskInfo& task = tasks[i];

        int row = m_taskTableWidget->rowCount();
        m_taskTableWidget->insertRow(row);

        // 任务ID
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(task.taskId));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_taskTableWidget->setItem(row, 0, idItem);

        // 任务名称
        QTableWidgetItem* nameItem = new QTableWidgetItem(task.taskName);
        m_taskTableWidget->setItem(row, 1, nameItem);

        // 采样率
        QTableWidgetItem* rateItem = new QTableWidgetItem(QString::number(task.sampleRate, 'f', 1));
        rateItem->setTextAlignment(Qt::AlignCenter);
        m_taskTableWidget->setItem(row, 2, rateItem);

        // 时长
        QTableWidgetItem* durationItem = new QTableWidgetItem(QString::number(task.duration, 'f', 2));
        durationItem->setTextAlignment(Qt::AlignCenter);
        m_taskTableWidget->setItem(row, 3, durationItem);

        // 通道数
        QTableWidgetItem* channelItem = new QTableWidgetItem(QString::number(task.channelCount));
        channelItem->setTextAlignment(Qt::AlignCenter);
        m_taskTableWidget->setItem(row, 4, channelItem);

        // 创建时间
        QTableWidgetItem* timeItem = new QTableWidgetItem(task.createTime);
        m_taskTableWidget->setItem(row, 5, timeItem);

        // 描述
        QTableWidgetItem* descItem = new QTableWidgetItem(task.description);
        m_taskTableWidget->setItem(row, 6, descItem);
    }

    // 更新状态
    m_statusLabel->setText(QString("共 %1 条记录").arg(tasks.size()));

    // 默认禁用操作按钮
    m_viewButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_analyzeButton->setEnabled(false);
}

void HistoryViewer::onSearchClicked()
{
    QString keyword = m_searchLineEdit->text().trimmed();

    if (keyword.isEmpty()) {
        loadTasks();
        return;
    }

    if (!m_dbManager || !m_dbManager->isConnected()) {
        QMessageBox::warning(this, "警告", "数据库未连接");
        return;
    }

    m_currentTasks = m_dbManager->searchTasks(keyword);
    updateTaskTable(m_currentTasks);

    if (m_currentTasks.isEmpty()) {
        m_statusLabel->setText("未找到匹配的记录");
    }
}

void HistoryViewer::onRefreshClicked()
{
    m_searchLineEdit->clear();
    loadTasks();
}

void HistoryViewer::onViewClicked()
{
    int currentRow = m_taskTableWidget->currentRow();
    if (currentRow < 0 || currentRow >= m_currentTasks.size()) {
        return;
    }

    int taskId = m_currentTasks[currentRow].taskId;

    // 显示任务详情
    showTaskDetails(taskId);

    // 发出回放请求信号
    emit replayRequested(taskId);

    // 关闭对话框
    accept();
}

void HistoryViewer::onDeleteClicked()
{
    int currentRow = m_taskTableWidget->currentRow();
    if (currentRow < 0 || currentRow >= m_currentTasks.size()) {
        return;
    }

    const TaskInfo& task = m_currentTasks[currentRow];

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除任务 '%1' 及其所有数据吗？\n此操作无法撤销！")
            .arg(task.taskName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_dbManager->deleteTask(task.taskId)) {
            QMessageBox::information(this, "成功", "任务已删除");
            loadTasks();
        } else {
            QMessageBox::critical(this, "错误",
                                  QString("删除任务失败: %1").arg(m_dbManager->getLastError()));
        }
    }
}

void HistoryViewer::onAnalyzeClicked()
{
    int currentRow = m_taskTableWidget->currentRow();
    if (currentRow < 0 || currentRow >= m_currentTasks.size()) {
        return;
    }

    int taskId = m_currentTasks[currentRow].taskId;

    // 获取分析结果
    QVector<AnalysisResult> results = m_dbManager->getAnalysisResults(taskId);

    if (results.isEmpty()) {
        QMessageBox::information(this, "分析结果", "该任务暂无分析结果");
        return;
    }

    // 显示分析结果
    QString message = "=== 分析结果 ===\n\n";

    for (const auto& result : results) {
        message += QString("通道 %1:\n").arg(result.channel);
        message += QString("  最大幅值: %1\n").arg(result.maxAmplitude);
        message += QString("  最小幅值: %1\n").arg(result.minAmplitude);
        message += QString("  平均幅值: %1\n").arg(result.avgAmplitude);
        message += QString("  均方根值: %1\n").arg(result.rmsValue);
        message += QString("  主频率: %1 Hz\n").arg(result.frequency);
        message += QString("  分析时间: %1\n\n").arg(result.analysisTime);
    }

    QMessageBox::information(this, "分析结果", message);
}

void HistoryViewer::onTableSelectionChanged()
{
    bool hasSelection = m_taskTableWidget->currentRow() >= 0;
    m_viewButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_analyzeButton->setEnabled(hasSelection);
}

void HistoryViewer::showTaskDetails(int taskId)
{
    if (!m_dbManager || !m_dbManager->isConnected()) {
        return;
    }

    TaskInfo task = m_dbManager->getTask(taskId);

    if (task.taskId < 0) {
        QMessageBox::warning(this, "警告", "未找到任务信息");
        return;
    }

    QString details = QString(
                          "任务ID: %1\n"
                          "任务名称: %2\n"
                          "采样率: %3 Hz\n"
                          "时长: %4 秒\n"
                          "通道数: %5\n"
                          "创建时间: %6\n"
                          "描述: %7"
                          ).arg(task.taskId)
                          .arg(task.taskName)
                          .arg(task.sampleRate)
                          .arg(task.duration)
                          .arg(task.channelCount)
                          .arg(task.createTime)
                          .arg(task.description.isEmpty() ? "无" : task.description);

    qDebug() << "任务详情:" << details;
}
