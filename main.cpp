#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

class TKahanAccumulator {
private:
    double Sum;
    double Addition;
public:
    TKahanAccumulator(const double value = 0.)
        : Sum(value)
        , Addition(0.)
    {
    }

    TKahanAccumulator& operator += (const double value) {
        const double y = value - Addition;
        const double t = Sum + y;
        Addition = (t - Sum) - y;
        Sum = t;
        return *this;
    }

    TKahanAccumulator& operator += (const TKahanAccumulator& other) {
        return *this += (double)other;
    }

    operator double() const {
        return Sum + Addition;
    }
};

double DummyCovariation(const std::vector<double>& x, const std::vector<double>& y) {
    const double sumCrossProducts = std::inner_product(x.begin(), x.end(), y.begin(), 0.);
    const double sumX = std::accumulate(x.begin(), x.end(), 0.);
    const double sumY = std::accumulate(y.begin(), y.end(), 0.);

    return (sumCrossProducts - sumX * sumY / x.size()) / x.size();
}

double KahanCovariation(const std::vector<double>& x, const std::vector<double>& y) {
    TKahanAccumulator sumCrossProducts;
    TKahanAccumulator sumX;
    TKahanAccumulator sumY;

    for (size_t i = 0; i < x.size(); ++i) {
        sumCrossProducts += x[i] * y[i];
        sumX += x[i];
        sumY += y[i];
    }

    return (sumCrossProducts - sumX * sumY / x.size()) / x.size();
}

double SmartCovariation(const std::vector<double>& x, const std::vector<double>& y) {
    double sumProducts = 0.;
    double xMean = 0.;
    double yMean = 0.;

    for (size_t i = 0; i < x.size(); ++i) {
        xMean += (x[i] - xMean) / (i + 1);
        sumProducts += (x[i] - xMean) * (y[i] - yMean);
        yMean += (y[i] - yMean) / (i + 1);
    }

    return sumProducts / x.size();
}

double Error(const double target, const double value) {
    return fabs(value - target) / std::max(1., fabs(target));
}

int main() {
    size_t interestingSizes[] = { 100000, 1000000, 5000000 };
    double interestingMeans[] = { 1, 1000, 1000000 };

    for (const double mean : interestingMeans) {
        std::vector<double> x;
        std::vector<double> y;

        const double xMean = mean;
        const double yMean = mean;

        double xDiff = 1e-2;
        double yDiff = 1e-2;

        const double actualCovariation = xDiff * yDiff;

        printf("mean: %10lf\n", mean);
        for (size_t interestingSize : interestingSizes) {
            x.reserve(interestingSize);
            y.reserve(interestingSize);

            for (size_t i = x.size(); i < interestingSize; ++i) {
                x.push_back(xMean + xDiff);
                y.push_back(yMean + yDiff);

                xDiff = -xDiff;
                yDiff = -yDiff;
            }

            const double dummyCovariation = DummyCovariation(x, y);
            const double kahanCovariation = KahanCovariation(x, y);
            const double smartCovariation = SmartCovariation(x, y);

            const double dummyError = Error(actualCovariation, dummyCovariation);
            const double kahanError = Error(actualCovariation, kahanCovariation);
            const double smartError = Error(actualCovariation, smartCovariation);

            printf("\tsize: %lu\n", interestingSize);
            printf("\t\tactual covariation: %.10lf\n", actualCovariation);
            printf("\t\tsmart covariation:  %.10lf (error: %.10lf)\n", smartCovariation, smartError);
            printf("\t\tkahan covariation:  %.10lf (error: %.10lf)\n", kahanCovariation, kahanError);
            printf("\t\tdummy covariation:  %.10lf (error: %.10lf)\n", dummyCovariation, dummyError);
            printf("\n");
        }
    }

    return 0;
}
