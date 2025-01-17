#define _USE_MATH_DEFINES // Windows
#include <omp.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <string>
#include <algorithm>
#include <numeric>

// Structure pour les statistiques
template<typename T>
struct TimingStats {
    T mean;
    T min;
    T max;
    T stddev;

    TimingStats(T m = 0, T mn = 0, T mx = 0, T sd = 0)
        : mean(m), min(mn), max(mx), stddev(sd) {}
};

class QgsPoint {
private:
    double mX, mY, mZ, mM;
    bool m3D, mMeasure;

public:
    QgsPoint(double x = 0, double y = 0, double z = 0, double m = 0)
        : mX(x), mY(y), mZ(z), mM(m), m3D(false), mMeasure(false) {}

    double x() const { return mX; }
    double y() const { return mY; }
    double z() const { return mZ; }
    double m() const { return mM; }
    bool is3D() const { return m3D; }
    bool isMeasure() const { return mMeasure; }

    QgsPoint project(double distance, double azimuth) const {
        double dx = distance * std::cos(azimuth);
        double dy = distance * std::sin(azimuth);
        return QgsPoint(mX + dx, mY + dy, mZ, mM);
    }
};

class QgsEllipse {
private:
    QgsPoint mCenter;
    double mSemiMajorAxis;
    double mSemiMinorAxis;
    double mAzimuth;

public:
    QgsEllipse(const QgsPoint& center, double semiMajorAxis, double semiMinorAxis, double azimuth)
        : mCenter(center), mSemiMajorAxis(semiMajorAxis), mSemiMinorAxis(semiMinorAxis), mAzimuth(azimuth) {}

    // Méthode 1 originale
    void pointsInternalMethod1(unsigned int segments, std::vector<double>& x, std::vector<double>& y,
        std::vector<double>& z, std::vector<double>& m) const {
        if (segments < 3) return;

        const double centerX = mCenter.x();
        const double centerY = mCenter.y();
        const double centerZ = mCenter.z();
        const double centerM = mCenter.m();
        const bool hasZ = mCenter.is3D();
        const bool hasM = mCenter.isMeasure();

        const QgsPoint p1 = mCenter.project(mSemiMajorAxis, mAzimuth);
        const double azimuth = std::atan2(p1.y() - mCenter.y(), p1.x() - mCenter.x());
        const double cosAzimuth = std::cos(azimuth);
        const double sinAzimuth = std::sin(azimuth);

        std::vector<double> cosT(segments), sinT(segments);
        for (unsigned int i = 0; i < segments; ++i) {
            double angle = 2 * M_PI - ((2 * M_PI) / segments * i);
            cosT[i] = std::cos(angle);
            sinT[i] = std::sin(angle);
        }

        x.resize(segments);
        y.resize(segments);
        if (hasZ) z.resize(segments);
        if (hasM) m.resize(segments);

        double* xOut = x.data();
        double* yOut = y.data();
        double* zOut = hasZ ? z.data() : nullptr;
        double* mOut = hasM ? m.data() : nullptr;

        for (unsigned int i = 0; i < segments; ++i) {
            *xOut++ = centerX + mSemiMajorAxis * cosT[i] * cosAzimuth -
                mSemiMinorAxis * sinT[i] * sinAzimuth;
            *yOut++ = centerY + mSemiMajorAxis * cosT[i] * sinAzimuth +
                mSemiMinorAxis * sinT[i] * cosAzimuth;
            if (zOut) *zOut++ = centerZ;
            if (mOut) *mOut++ = centerM;
        }
    }

    // Méthode 2 originale
    void pointsInternalMethod2(unsigned int segments, std::vector<double>& x, std::vector<double>& y,
        std::vector<double>& z, std::vector<double>& m) const {
        if (segments < 3) return;

        const double centerX = mCenter.x();
        const double centerY = mCenter.y();
        const double centerZ = mCenter.z();
        const double centerM = mCenter.m();
        const bool hasZ = mCenter.is3D();
        const bool hasM = mCenter.isMeasure();

        std::vector<double> t(segments);
        const QgsPoint p1 = mCenter.project(mSemiMajorAxis, mAzimuth);
        const double azimuth = std::atan2(p1.y() - mCenter.y(), p1.x() - mCenter.x());

        for (unsigned int i = 0; i < segments; ++i) {
            t[i] = 2 * M_PI - ((2 * M_PI) / segments * i);
        }

        x.resize(segments);
        y.resize(segments);
        if (hasZ) z.resize(segments);
        if (hasM) m.resize(segments);

        double* xOut = x.data();
        double* yOut = y.data();
        double* zOut = hasZ ? z.data() : nullptr;
        double* mOut = hasM ? m.data() : nullptr;

        const double cosAzimuth = std::cos(azimuth);
        const double sinAzimuth = std::sin(azimuth);

        for (double it : t) {
            const double cosT{ std::cos(it) };
            const double sinT{ std::sin(it) };
            *xOut++ = centerX + mSemiMajorAxis * cosT * cosAzimuth -
                mSemiMinorAxis * sinT * sinAzimuth;
            *yOut++ = centerY + mSemiMajorAxis * cosT * sinAzimuth +
                mSemiMinorAxis * sinT * cosAzimuth;
            if (zOut) *zOut++ = centerZ;
            if (mOut) *mOut++ = centerM;
        }
    }

    // Méthode 1 avec OpenMP
    void pointsInternalMethod1OMP(unsigned int segments, std::vector<double>& x, std::vector<double>& y,
        std::vector<double>& z, std::vector<double>& m) const {
        if (segments < 3) return;

        const double centerX = mCenter.x();
        const double centerY = mCenter.y();
        const double centerZ = mCenter.z();
        const double centerM = mCenter.m();
        const bool hasZ = mCenter.is3D();
        const bool hasM = mCenter.isMeasure();

        const QgsPoint p1 = mCenter.project(mSemiMajorAxis, mAzimuth);
        const double azimuth = std::atan2(p1.y() - mCenter.y(), p1.x() - mCenter.x());
        const double cosAzimuth = std::cos(azimuth);
        const double sinAzimuth = std::sin(azimuth);

        std::vector<double> cosT(segments), sinT(segments);

#pragma omp parallel for schedule(static)
        for (int i = 0; i < static_cast<int>(segments); ++i) {
            double angle = 2 * M_PI - ((2 * M_PI) / segments * i);
            cosT[i] = std::cos(angle);
            sinT[i] = std::sin(angle);
        }

        x.resize(segments);
        y.resize(segments);
        if (hasZ) z.resize(segments);
        if (hasM) m.resize(segments);

#pragma omp parallel for schedule(static)
        for (int i = 0; i < static_cast<int>(segments); ++i) {
            x[i] = centerX + mSemiMajorAxis * cosT[i] * cosAzimuth -
                mSemiMinorAxis * sinT[i] * sinAzimuth;
            y[i] = centerY + mSemiMajorAxis * cosT[i] * sinAzimuth +
                mSemiMinorAxis * sinT[i] * cosAzimuth;
            if (hasZ) z[i] = centerZ;
            if (hasM) m[i] = centerM;
        }
    }

    // Méthode 2 avec OpenMP
    void pointsInternalMethod2OMP(unsigned int segments, std::vector<double>& x, std::vector<double>& y,
        std::vector<double>& z, std::vector<double>& m) const {
        if (segments < 3) return;

        const double centerX = mCenter.x();
        const double centerY = mCenter.y();
        const double centerZ = mCenter.z();
        const double centerM = mCenter.m();
        const bool hasZ = mCenter.is3D();
        const bool hasM = mCenter.isMeasure();

        std::vector<double> t(segments);
        const QgsPoint p1 = mCenter.project(mSemiMajorAxis, mAzimuth);
        const double azimuth = std::atan2(p1.y() - mCenter.y(), p1.x() - mCenter.x());
        const double cosAzimuth = std::cos(azimuth);
        const double sinAzimuth = std::sin(azimuth);

#pragma omp parallel for schedule(static)
        for (int i = 0; i < static_cast<int>(segments); ++i) {
            t[i] = 2 * M_PI - ((2 * M_PI) / segments * i);
        }

        x.resize(segments);
        y.resize(segments);
        if (hasZ) z.resize(segments);
        if (hasM) m.resize(segments);

#pragma omp parallel for schedule(static)
        for (int i = 0; i < static_cast<int>(segments); ++i) {
            const double cosT{ std::cos(t[i]) };
            const double sinT{ std::sin(t[i]) };
            x[i] = centerX + mSemiMajorAxis * cosT * cosAzimuth -
                mSemiMinorAxis * sinT * sinAzimuth;
            y[i] = centerY + mSemiMajorAxis * cosT * sinAzimuth +
                mSemiMinorAxis * sinT * cosAzimuth;
            if (hasZ) z[i] = centerZ;
            if (hasM) m[i] = centerM;
        }
    }

    // Méthode 2 bis
    void pointsInternalMethod2BIS(unsigned int segments, std::vector<double>& x, std::vector<double>& y,
        std::vector<double>& z, std::vector<double>& m) const {
        if (segments < 3) return;

        const double centerX = mCenter.x();
        const double centerY = mCenter.y();
        const double centerZ = mCenter.z();
        const double centerM = mCenter.m();
        const bool hasZ = mCenter.is3D();
        const bool hasM = mCenter.isMeasure();

        std::vector<double> t(segments);
        const QgsPoint p1 = mCenter.project(mSemiMajorAxis, mAzimuth);
        const double azimuth = std::atan2(p1.y() - mCenter.y(), p1.x() - mCenter.x());
        const double cosAzimuth = std::cos(azimuth);
        const double sinAzimuth = std::sin(azimuth);

        for (int i = 0; i < static_cast<int>(segments); ++i) {
            t[i] = 2 * M_PI - ((2 * M_PI) / segments * i);
        }

        x.resize(segments);
        y.resize(segments);
        if (hasZ) z.resize(segments);
        if (hasM) m.resize(segments);

        for (int i = 0; i < static_cast<int>(segments); ++i) {
            const double cosT{ std::cos(t[i]) };
            const double sinT{ std::sin(t[i]) };
            x[i] = centerX + mSemiMajorAxis * cosT * cosAzimuth -
                mSemiMinorAxis * sinT * sinAzimuth;
            y[i] = centerY + mSemiMajorAxis * cosT * sinAzimuth +
                mSemiMinorAxis * sinT * cosAzimuth;
            if (hasZ) z[i] = centerZ;
            if (hasM) m[i] = centerM;
        }
    }
};


// Fonction de mesure détaillée
template<typename Func>
TimingStats<double> measureExecutionTimeDetailed(Func&& func, int iterations = 1000) {
    // Warmup
    for (int i = 0; i < 100; ++i) {
        func();
    }

    std::vector<double> times(iterations);

    // Mesures
    for (int i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> duration = end - start;
        times[i] = duration.count();
    }

    // Calcul des statistiques
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double mean = sum / iterations;

    double min = *std::min_element(times.begin(), times.end());
    double max = *std::max_element(times.begin(), times.end());

    double sq_sum = std::inner_product(times.begin(), times.end(), times.begin(), 0.0);
    double stddev = std::sqrt(sq_sum / iterations - mean * mean);

    return TimingStats<double>(mean, min, max, stddev);
}

int main(int argc, char* argv[]) {
    int num_threads = (argc > 1) ? std::atoi(argv[1]) : omp_get_max_threads();
    omp_set_num_threads(num_threads);

    std::cout << "Threads : " << num_threads << std::endl;

    // Paramètres de test
    QgsPoint center(0, 0);
    double semiMajorAxis = 10.0;
    double semiMinorAxis = 5.0;
    double azimuth = M_PI / 4; // 45 degrés
    QgsEllipse ellipse(center, semiMajorAxis, semiMinorAxis, azimuth);

    // Vecteurs pour stocker les résultats
    std::vector<double> x, y, z, m;

    // Tests avec différents nombres de segments
    std::vector<unsigned int> segmentCounts;
    for (unsigned int segments = 4; segments <= 524288; segments *= 2) {
        segmentCounts.push_back(segments);
    }

    // Structure pour stocker les statistiques moyennes par méthode
    struct MethodStats {
        std::string method;
        double mean_time;
        MethodStats(const std::string& m, double t) : method(m), mean_time(t) {}
    };

    // En-tête CSV
    std::cout << "segments,method,mean,min,max,stddev\n";

    for (unsigned int segments : segmentCounts) {
        std::vector<MethodStats> all_stats;

        auto printStats = [segments](const std::string& method, const TimingStats<double>& stats) {
            std::cout << segments << ","
                << method << ","
                << std::fixed << std::setprecision(9)
                << stats.mean << ","
                << stats.min << ","
                << stats.max << ","
                << stats.stddev << "\n";
            };

        auto stats1 = measureExecutionTimeDetailed([&]() {
            ellipse.pointsInternalMethod1(segments, x, y, z, m);
            });
        printStats("method1", stats1);
        all_stats.emplace_back("method1", stats1.mean);

        auto stats2 = measureExecutionTimeDetailed([&]() {
            ellipse.pointsInternalMethod2(segments, x, y, z, m);
            });
        printStats("method2", stats2);
        all_stats.emplace_back("method2", stats2.mean);

        auto stats1OMP = measureExecutionTimeDetailed([&]() {
            ellipse.pointsInternalMethod1OMP(segments, x, y, z, m);
            });
        printStats("method1_omp", stats1OMP);
        all_stats.emplace_back("method1_omp", stats1OMP.mean);

        auto stats2OMP = measureExecutionTimeDetailed([&]() {
            ellipse.pointsInternalMethod2OMP(segments, x, y, z, m);
            });
        printStats("method2_omp", stats2OMP);
        all_stats.emplace_back("method2_omp", stats2OMP.mean);

        auto stats2BIS = measureExecutionTimeDetailed([&]() {
            ellipse.pointsInternalMethod2BIS(segments, x, y, z, m);
            });
        printStats("method2_bis", stats2BIS);
        all_stats.emplace_back("method2_bis", stats2BIS.mean);

        // Trouver la méthode la plus rapide
        auto fastest = std::min_element(all_stats.begin(), all_stats.end(),
            [](const MethodStats& a, const MethodStats& b) {
                return a.mean_time < b.mean_time;
            });

        std::cout << "* " << segments << ","
            << fastest->method << ","
            << std::fixed << std::setprecision(9) << fastest->mean_time << "\n";
    }

    return 0;
}
