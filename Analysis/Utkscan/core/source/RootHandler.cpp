/** \file RootHandler.cpp
 * \brief Implemenation of class to dump event info to a root tree
 * \author David Miller and S. V. Paulauskas
 * \date Jan 2010
 */
#include "RootHandler.hpp"

#include <iostream>
#include <thread>

using namespace std;

RootHandler *RootHandler::instance_ = nullptr;
TFile *RootHandler::file_ = nullptr; //!< The ROOT file that all the information will be stored in.
set<TTree *> RootHandler::treeList_; //!< The list of trees known to the system
map<unsigned int, TH1 *> RootHandler::histogramList_; //!< The list of 1D histograms known to the system
mutex RootHandler::flushMutex_;

/** Instance is created upon first call */
RootHandler *RootHandler::get() {
    if (!instance_)
        instance_ = new RootHandler("histograms.root");
    return (instance_);
}

RootHandler *RootHandler::get(const std::string &fileName) {
    if (!instance_)
        instance_ = new RootHandler(fileName);
    return (instance_);
}

RootHandler::RootHandler(const std::string &fileName) {
    file_ = new TFile(fileName.c_str(), "recreate");
}

RootHandler::~RootHandler() {
    if(file_) {
        while(!flushMutex_.try_lock())
            usleep(1000000);

        file_->Write(0, TObject::kWriteDelete);
        file_->Close();
        delete file_;
    }

    instance_ = nullptr;
}

void RootHandler::AddBranch(TTree *tree, const std::string &name, const std::string &definition) {
    if (!file_)
        throw invalid_argument("The File wasn't opened!");
}

bool RootHandler::Plot(const unsigned int &id, const double &xval, const double &yval/*=-1*/, const double &zval/*=-1*/) {
    auto histogramPair = histogramList_.find(id);
    if(histogramPair == histogramList_.end())
        return false;
    ///@TODO : We need to enable this again at some point. It was causing unspecified errors since it wasn't caught
    /// properly.
//        throw invalid_argument("RootHandler::Plot - Received a request for histogram ID " + to_string(id)
//                               + ", which is unknown to us. Please check that you have defined this histogram.");
    bool hasYval = yval != -1;
    bool hasZval = zval != -1;

    if(!hasYval && !hasZval)
        histogramPair->second->Fill(xval);
    if(hasYval && !hasZval)
        dynamic_cast<TH2I*>(histogramPair->second)->Fill(xval, yval);
    if(!hasYval && hasZval)
        dynamic_cast<TH2I*>(histogramPair->second)->Fill(xval, zval);
    if(hasYval && hasZval)
        dynamic_cast<TH3I*>(histogramPair->second)->Fill(xval, yval, zval);
    return true;
}

///@TODO Update this so that we're being a little more flexible with our histogramming. At the moment, I'm wanting to
/// mimic the function calls to DAMM as closely as possible. This will reduce the amount of rewrites for now.
TH1 *RootHandler::RegisterHistogram(const unsigned int &id, const std::string &title, const unsigned int &xBins,
                                    const unsigned int &yBins/* = 0*/, const unsigned int &zBins/* = 0*/) {
    auto histogram = histogramList_.find(id);
    if (histogram != histogramList_.end())
        return histogram->second;

    if (!yBins && !zBins)
        return histogramList_.emplace(make_pair(id, new TH1I(("h"+to_string(id)).c_str(), title.c_str(), xBins, 0, xBins))).first->second;
    if(yBins && !zBins)
        return histogramList_.emplace(make_pair(id, new TH2I(("h"+to_string(id)).c_str(), title.c_str(), xBins, 0, xBins, yBins, 0, yBins))).first->second;
    if(!yBins)
        return histogramList_.emplace(make_pair(id, new TH2I(("h"+to_string(id)).c_str(), title.c_str(), xBins, 0, xBins, zBins, 0, zBins))).first->second;
    return histogramList_.emplace(make_pair(id, new TH3I(("h"+to_string(id)).c_str(), title.c_str(), xBins, 0, xBins, yBins, 0, yBins, zBins, 0, zBins))).first->second;
}

void RootHandler::AsyncFlush() {
    for (const auto &tree : treeList_) {
        tree->Fill();
        if (tree->GetEntries() % 10000 == 0)
            tree->AutoSave();
    }

    file_->Write(0, TObject::kWriteDelete);
    flushMutex_.unlock();
}

void RootHandler::Flush() {
    if(flushMutex_.try_lock()) {
        thread worker0(AsyncFlush);
        worker0.detach();
    }
}