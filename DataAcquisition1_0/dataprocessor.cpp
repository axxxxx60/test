#include "dataprocessor.h"
#include <cmath>
#include <algorithm>

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
{
}

DataProcessor::~DataProcessor()
{
}

QVector<DataPoint> DataProcessor::applyLowPassFilter(const QVector<DataPoint>& data,
                                                     double cutoffFreq, double sampleRate)
{
    if (data.isEmpty()) return data;

    QVector<DataPoint> filtered = data;
    double alpha = calculateAlpha(cutoffFreq, sampleRate);

    for (int i = 1; i < filtered.size(); ++i) {
        filtered[i].amplitude = alpha * data[i].amplitude +
                                (1 - alpha) * filtered[i-1].amplitude;
    }

    return filtered;
}

QVector<DataPoint> DataProcessor::applyHighPassFilter(const QVector<DataPoint>& data,
                                                      double cutoffFreq, double sampleRate)
{
    if (data.isEmpty()) return data;

    QVector<DataPoint> filtered = data;
    double alpha = calculateAlpha(cutoffFreq, sampleRate);

    filtered[0].amplitude = data[0].amplitude;
    for (int i = 1; i < filtered.size(); ++i) {
        filtered[i].amplitude = alpha * (filtered[i-1].amplitude +
                                         data[i].amplitude - data[i-1].amplitude);
    }

    return filtered;
}

QVector<DataPoint> DataProcessor::downsample(const QVector<DataPoint>& data, int factor)
{
    if (data.isEmpty() || factor <= 1) return data;

    QVector<DataPoint> downsampled;
    for (int i = 0; i < data.size(); i += factor) {
        downsampled.append(data[i]);
    }

    return downsampled;
}

QVector<DataPoint> DataProcessor::movingAverageSmooth(const QVector<DataPoint>& data,
                                                      int windowSize)
{
    if (data.isEmpty() || windowSize <= 1) return data;

    QVector<DataPoint> smoothed = data;
    int halfWindow = windowSize / 2;

    for (int i = 0; i < data.size(); ++i) {
        int start = qMax(0, i - halfWindow);
        int end = qMin(data.size() - 1, i + halfWindow);

        double sum = 0.0;
        int count = 0;
        for (int j = start; j <= end; ++j) {
            sum += data[j].amplitude;
            count++;
        }

        smoothed[i].amplitude = sum / count;
    }

    return smoothed;
}

QVector<DataPoint> DataProcessor::normalizeData(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) return data;

    double minAmp = data[0].amplitude;
    double maxAmp = data[0].amplitude;

    for (const auto& point : data) {
        minAmp = qMin(minAmp, point.amplitude);
        maxAmp = qMax(maxAmp, point.amplitude);
    }

    double range = maxAmp - minAmp;
    if (range == 0) return data;

    QVector<DataPoint> normalized = data;
    for (auto& point : normalized) {
        point.amplitude = (point.amplitude - minAmp) / range;
    }

    return normalized;
}

QVector<DataPoint> DataProcessor::scaleData(const QVector<DataPoint>& data,
                                            double scaleX, double scaleY)
{
    QVector<DataPoint> scaled = data;
    for (auto& point : scaled) {
        point.time *= scaleX;
        point.amplitude *= scaleY;
    }
    return scaled;
}

double DataProcessor::calculateAlpha(double cutoffFreq, double sampleRate)
{
    double rc = 1.0 / (2.0 * M_PI * cutoffFreq);
    double dt = 1.0 / sampleRate;
    return dt / (rc + dt);
}
