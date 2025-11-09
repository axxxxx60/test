#include "dataanalyzer.h"
#include <cmath>
#include <algorithm>
#include <QDebug>
#include <QDateTime>

DataAnalyzer::DataAnalyzer(QObject *parent)
    : QObject(parent)
{
}

DataAnalyzer::~DataAnalyzer()
{
}

double DataAnalyzer::calculateMax(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return 0.0;
    }

    double maxVal = data[0].amplitude;
    for (const auto& point : data) {
        if (point.amplitude > maxVal) {
            maxVal = point.amplitude;
        }
    }

    return maxVal;
}

double DataAnalyzer::calculateMin(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return 0.0;
    }

    double minVal = data[0].amplitude;
    for (const auto& point : data) {
        if (point.amplitude < minVal) {
            minVal = point.amplitude;
        }
    }

    return minVal;
}

double DataAnalyzer::calculateMean(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const auto& point : data) {
        sum += point.amplitude;
    }

    return sum / data.size();
}

double DataAnalyzer::calculateRMS(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return 0.0;
    }

    double sumSquares = 0.0;
    for (const auto& point : data) {
        sumSquares += point.amplitude * point.amplitude;
    }

    return std::sqrt(sumSquares / data.size());
}

double DataAnalyzer::calculateStdDev(const QVector<DataPoint>& data)
{
    if (data.size() < 2) {
        return 0.0;
    }

    double mean = calculateMean(data);
    double sumSquaredDiff = 0.0;

    for (const auto& point : data) {
        double diff = point.amplitude - mean;
        sumSquaredDiff += diff * diff;
    }

    return std::sqrt(sumSquaredDiff / (data.size() - 1));
}

int DataAnalyzer::nextPowerOf2(int n)
{
    int power = 1;
    while (power < n) {
        power *= 2;
    }
    return power;
}

int DataAnalyzer::reverseBits(int num, int bits)
{
    int reversed = 0;
    for (int i = 0; i < bits; ++i) {
        reversed = (reversed << 1) | (num & 1);
        num >>= 1;
    }
    return reversed;
}

void DataAnalyzer::fft(QVector<std::complex<double>>& data)
{
    int n = data.size();
    if (n <= 1) return;

    // 计算所需的位数
    int bits = 0;
    int temp = n;
    while (temp > 1) {
        temp >>= 1;
        bits++;
    }

    // 位反转排序
    for (int i = 0; i < n; ++i) {
        int j = reverseBits(i, bits);
        if (j > i) {
            std::swap(data[i], data[j]);
        }
    }

    // FFT 蝶形运算
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        std::complex<double> wlen(std::cos(angle), std::sin(angle));

        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);

            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = data[i + j];
                std::complex<double> v = data[i + j + len / 2] * w;

                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;

                w *= wlen;
            }
        }
    }
}

QVector<double> DataAnalyzer::calculateFFT(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return QVector<double>();
    }

    emit analysisProgress(10, "准备FFT数据...");

    // 确定FFT大小（2的幂次）
    int fftSize = nextPowerOf2(data.size());

    // 准备复数数据
    QVector<std::complex<double>> complexData(fftSize);
    for (int i = 0; i < data.size(); ++i) {
        complexData[i] = std::complex<double>(data[i].amplitude, 0.0);
    }
    // 零填充
    for (int i = data.size(); i < fftSize; ++i) {
        complexData[i] = std::complex<double>(0.0, 0.0);
    }

    emit analysisProgress(30, "执行FFT变换...");

    // 执行FFT
    fft(complexData);

    emit analysisProgress(60, "计算频谱幅值...");

    // 计算幅值谱（只取前半部分，因为FFT结果是对称的）
    QVector<double> magnitude(fftSize / 2);
    for (int i = 0; i < fftSize / 2; ++i) {
        magnitude[i] = std::abs(complexData[i]);
    }

    emit analysisProgress(90, "FFT计算完成");

    return magnitude;
}

QVector<double> DataAnalyzer::calculatePowerSpectrum(const QVector<DataPoint>& data)
{
    if (data.isEmpty()) {
        return QVector<double>();
    }

    emit analysisProgress(10, "计算功率谱...");

    // 获取FFT结果
    QVector<double> fftMagnitude = calculateFFT(data);

    emit analysisProgress(70, "计算功率值...");

    // 计算功率谱（幅值的平方）
    QVector<double> powerSpectrum(fftMagnitude.size());
    for (int i = 0; i < fftMagnitude.size(); ++i) {
        powerSpectrum[i] = fftMagnitude[i] * fftMagnitude[i];
    }

    emit analysisProgress(100, "功率谱计算完成");

    return powerSpectrum;
}

double DataAnalyzer::calculateDominantFrequency(const QVector<DataPoint>& data,
                                                double sampleRate)
{
    if (data.size() < 2) {
        return 0.0;
    }

    emit analysisProgress(10, "分析主频率...");

    // 计算FFT
    QVector<double> fftMagnitude = calculateFFT(data);

    emit analysisProgress(70, "查找峰值频率...");

    if (fftMagnitude.isEmpty()) {
        return 0.0;
    }

    // 查找最大幅值的索引（跳过直流分量，从索引1开始）
    int maxIndex = 1;
    double maxValue = fftMagnitude[1];

    for (int i = 2; i < fftMagnitude.size(); ++i) {
        if (fftMagnitude[i] > maxValue) {
            maxValue = fftMagnitude[i];
            maxIndex = i;
        }
    }

    // 计算频率分辨率
    int fftSize = nextPowerOf2(data.size());
    double frequencyResolution = sampleRate / fftSize;

    // 计算主频率
    double dominantFreq = maxIndex * frequencyResolution;

    emit analysisProgress(100, "主频率分析完成");

    qDebug() << "主频率:" << dominantFreq << "Hz (索引:" << maxIndex
             << ", 幅值:" << maxValue << ")";

    return dominantFreq;
}

AnalysisResult DataAnalyzer::performFullAnalysis(const QVector<DataPoint>& data,
                                                 int taskId, int channel,
                                                 double sampleRate)
{
    AnalysisResult result;
    result.taskId = taskId;
    result.channel = channel;

    if (data.isEmpty()) {
        qWarning() << "数据为空，无法进行分析";
        return result;
    }

    emit analysisProgress(0, "开始数据分析...");

    // 统计分析
    emit analysisProgress(10, "计算统计参数...");
    result.maxAmplitude = calculateMax(data);

    emit analysisProgress(20, "计算最小值...");
    result.minAmplitude = calculateMin(data);

    emit analysisProgress(30, "计算平均值...");
    result.avgAmplitude = calculateMean(data);

    emit analysisProgress(40, "计算均方根值...");
    result.rmsValue = calculateRMS(data);

    // 频域分析
    emit analysisProgress(50, "进行频域分析...");
    result.frequency = calculateDominantFrequency(data, sampleRate);

    // 设置分析时间
    result.analysisTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    emit analysisProgress(100, "分析完成");

    // 输出分析结果
    qDebug() << "=== 分析结果 ===";
    qDebug() << "任务ID:" << result.taskId;
    qDebug() << "通道:" << result.channel;
    qDebug() << "最大幅值:" << result.maxAmplitude;
    qDebug() << "最小幅值:" << result.minAmplitude;
    qDebug() << "平均幅值:" << result.avgAmplitude;
    qDebug() << "均方根值:" << result.rmsValue;
    qDebug() << "主频率:" << result.frequency << "Hz";
    qDebug() << "分析时间:" << result.analysisTime;

    emit analysisCompleted(result);

    return result;
}
