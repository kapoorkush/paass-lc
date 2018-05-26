/// @file RootFitter.cpp
/// @brief Class to handle fitting traces using ROOT
/// @author S. V. Paulauskas
/// @date December 18, 2016
#include "RootFitter.hpp"
#include "TimingConfiguration.hpp"
#include "VandleTimingFunction.hpp"

#include <TF1.h>
#include <TGraph.h>

#include <stdexcept>

using namespace std;

RootFitter::RootFitter() {
    vandleTimingFunction_ = new VandleTimingFunction();
    func_ = new TF1("func", vandleTimingFunction_, 0., 1.e6, 5);
}

RootFitter::~RootFitter() {
    delete vandleTimingFunction_;
    delete func_;
}

double RootFitter::CalculatePhase(const std::vector<double> &data, const TimingConfiguration &cfg,
                                  const std::pair<unsigned int, double> &maxInfo, std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("RootFitter::CalculatePhase - The data was sized zero.");

    vector<double> xvals;
    for (unsigned int i = 0; i < data.size(); i++)
        xvals.push_back(double(i));

    TGraph graph((int) data.size(), &(xvals[0]), &(data[0]));

    func_->SetParameters(0, cfg.GetQdc() * 0.5);
    func_->FixParameter(2, cfg.GetBeta());
    func_->FixParameter(3, cfg.GetGamma());
    func_->FixParameter(4, 0.0);

    graph.Fit(func_, "WRQ", "", 0, data.size());

    return func_->GetParameter(0);
}