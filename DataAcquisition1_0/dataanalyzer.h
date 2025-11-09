#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <QObject>
#include <QVector>
#include <complex>
#include "databuffer.h"
#include "databasemanager.h"

class DataAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit DataAnalyzer(QObject *parent = nullptr);
    ~DataAnalyzer();

    // 统计分析
    double calculateMax(const QVector<DataPoint>& data);
    double calculateMin(const QVector<DataPoint>& data);
    double calculateMean(const QVector<DataPoint>& data);
    double calculateRMS(const QVector<DataPoint>& data);
    double calculateStdDev(const QVector<DataPoint>& data);

    // 频域分析
    double calculateDominantFrequency(const QVector<DataPoint>& data, double sampleRate);
    QVector<double> calculateFFT(const QVector<DataPoint>& data);
    QVector<double> calculatePowerSpectrum(const QVector<DataPoint>& data);

    // 完整分析（生成分析结果结构）
    AnalysisResult performFullAnalysis(const QVector<DataPoint>& data,
                                       int taskId, int channel, double sampleRate);

signals:
    void analysisProgress(int percentage, const QString& message);
    void analysisCompleted(const AnalysisResult& result);

private:
    void fft(QVector<std::complex<double>>& data);
    int reverseBits(int num, int bits);
    int nextPowerOf2(int n);
};

#endif // DATAANALYZER_H
