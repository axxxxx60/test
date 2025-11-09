#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QMap>
#include "databuffer.h"

class WaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformWidget(QWidget *parent = nullptr);
    ~WaveformWidget();

    void setDataBuffer(DataBuffer* buffer);
    void setChannelVisible(int channel, bool visible);
    bool isChannelVisible(int channel) const;

    void setUpdateInterval(int ms);
    void setMaxDisplayPoints(int points);

    void setTimeRange(double seconds);
    void setAmplitudeRange(double min, double max);
    void setAutoScale(bool enable);

    // 设置显示数据（用于历史数据回放）
    void setDisplayData(int channel, const QVector<DataPoint>& data);
    void clearDisplayData();

    // 设置颜色
    void setChannelColor(int channel, const QColor& color);

public slots:
    void startDisplay();
    void stopDisplay();
    void clearDisplay();
    void updateDisplay();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawGrid(QPainter& painter);
    void drawAxes(QPainter& painter);
    void drawWaveform(QPainter& painter, int channel);
    void drawLabels(QPainter& painter);
    void calculateScales();
    QColor getChannelColor(int channel);

    DataBuffer* m_dataBuffer;
    QTimer* m_updateTimer;

    // 显示参数
    QMap<int, bool> m_channelVisible;  // 通道可见性
    int m_maxDisplayPoints;
    double m_timeRange;
    double m_amplitudeMin;
    double m_amplitudeMax;
    bool m_autoScale;

    // 颜色映射
    QMap<int, QColor> m_channelColors;
    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_axesColor;

    // 坐标转换参数
    double m_xScale;
    double m_yScale;
    int m_leftMargin;
    int m_rightMargin;
    int m_topMargin;
    int m_bottomMargin;

    // 历史数据显示
    QMap<int, QVector<DataPoint>> m_historyData;
    bool m_displayingHistory;
};

#endif // WAVEFORMWIDGET_H
