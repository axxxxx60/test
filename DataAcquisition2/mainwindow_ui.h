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
#include <QRadioButton>
#include <QTabWidget>
#include <QSplitter>

class MainWindowUI
{
public:
    // 主布局
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QSplitter *mainSplitter;

    // 左侧控制面板
    QWidget *leftPanel;
    QVBoxLayout *leftPanelLayout;

    // 网络连接组
    QGroupBox *connectionGroupBox;
    QGridLayout *connectionLayout;
    QRadioButton *serverModeRadio;
    QRadioButton *clientModeRadio;
    QLabel *serverPortLabel;
    QSpinBox *serverPortSpinBox;
    QLabel *clientHostLabel;
    QLineEdit *clientHostEdit;
    QPushButton *connectButton;
    QPushButton *disconnectButton;

    // 任务信息组
    QGroupBox *taskInfoGroupBox;
    QGridLayout *taskInfoLayout;
    QLabel *taskNameLabel;
    QLineEdit *taskNameEdit;
    QLabel *sampleRateLabel;
    QDoubleSpinBox *sampleRateSpinBox;
    QLabel *taskDescLabel;
    QTextEdit *taskDescEdit;

    // 数据采集控制组
    QGroupBox *acquisitionGroupBox;
    QVBoxLayout *acquisitionLayout;
    QPushButton *startAcquisitionButton;
    QPushButton *stopAcquisitionButton;
    QPushButton *saveDataButton;
    QPushButton *clearDataButton;
    QPushButton *analyzeButton;

    // 通道选择组
    QGroupBox *channelSelectGroupBox;
    QVBoxLayout *channelSelectLayout;
    QCheckBox *channelCheckBoxes[13];

    // 数据库配置组
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

    // 右侧波形显示区域
    QWidget *rightPanel;
    QVBoxLayout *rightPanelLayout;
    QVBoxLayout *waveformLayout;

    void setupUi(QWidget *mainWindow)
    {
        // 创建中央部件
        centralWidget = new QWidget(mainWindow);
        mainLayout = new QVBoxLayout(centralWidget);

        // 创建主分割器
        mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);

        // ========== 左侧控制面板 ==========
        leftPanel = new QWidget();
        leftPanelLayout = new QVBoxLayout(leftPanel);

        // 网络连接组
        connectionGroupBox = new QGroupBox("网络连接");
        connectionLayout = new QGridLayout(connectionGroupBox);

        serverModeRadio = new QRadioButton("服务器模式");
        serverModeRadio->setChecked(true);
        clientModeRadio = new QRadioButton("客户端模式");

        serverPortLabel = new QLabel("端口:");
        serverPortSpinBox = new QSpinBox();
        serverPortSpinBox->setRange(1, 65535);
        serverPortSpinBox->setValue(8888);

        clientHostLabel = new QLabel("主机地址:");
        clientHostEdit = new QLineEdit();
        clientHostEdit->setPlaceholderText("192.168.1.100");

        connectButton = new QPushButton("连接");
        disconnectButton = new QPushButton("断开");
        disconnectButton->setEnabled(false);

        connectionLayout->addWidget(serverModeRadio, 0, 0, 1, 2);
        connectionLayout->addWidget(clientModeRadio, 1, 0, 1, 2);
        connectionLayout->addWidget(serverPortLabel, 2, 0);
        connectionLayout->addWidget(serverPortSpinBox, 2, 1);
        connectionLayout->addWidget(clientHostLabel, 3, 0);
        connectionLayout->addWidget(clientHostEdit, 3, 1);
        connectionLayout->addWidget(connectButton, 4, 0);
        connectionLayout->addWidget(disconnectButton, 4, 1);

        leftPanelLayout->addWidget(connectionGroupBox);

        // 任务信息组
        taskInfoGroupBox = new QGroupBox("任务信息");
        taskInfoLayout = new QGridLayout(taskInfoGroupBox);

        taskNameLabel = new QLabel("任务名称:");
        taskNameEdit = new QLineEdit();

        sampleRateLabel = new QLabel("采样率(Hz):");
        sampleRateSpinBox = new QDoubleSpinBox();
        sampleRateSpinBox->setRange(1, 1000000);
        sampleRateSpinBox->setValue(1000.0);
        sampleRateSpinBox->setDecimals(1);

        taskDescLabel = new QLabel("任务描述:");
        taskDescEdit = new QTextEdit();
        taskDescEdit->setMaximumHeight(80);

        taskInfoLayout->addWidget(taskNameLabel, 0, 0);
        taskInfoLayout->addWidget(taskNameEdit, 0, 1);
        taskInfoLayout->addWidget(sampleRateLabel, 1, 0);
        taskInfoLayout->addWidget(sampleRateSpinBox, 1, 1);
        taskInfoLayout->addWidget(taskDescLabel, 2, 0);
        taskInfoLayout->addWidget(taskDescEdit, 2, 1);

        leftPanelLayout->addWidget(taskInfoGroupBox);

        // 通道选择组
        channelSelectGroupBox = new QGroupBox("通道选择");
        channelSelectLayout = new QVBoxLayout(channelSelectGroupBox);

        for (int i = 0; i < 13; ++i) {
            channelCheckBoxes[i] = new QCheckBox(QString("通道 %1").arg(i));
            channelCheckBoxes[i]->setChecked(i < 2); // 默认选中前两个通道
            channelSelectLayout->addWidget(channelCheckBoxes[i]);
        }

        leftPanelLayout->addWidget(channelSelectGroupBox);

        // 数据采集控制组
        acquisitionGroupBox = new QGroupBox("数据采集控制");
        acquisitionLayout = new QVBoxLayout(acquisitionGroupBox);

        startAcquisitionButton = new QPushButton("开始采集");
        stopAcquisitionButton = new QPushButton("停止采集");
        saveDataButton = new QPushButton("保存数据");
        clearDataButton = new QPushButton("清空数据");
        analyzeButton = new QPushButton("数据分析");

        startAcquisitionButton->setEnabled(false);
        stopAcquisitionButton->setEnabled(false);
        saveDataButton->setEnabled(false);
        analyzeButton->setEnabled(false);

        acquisitionLayout->addWidget(startAcquisitionButton);
        acquisitionLayout->addWidget(stopAcquisitionButton);
        acquisitionLayout->addWidget(saveDataButton);
        acquisitionLayout->addWidget(clearDataButton);
        acquisitionLayout->addWidget(analyzeButton);

        leftPanelLayout->addWidget(acquisitionGroupBox);

        // 数据库配置组
        dbConfigGroupBox = new QGroupBox("数据库配置");
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
        viewHistoryButton = new QPushButton("查看历史");
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

        leftPanelLayout->addWidget(dbConfigGroupBox);
        leftPanelLayout->addStretch();

        // ========== 右侧波形显示区域 ==========
        rightPanel = new QWidget();
        rightPanelLayout = new QVBoxLayout(rightPanel);

        QLabel *waveformTitle = new QLabel("波形显示");
        waveformTitle->setAlignment(Qt::AlignCenter);
        QFont titleFont = waveformTitle->font();
        titleFont.setPointSize(12);
        titleFont.setBold(true);
        waveformTitle->setFont(titleFont);

        rightPanelLayout->addWidget(waveformTitle);

        waveformLayout = new QVBoxLayout();
        rightPanelLayout->addLayout(waveformLayout);

        // 添加到分割器
        mainSplitter->addWidget(leftPanel);
        mainSplitter->addWidget(rightPanel);
        mainSplitter->setStretchFactor(0, 1);
        mainSplitter->setStretchFactor(1, 3);

        // 添加到主布局
        mainLayout->addWidget(mainSplitter);
    }
};

#endif // MAINWINDOW_UI_H
