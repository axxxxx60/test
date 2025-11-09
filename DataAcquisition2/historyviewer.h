#ifndef HISTORYVIEWER_H
#define HISTORYVIEWER_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include "databasemanager.h"
#include "waveformwidget.h"

class HistoryViewer : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryViewer(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~HistoryViewer();

signals:
    void taskSelected(int taskId);
    void replayRequested(int taskId);

private slots:
    void onSearchClicked();
    void onRefreshClicked();
    void onViewClicked();
    void onDeleteClicked();
    void onAnalyzeClicked();
    void onTableSelectionChanged();

private:
    void createWidgets();
    void setupLayout();
    void setupTableWidget();
    void connectSignals();
    void loadTasks();
    void updateTaskTable(const QVector<TaskInfo>& tasks);
    void showTaskDetails(int taskId);

    // UI控件
    QLineEdit* m_searchLineEdit;
    QPushButton* m_searchButton;
    QPushButton* m_refreshButton;
    QTableWidget* m_taskTableWidget;
    QPushButton* m_viewButton;
    QPushButton* m_deleteButton;
    QPushButton* m_analyzeButton;
    QPushButton* m_closeButton;
    QLabel* m_statusLabel;

    // 数据
    DatabaseManager* m_dbManager;
    QVector<TaskInfo> m_currentTasks;
};

#endif // HISTORYVIEWER_H
