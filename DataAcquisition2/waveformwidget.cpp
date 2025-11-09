#include "waveformwidget.h"
#include <QPainter>
#include <QDebug>
#include <cmath>

WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget(parent)
    , m_dataBuffer(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_maxDisplayPoints(5000)
    , m_timeRange(10.0)
    , m_amplitudeMin(-1.0)
    , m_amplitudeMax(1.0)
    , m_autoScale(true)
    , m_backgroundColor(Qt::black)
    , m_gridColor(QColor(50, 50, 50))
    , m_axesColor(Qt::white)
    , m_leftMargin(60)
    , m_rightMargin(20)
    , m_topMargin(20)
    , m_bottomMargin(40)
    , m_displayingHistory(false)
{
    setMinimumSize(400, 300);
    setAutoFillBackground(true);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_backgroundColor);
    setPalette(pal);

    // 初始化通道颜色
    QList<QColor> colors = {
        Qt::blue, Qt::red, Qt::green, Qt::cyan,
        Qt::magenta, Qt::yellow, QColor(255, 128, 0), // 橙色
        QColor(128, 0, 255), // 紫色
        QColor(0, 255, 128), // 青绿色
        QColor(255, 0, 128), // 粉红色
        QColor(128, 255, 0), // 黄绿色
        QColor(0, 128, 255), // 天蓝色
        QColor(255, 255, 128) // 淡黄色
    };

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        m_channelVisible[i] = (i < 2); // 默认显示前两个通道
        m_channelColors[i] = colors[i % colors.size()];
    }

    connect(m_updateTimer, &QTimer::timeout, this, &WaveformWidget::updateDisplay);
    m_updateTimer->setInterval(50);  // 20Hz刷新率
}

WaveformWidget::~WaveformWidget()
{
}

void WaveformWidget::setDataBuffer(DataBuffer* buffer)
{
    m_dataBuffer = buffer;
}

void WaveformWidget::setChannelVisible(int channel, bool visible)
{
    if (channel >= 0 && channel < MAX_CHANNELS) {
        m_channelVisible[channel] = visible;
        update();
    }
}

bool WaveformWidget::isChannelVisible(int channel) const
{
    return m_channelVisible.value(channel, false);
}

void WaveformWidget::setUpdateInterval(int ms)
{
    m_updateTimer->setInterval(ms);
}

void WaveformWidget::setMaxDisplayPoints(int points)
{
    m_maxDisplayPoints = points;
}

void WaveformWidget::setTimeRange(double seconds)
{
    m_timeRange = seconds;
    calculateScales();
    update();
}

void WaveformWidget::setAmplitudeRange(double min, double max)
{
    m_amplitudeMin = min;
    m_amplitudeMax = max;
    m_autoScale = false;
    calculateScales();
    update();
}

void WaveformWidget::setAutoScale(bool enable)
{
    m_autoScale = enable;
}

void WaveformWidget::setDisplayData(int channel, const QVector<DataPoint>& data)
{
    if (channel >= 0 && channel < MAX_CHANNELS) {
        m_historyData[channel] = data;
        m_displayingHistory = true;
        update();
    }
}

void WaveformWidget::clearDisplayData()
{
    m_historyData.clear();
    m_displayingHistory = false;
    update();
}

void WaveformWidget::setChannelColor(int channel, const QColor& color)
{
    if (channel >= 0 && channel < MAX_CHANNELS) {
        m_channelColors[channel] = color;
        update();
    }
}

QColor WaveformWidget::getChannelColor(int channel)
{
    return m_channelColors.value(channel, Qt::white);
}

void WaveformWidget::startDisplay()
{
    m_updateTimer->start();
}

void WaveformWidget::stopDisplay()
{
    m_updateTimer->stop();
}

void WaveformWidget::clearDisplay()
{
    clearDisplayData();
}

void WaveformWidget::updateDisplay()
{
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 填充背景
    painter.fillRect(rect(), m_backgroundColor);

    // 计算缩放比例
    calculateScales();

    // 绘制网格
    drawGrid(painter);

    // 绘制坐标轴
    drawAxes(painter);

    // 绘制所有可见通道的波形
    for (int channel = 0; channel < MAX_CHANNELS; ++channel) {
        if (m_channelVisible.value(channel, false)) {
            drawWaveform(painter, channel);
        }
    }

    // 绘制标签
    drawLabels(painter);
}

void WaveformWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    calculateScales();
}

void WaveformWidget::drawGrid(QPainter& painter)
{
    painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));

    int drawWidth = width() - m_leftMargin - m_rightMargin;
    int drawHeight = height() - m_topMargin - m_bottomMargin;

    // 垂直网格线（时间）
    for (int i = 0; i <= 10; ++i) {
        int x = m_leftMargin + i * drawWidth / 10;
        painter.drawLine(x, m_topMargin, x, height() - m_bottomMargin);
    }

    // 水平网格线（幅值）
    for (int i = 0; i <= 8; ++i) {
        int y = m_topMargin + i * drawHeight / 8;
        painter.drawLine(m_leftMargin, y, width() - m_rightMargin, y);
    }
}

void WaveformWidget::drawAxes(QPainter& painter)
{
    painter.setPen(QPen(m_axesColor, 2));

    // X轴
    painter.drawLine(m_leftMargin, height() - m_bottomMargin,
                     width() - m_rightMargin, height() - m_bottomMargin);

    // Y轴
    painter.drawLine(m_leftMargin, m_topMargin,
                     m_leftMargin, height() - m_bottomMargin);
}

void WaveformWidget::drawWaveform(QPainter& painter, int channel)
{
    QVector<DataPoint> data;

    if (m_displayingHistory) {
        data = m_historyData.value(channel);
    } else if (m_dataBuffer) {
        data = m_dataBuffer->getChannelData(channel, m_maxDisplayPoints);
    }

    if (data.isEmpty()) return;

    // 设置画笔颜色
    QColor color = getChannelColor(channel);
    painter.setPen(QPen(color, 2));

    // 自动缩放
    if (m_autoScale && !data.isEmpty()) {
        double minAmp = data[0].amplitude;
        double maxAmp = data[0].amplitude;
        for (const auto& point : data) {
            minAmp = qMin(minAmp, point.amplitude);
            maxAmp = qMax(maxAmp, point.amplitude);
        }
        double range = maxAmp - minAmp;
        if (range > 0) {
            m_amplitudeMin = minAmp - range * 0.1;
            m_amplitudeMax = maxAmp + range * 0.1;
        }
    }

    // 绘制波形
    int drawHeight = height() - m_topMargin - m_bottomMargin;
    int drawWidth = width() - m_leftMargin - m_rightMargin;

    double timeMin = data.isEmpty() ? 0 : data.first().time;
    double timeMax = data.isEmpty() ? 1 : data.last().time;
    double timeRange = timeMax - timeMin;
    if (timeRange <= 0) timeRange = 1.0;

    QPainterPath path;
    bool firstPoint = true;

    for (const auto& point : data) {
        double normalizedTime = (point.time - timeMin) / timeRange;
        double normalizedAmp = (point.amplitude - m_amplitudeMin) /
                               (m_amplitudeMax - m_amplitudeMin);

        // 限制范围
        normalizedAmp = qBound(0.0, normalizedAmp, 1.0);

        int x = m_leftMargin + static_cast<int>(normalizedTime * drawWidth);
        int y = height() - m_bottomMargin -
                static_cast<int>(normalizedAmp * drawHeight);

        if (firstPoint) {
            path.moveTo(x, y);
            firstPoint = false;
        } else {
            path.lineTo(x, y);
        }
    }

    painter.drawPath(path);
}

void WaveformWidget::drawLabels(QPainter& painter)
{
    painter.setPen(m_axesColor);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    // Y轴标签（幅值）
    for (int i = 0; i <= 4; ++i) {
        double value = m_amplitudeMin +
                       (m_amplitudeMax - m_amplitudeMin) * i / 4.0;
        int y = height() - m_bottomMargin -
                (height() - m_topMargin - m_bottomMargin) * i / 4;

        QString text = QString::number(value, 'f', 2);
        painter.drawText(5, y + 5, text);
    }

    // X轴标签（时间）
    QVector<DataPoint> data;

    // 找到第一个有数据的通道
    for (int ch = 0; ch < MAX_CHANNELS; ++ch) {
        if (m_displayingHistory) {
            data = m_historyData.value(ch);
        } else if (m_dataBuffer) {
            data = m_dataBuffer->getChannelData(ch, m_maxDisplayPoints);
        }

        if (!data.isEmpty()) break;
    }

    if (!data.isEmpty()) {
        double timeMin = data.first().time;
        double timeMax = data.last().time;

        for (int i = 0; i <= 5; ++i) {
            double timeValue = timeMin + (timeMax - timeMin) * i / 5.0;
            int x = m_leftMargin +
                    (width() - m_leftMargin - m_rightMargin) * i / 5;

            QString text = QString::number(timeValue, 'f', 2) + "s";
            painter.drawText(x - 20, height() - 10, text);
        }
    }

    // 通道图例
    int legendX = width() - m_rightMargin - 150;
    int legendY = m_topMargin + 10;

    for (int channel = 0; channel < MAX_CHANNELS; ++channel) {
        if (m_channelVisible.value(channel, false)) {
            QColor color = getChannelColor(channel);
            painter.setPen(color);
            painter.drawLine(legendX, legendY, legendX + 30, legendY);
            painter.setPen(m_axesColor);
            painter.drawText(legendX + 35, legendY + 5, QString("通道%1").arg(channel));
            legendY += 20;

            // 如果图例太多，换到第二列
            if (legendY > height() - m_bottomMargin - 40) {
                legendX -= 100;
                legendY = m_topMargin + 10;
            }
        }
    }
}

void WaveformWidget::calculateScales()
{
    int drawWidth = width() - m_leftMargin - m_rightMargin;
    int drawHeight = height() - m_topMargin - m_bottomMargin;

    if (drawWidth > 0) {
        m_xScale = drawWidth / m_timeRange;
    }

    if (drawHeight > 0 && (m_amplitudeMax - m_amplitudeMin) > 0) {
        m_yScale = drawHeight / (m_amplitudeMax - m_amplitudeMin);
    }
}
