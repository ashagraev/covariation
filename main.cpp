#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
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

class ICovariationCalculator {
private:
public:
    virtual void Add(const double x, const double y) = 0;
    virtual double Covariation() const = 0;
    virtual std::string Name() const = 0;
};

template <class TAccumulatorType>
class TTypedCovariationCalculator : public ICovariationCalculator {
private:
    size_t Count = 0;
    TAccumulatorType SumX = 0.;
    TAccumulatorType SumY = 0.;
    TAccumulatorType SumProducts = 0.;
public:
    void Add(const double x, const double y) override {
        ++Count;
        SumX += x;
        SumY += y;
        SumProducts += x * y;
    }

    double Covariation() const override {
        return ((double) SumProducts - (double) SumX * (double) SumY / Count) / Count;
    }

    std::string Name() const override;
};

using TDummyCovariationCalculator = TTypedCovariationCalculator<double>;
using TKahanCovariationCalculator = TTypedCovariationCalculator<TKahanAccumulator>;

template <>
std::string TDummyCovariationCalculator::Name() const {
    return "Dummy";
};

template <>
std::string TKahanCovariationCalculator::Name() const {
    return "Kahan";
};

class TWelfordCovariationCalculator : public ICovariationCalculator {
private:
    size_t Count = 0;
    double MeanX = 0.;
    double MeanY = 0.;
    double SumProducts = 0.;
public:
    void Add(const double x, const double y) override {
        ++Count;
        MeanX += (x - MeanX) / Count;
        SumProducts += (x - MeanX) * (y - MeanY);
        MeanY += (y - MeanY) / Count;
    }

    double Covariation() const override {
        return SumProducts / Count;
    }

    std::string Name() const override {
        return "Welford";
    }
};

double Error(const double target, const double value) {
    return fabs(value - target) / fabs(target);
}

class TPrinter {
private:
    std::string Title;

    std::vector<std::string> Columns;
    std::vector<std::vector<std::string>> Rows;
public:
    TPrinter(const std::string& title)
        : Title(title)
    {
    }

    void AddColumn(const std::string& name) {
        Columns.push_back(name);
    }

    void AddRow() {
        Rows.push_back(std::vector<std::string>());
    }

    template <class TValue>
    void AddToRow(const TValue& value) {
        std::stringstream ss;
        ss.precision(10);
        ss << value;
        Rows.back().push_back(ss.str());
    }

    void Print() const {
        auto printRow = [&](const std::vector<std::string>& row){
            const size_t columnWidth = 25;

            std::stringstream rowStream;
            for (const std::string& element : row) {
                PrintNextColumn(rowStream, element, columnWidth);
            }

            printf("%s\n", rowStream.str().c_str());
        };

        printf("%s\n", Title.c_str());
        printRow(Columns);
        for (const std::vector<std::string>& row : Rows) {
            printRow(row);
        }
    }
private:
    template <class TValue>
    void PrintNextColumn(std::stringstream& out, const TValue& value, size_t columnWidth) const {
        const size_t startLength = out.str().size();
        out << value;
        const size_t currentLength = out.str().size();
        for (size_t i = currentLength; i < startLength + columnWidth; ++i) {
            out << " ";
        }
    }
};

int main() {
    double interestingMeans[] = { 100000, 10000000 };

    for (const double mean : interestingMeans) {
        const double xMean = mean;
        const double yMean = mean;

        double xDiff = 1;
        double yDiff = 1;

        const double actualCovariation = xDiff * yDiff;

        std::vector<std::shared_ptr<ICovariationCalculator>> calculators;
        calculators.push_back(std::shared_ptr<TDummyCovariationCalculator>(new TDummyCovariationCalculator()));
        calculators.push_back(std::shared_ptr<TKahanCovariationCalculator>(new TKahanCovariationCalculator()));
        calculators.push_back(std::shared_ptr<TWelfordCovariationCalculator>(new TWelfordCovariationCalculator()));

        TPrinter printer("mean: " + std::to_string(mean));
        printer.AddColumn("Count");
        for (auto&& calculator : calculators) {
            printer.AddColumn(calculator->Name());
        }

        std::vector<double> maxErrors(calculators.size(), 0.);

        const size_t count = 10000000;
        for (size_t i = 0; i < count; ++i) {
            if (i && (i % (count / 100) == 0)) {
                printer.AddRow();
                printer.AddToRow(i);
                for (size_t calculatorIdx = 0; calculatorIdx < calculators.size(); ++calculatorIdx) {
                    const double calculatedCovariation = calculators[calculatorIdx]->Covariation();
                    const double error = Error(actualCovariation, calculatedCovariation) * 100;
                    printer.AddToRow(error);

                    maxErrors[calculatorIdx] = std::max(maxErrors[calculatorIdx], error);
                }
            }

            xDiff = -xDiff;
            yDiff = -yDiff;

            for (auto&& calculator : calculators) {
                calculator->Add(xMean + xDiff, yMean + yDiff);
            }
        }

        printer.AddRow();
        printer.AddToRow("MaxError");
        for (size_t i = 0; i < calculators.size(); ++i) {
            printer.AddToRow(maxErrors[i]);
        }

        printer.Print();
        printf("\n\n");
    }

    return 0;
}
