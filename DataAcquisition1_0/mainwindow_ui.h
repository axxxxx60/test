#ifndef MAINWINDOW_UI_H
#define MAINWINDOW_UI_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QScrollArea>

class MainWindowUI
{
public:
    // 主布局
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;

    // 菜单栏
    QMenuBar *menuBar;
    QMenu *deviceMenu;
    QMenu *databaseMenu;
    QMenu *taskMenu;
    QMenu *channelMenu;
    QMenu *helpMenu;

    // 菜单动作
    QAction *connectDeviceAction;
    QAction *disconnectDeviceAction;
    QAction *connectDbAction;
    QAction *viewHistoryAction;
    QAction *taskInfoAction;
    QAction *channelSelectAction;
    QAction *aboutAction;

    // IIO设备连接对话框组件
    QDialog *deviceDialog;
    QGroupBox *deviceGroupBox;
    QGridLayout *deviceLayout;
    QLabel *deviceIpLabel;
    QLineEdit *deviceIpEdit;
    QLabel *sampleRateLabel;
    QDoubleSpinBox *sampleRateSpinBox;
    QLabel *bufferSizeLabel;
    QSpinBox *bufferSizeSpinBox;
    QPushButton *connectButton;
    QPushButton *disconnectButton;

    // 数据库对话框组件
    QDialog *databaseDialog;
    QGroupBox *dbConfigGroupBox;
    QGridLayout *dbConfigLayout;
    QLabel *dbHostLabel;
    QLineEdit *dbHostEdit;
    QLabel *dbPortLabel;
    QSpinBox *dbPortSpinBox;
    QLabel *dbNameLabel;
    QLineEdit *dbNameEdit;
    QLabel *dbUserLabel;
    QLineEdit *dbUserEdit;
    QLabel *dbPasswordLabel;
    QLineEdit *dbPasswordEdit;
    QPushButton *connectDbButton;
    QPushButton *viewHistoryButton;

    // 任务信息对话框组件
    QDialog *taskDialog;
    QGroupBox *taskInfoGroupBox;
    QGridLayout *taskInfoLayout;
    QLabel *taskNameLabel;
    QLineEdit *taskNameEdit;
    QLabel *taskDescLabel;
    QTextEdit *taskDescEdit;
    QPushButton *taskConfirmButton;

    // 通道选择对话框组件
    QDialog *channelDialog;
    QGroupBox *channelSelectGroupBox;
    QScrollArea *channelScrollArea;
    QWidget *channelWidget;
    QVBoxLayout *channelSelectLayout;
    QCheckBox *channelCheckBoxes[13];
    QPushButton *selectAllButton;
    QPushButton *clearAllButton;
    QPushButton *channelConfirmButton;

    // 主界面 - 采集控制和数据显示
    QWidget *controlPanel;
    QHBoxLayout *controlLayout;
    QGroupBox *acquisitionGroupBox;
    QHBoxLayout *acquisitionLayout;
    QPushButton *startAcquisitionButton;
    QPushButton *stopAcquisitionButton;
    QPushButton *saveDataButton;
    QPushButton *clearDataButton;
    QPushButton *analyzeButton;

    // 波形显示区域
    QWidget *displayPanel;
    QVBoxLayout *displayLayout;
    QLabel *waveformTitle;
    QWidget *waveformContainer;
    QVBoxLayout *waveformLayout;

    // 状态信息面板
    QGroupBox *statusGroupBox;
    QGridLayout *statusLayout;
    QLabel *connectionStatusLabel;
    QLabel *connectionStatusValue;
    QLabel *dbStatusLabel;
    QLabel *dbStatusValue;
    QLabel *channelCountLabel;
    QLabel *channelCountValue;
    QLabel *dataPointsLabel;
    QLabel *dataPointsValue;

    void setupUi(QWidget *mainWindow)
    {
        // ========== 创建菜单栏 ==========
        menuBar = new QMenuBar(mainWindow);

        // 设备连接菜单
        deviceMenu = new QMenu("设备连接(&D)", menuBar);
        connectDeviceAction = new QAction("连接IIO设备(&C)", mainWindow);
        connectDeviceAction->setShortcut(QKeySequence("Ctrl+D"));
        disconnectDeviceAction = new QAction("断开连接(&X)", mainWindow);
        disconnectDeviceAction->setShortcut(QKeySequence("Ctrl+Shift+D"));
        disconnectDeviceAction->setEnabled(false);
        deviceMenu->addAction(connectDeviceAction);
        deviceMenu->addAction(disconnectDeviceAction);

        // 数据库菜单
        databaseMenu = new QMenu("数据库(&B)", menuBar);
        connectDbAction = new QAction("连接数据库(&C)", mainWindow);
        connectDbAction->setShortcut(QKeySequence("Ctrl+B"));
        viewHistoryAction = new QAction("查看历史记录(&H)", mainWindow);
        viewHistoryAction->setShortcut(QKeySequence("Ctrl+H"));
        viewHistoryAction->setEnabled(false);
        databaseMenu->addAction(connectDbAction);
        databaseMenu->addSeparator();
        databaseMenu->addAction(viewHistoryAction);

        // 任务菜单
        taskMenu = new QMenu("任务配置(&T)", menuBar);
        taskInfoAction = new QAction("任务信息设置(&I)", mainWindow);
        taskInfoAction->setShortcut(QKeySequence("Ctrl+T"));
        taskMenu->addAction(taskInfoAction);

        // 通道菜单
        channelMenu = new QMenu("通道管理(&C)", menuBar);
        channelSelectAction = new QAction("通道选择(&S)", mainWindow);
        channelSelectAction->setShortcut(QKeySequence("Ctrl+L"));
        channelMenu->addAction(channelSelectAction);

        // 帮助菜单
        helpMenu = new QMenu("帮助(&H)", menuBar);
        aboutAction = new QAction("关于(&A)", mainWindow);
        helpMenu->addAction(aboutAction);

        menuBar->addMenu(deviceMenu);
        menuBar->addMenu(databaseMenu);
        menuBar->addMenu(taskMenu);
        menuBar->addMenu(channelMenu);
        menuBar->addMenu(helpMenu);

        // ========== 创建中央部件 ==========
        centralWidget = new QWidget(mainWindow);
        mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->setSpacing(10);

        // ========== 控制面板 ==========
        controlPanel = new QWidget();
        controlLayout = new QHBoxLayout(controlPanel);
        controlLayout->setContentsMargins(0, 0, 0, 0);

        // 采集控制组
        acquisitionGroupBox = new QGroupBox("数据采集控制");
        acquisitionLayout = new QHBoxLayout(acquisitionGroupBox);

        startAcquisitionButton = new QPushButton("开始采集");
        startAcquisitionButton->setMinimumHeight(35);
        startAcquisitionButton->setEnabled(false);

        stopAcquisitionButton = new QPushButton("停止采集");
        stopAcquisitionButton->setMinimumHeight(35);
        stopAcquisitionButton->setEnabled(false);

        saveDataButton = new QPushButton("保存数据");
        saveDataButton->setMinimumHeight(35);
        saveDataButton->setEnabled(false);

        clearDataButton = new QPushButton("清空数据");
        clearDataButton->setMinimumHeight(35);

        analyzeButton = new QPushButton("数据分析");
        analyzeButton->setMinimumHeight(35);
        analyzeButton->setEnabled(false);

        acquisitionLayout->addWidget(startAcquisitionButton);
        acquisitionLayout->addWidget(stopAcquisitionButton);
        acquisitionLayout->addWidget(saveDataButton);
        acquisitionLayout->addWidget(clearDataButton);
        acquisitionLayout->addWidget(analyzeButton);

        // 状态信息组
        statusGroupBox = new QGroupBox("系统状态");
        statusLayout = new QGridLayout(statusGroupBox);

        connectionStatusLabel = new QLabel("设备状态:");
        connectionStatusValue = new QLabel("未连接");
        connectionStatusValue->setStyleSheet("QLabel { color: red; font-weight: bold; }");

        dbStatusLabel = new QLabel("数据库:");
        dbStatusValue = new QLabel("未连接");
        dbStatusValue->setStyleSheet("QLabel { color: red; font-weight: bold; }");

        channelCountLabel = new QLabel("启用通道:");
        channelCountValue = new QLabel("0");

        dataPointsLabel = new QLabel("数据点数:");
        dataPointsValue = new QLabel("0");

        statusLayout->addWidget(connectionStatusLabel, 0, 0);
        statusLayout->addWidget(connectionStatusValue, 0, 1);
        statusLayout->addWidget(dbStatusLabel, 0, 2);
        statusLayout->addWidget(dbStatusValue, 0, 3);
        statusLayout->addWidget(channelCountLabel, 1, 0);
        statusLayout->addWidget(channelCountValue, 1, 1);
        statusLayout->addWidget(dataPointsLabel, 1, 2);
        statusLayout->addWidget(dataPointsValue, 1, 3);

        controlLayout->addWidget(acquisitionGroupBox, 3);
        controlLayout->addWidget(statusGroupBox, 2);

        mainLayout->addWidget(controlPanel);

        // ========== 波形显示区域 ==========
        displayPanel = new QWidget();
        displayLayout = new QVBoxLayout(displayPanel);
        displayLayout->setContentsMargins(0, 0, 0, 0);

        waveformTitle = new QLabel("多通道波形显示");
        waveformTitle->setAlignment(Qt::AlignCenter);
        QFont titleFont = waveformTitle->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        waveformTitle->setFont(titleFont);
        waveformTitle->setStyleSheet("QLabel { background-color: #34495e; color: white; padding: 8px; }");

        waveformContainer = new QWidget();
        waveformLayout = new QVBoxLayout(waveformContainer);
        waveformLayout->setContentsMargins(0, 0, 0, 0);

        displayLayout->addWidget(waveformTitle);
        displayLayout->addWidget(waveformContainer, 1);

        mainLayout->addWidget(displayPanel, 1);

        // ========== 创建IIO设备连接对话框 ==========
        createDeviceDialog(mainWindow);

        // ========== 创建数据库对话框 ==========
        createDatabaseDialog(mainWindow);

        // ========== 创建任务信息对话框 ==========
        createTaskDialog(mainWindow);

        // ========== 创建通道选择对话框 ==========
        createChannelDialog(mainWindow);
    }

private:
    void createDeviceDialog(QWidget *parent)
    {
        deviceDialog = new QDialog(parent);
        deviceDialog->setWindowTitle("IIO设备连接配置");
        deviceDialog->setMinimumWidth(450);

        QVBoxLayout *dialogLayout = new QVBoxLayout(deviceDialog);

        deviceGroupBox = new QGroupBox("设备设置");
        deviceLayout = new QGridLayout(deviceGroupBox);

        deviceIpLabel = new QLabel("设备IP地址:");
        deviceIpEdit = new QLineEdit();
        deviceIpEdit->setText("192.168.10.131");
        deviceIpEdit->setPlaceholderText("例如: 192.168.10.131");

        sampleRateLabel = new QLabel("采样率(Hz):");
        sampleRateSpinBox = new QDoubleSpinBox();
        sampleRateSpinBox->setRange(1, 1000000);
        sampleRateSpinBox->setValue(1000.0);
        sampleRateSpinBox->setDecimals(1);

        bufferSizeLabel = new QLabel("缓冲区大小:");
        bufferSizeSpinBox = new QSpinBox();
        bufferSizeSpinBox->setRange(256, 32768);
        bufferSizeSpinBox->setValue(2560);
        bufferSizeSpinBox->setSingleStep(256);

        connectButton = new QPushButton("连接");
        disconnectButton = new QPushButton("断开");
        disconnectButton->setEnabled(false);

        deviceLayout->addWidget(deviceIpLabel, 0, 0);
        deviceLayout->addWidget(deviceIpEdit, 0, 1);
        deviceLayout->addWidget(sampleRateLabel, 1, 0);
        deviceLayout->addWidget(sampleRateSpinBox, 1, 1);
        deviceLayout->addWidget(bufferSizeLabel, 2, 0);
        deviceLayout->addWidget(bufferSizeSpinBox, 2, 1);
        deviceLayout->addWidget(connectButton, 3, 0);
        deviceLayout->addWidget(disconnectButton, 3, 1);

        dialogLayout->addWidget(deviceGroupBox);

        QPushButton *closeButton = new QPushButton("关闭");
        dialogLayout->addWidget(closeButton);

        QObject::connect(closeButton, &QPushButton::clicked, deviceDialog, &QDialog::close);
    }

    void createDatabaseDialog(QWidget *parent)
    {
        databaseDialog = new QDialog(parent);
        databaseDialog->setWindowTitle("数据库配置");
        databaseDialog->setMinimumWidth(450);

        QVBoxLayout *dialogLayout = new QVBoxLayout(databaseDialog);

        dbConfigGroupBox = new QGroupBox("MySQL配置");
        dbConfigLayout = new QGridLayout(dbConfigGroupBox);

        dbHostLabel = new QLabel("主机:");
        dbHostEdit = new QLineEdit();
        dbHostEdit->setText("localhost");

        dbPortLabel = new QLabel("端口:");
        dbPortSpinBox = new QSpinBox();
        dbPortSpinBox->setRange(1, 65535);
        dbPortSpinBox->setValue(3306);

        dbNameLabel = new QLabel("数据库:");
        dbNameEdit = new QLineEdit();
        dbNameEdit->setText("DataAcquisitionSystem");

        dbUserLabel = new QLabel("用户名:");
        dbUserEdit = new QLineEdit();
        dbUserEdit->setText("root");

        dbPasswordLabel = new QLabel("密码:");
        dbPasswordEdit = new QLineEdit();
        dbPasswordEdit->setEchoMode(QLineEdit::Password);

        connectDbButton = new QPushButton("连接数据库");
        viewHistoryButton = new QPushButton("查看历史记录");
        viewHistoryButton->setEnabled(false);

        dbConfigLayout->addWidget(dbHostLabel, 0, 0);
        dbConfigLayout->addWidget(dbHostEdit, 0, 1);
        dbConfigLayout->addWidget(dbPortLabel, 1, 0);
        dbConfigLayout->addWidget(dbPortSpinBox, 1, 1);
        dbConfigLayout->addWidget(dbNameLabel, 2, 0);
        dbConfigLayout->addWidget(dbNameEdit, 2, 1);
        dbConfigLayout->addWidget(dbUserLabel, 3, 0);
        dbConfigLayout->addWidget(dbUserEdit, 3, 1);
        dbConfigLayout->addWidget(dbPasswordLabel, 4, 0);
        dbConfigLayout->addWidget(dbPasswordEdit, 4, 1);
        dbConfigLayout->addWidget(connectDbButton, 5, 0);
        dbConfigLayout->addWidget(viewHistoryButton, 5, 1);

        dialogLayout->addWidget(dbConfigGroupBox);

        QPushButton *closeButton = new QPushButton("关闭");
        dialogLayout->addWidget(closeButton);

        QObject::connect(closeButton, &QPushButton::clicked, databaseDialog, &QDialog::close);
    }

    void createTaskDialog(QWidget *parent)
    {
        taskDialog = new QDialog(parent);
        taskDialog->setWindowTitle("任务信息配置");
        taskDialog->setMinimumWidth(450);

        QVBoxLayout *dialogLayout = new QVBoxLayout(taskDialog);

        taskInfoGroupBox = new QGroupBox("任务信息");
        taskInfoLayout = new QGridLayout(taskInfoGroupBox);

        taskNameLabel = new QLabel("任务名称:");
        taskNameEdit = new QLineEdit();
        taskNameEdit->setPlaceholderText("输入任务名称");

        taskDescLabel = new QLabel("任务描述:");
        taskDescEdit = new QTextEdit();
        taskDescEdit->setMaximumHeight(100);
        taskDescEdit->setPlaceholderText("输入任务描述信息...");

        taskInfoLayout->addWidget(taskNameLabel, 0, 0);
        taskInfoLayout->addWidget(taskNameEdit, 0, 1);
        taskInfoLayout->addWidget(taskDescLabel, 1, 0, Qt::AlignTop);
        taskInfoLayout->addWidget(taskDescEdit, 1, 1);

        dialogLayout->addWidget(taskInfoGroupBox);

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        taskConfirmButton = new QPushButton("确定");
        QPushButton *cancelButton = new QPushButton("取消");
        buttonLayout->addStretch();
        buttonLayout->addWidget(taskConfirmButton);
        buttonLayout->addWidget(cancelButton);

        dialogLayout->addLayout(buttonLayout);

        QObject::connect(taskConfirmButton, &QPushButton::clicked, taskDialog, &QDialog::accept);
        QObject::connect(cancelButton, &QPushButton::clicked, taskDialog, &QDialog::reject);
    }

    void createChannelDialog(QWidget *parent)
    {
        channelDialog = new QDialog(parent);
        channelDialog->setWindowTitle("通道选择");
        channelDialog->setMinimumSize(350, 500);

        QVBoxLayout *dialogLayout = new QVBoxLayout(channelDialog);

        channelSelectGroupBox = new QGroupBox("通道列表 (最多13个通道)");
        QVBoxLayout *groupLayout = new QVBoxLayout(channelSelectGroupBox);

        // 创建滚动区域
        channelScrollArea = new QScrollArea();
        channelScrollArea->setWidgetResizable(true);
        channelScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        channelWidget = new QWidget();
        channelSelectLayout = new QVBoxLayout(channelWidget);
        channelSelectLayout->setSpacing(5);

        for (int i = 0; i < 13; ++i) {
            channelCheckBoxes[i] = new QCheckBox(QString("通道 %1").arg(i));
            channelCheckBoxes[i]->setChecked(i < 2); // 默认选中前两个通道
            channelSelectLayout->addWidget(channelCheckBoxes[i]);
        }

        channelScrollArea->setWidget(channelWidget);
        groupLayout->addWidget(channelScrollArea);

        // 快捷按钮
        QHBoxLayout *quickButtonLayout = new QHBoxLayout();
        selectAllButton = new QPushButton("全选");
        clearAllButton = new QPushButton("清空");
        quickButtonLayout->addWidget(selectAllButton);
        quickButtonLayout->addWidget(clearAllButton);
        groupLayout->addLayout(quickButtonLayout);

        dialogLayout->addWidget(channelSelectGroupBox);

        // 确认/取消按钮
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        channelConfirmButton = new QPushButton("确定");
        QPushButton *cancelButton = new QPushButton("取消");
        buttonLayout->addStretch();
        buttonLayout->addWidget(channelConfirmButton);
        buttonLayout->addWidget(cancelButton);

        dialogLayout->addLayout(buttonLayout);

        // 连接信号
        QObject::connect(selectAllButton, &QPushButton::clicked, [this]() {
            for (int i = 0; i < 13; ++i) {
                channelCheckBoxes[i]->setChecked(true);
            }
        });

        QObject::connect(clearAllButton, &QPushButton::clicked, [this]() {
            for (int i = 0; i < 13; ++i) {
                channelCheckBoxes[i]->setChecked(false);
            }
        });

        QObject::connect(channelConfirmButton, &QPushButton::clicked, channelDialog, &QDialog::accept);
        QObject::connect(cancelButton, &QPushButton::clicked, channelDialog, &QDialog::reject);
    }
};

#endif // MAINWINDOW_UI_H
