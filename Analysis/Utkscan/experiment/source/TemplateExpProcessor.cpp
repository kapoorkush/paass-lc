///@file TemplateExpProcessor.cpp
///@brief Example class for experiment specific setups
///@author S. V. Paulauskas
///@date May 20, 2016
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "CloverProcessor.hpp"
#include "RawEvent.hpp"
#include "TemplateProcessor.hpp"
#include "TemplateExpProcessor.hpp"
#include "TimingMapBuilder.hpp"

static double tof_;
static double tEnergy;

namespace dammIds {
    namespace experiment {
        const int D_TSIZE = 0; //!< Size of the event
        const int D_GEENERGY = 1; //!< Gamma energy
        const int DD_TENVSGEN = 2; //!< Energy vs Gamma Energy
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void TemplateExpProcessor::DeclarePlots(void) {
    histo.DeclareHistogram1D(D_TSIZE, S3, "Num Template Evts");
    histo.DeclareHistogram1D(D_GEENERGY, SA, "Gamma Energy with Cut");
    histo.DeclareHistogram2D(DD_TENVSGEN, SA, SA, "Template En vs. Ge En");
}

TemplateExpProcessor::TemplateExpProcessor() : EventProcessor(OFFSET, RANGE, "TemplateExpProcessor") {
    gCutoff_ = 0.; ///Set the gamma cutoff energy to a default of 0.
    SetAssociatedTypes();
    SetupAsciiOutput();
    SetupRootOutput();
}

TemplateExpProcessor::TemplateExpProcessor(const double &gcut) : EventProcessor(OFFSET, RANGE, "TemplateExpProcessor") {
    gCutoff_ = gcut;
    SetAssociatedTypes();
    SetupAsciiOutput();
    SetupRootOutput();
}

///Destructor to close output files and clean up pointers
TemplateExpProcessor::~TemplateExpProcessor() {
    poutstream_->close();
    delete (poutstream_);
    prootfile_->Write();
    prootfile_->Close();
    delete (prootfile_);
}

///Associates this Experiment Processor with template and ge detector types
void TemplateExpProcessor::SetAssociatedTypes(void) {
    associatedTypes.insert("template");
    associatedTypes.insert("clover");
}

///Sets up the name of the output ascii data file
void TemplateExpProcessor::SetupAsciiOutput(void) {
    stringstream name;
    name << Globals::get()->GetOutputPath() << Globals::get()->GetOutputFileName() << ".dat";
    poutstream_ = new ofstream(name.str().c_str());
}

///Sets up ROOT output file, tree, branches, histograms.
void TemplateExpProcessor::SetupRootOutput(void) {
    stringstream rootname;
    rootname << Globals::get()->GetOutputPath() << Globals::get()->GetOutputFileName() << "-Template.root";
    prootfile_ = new TFile(rootname.str().c_str(), "RECREATE");
    proottree_ = new TTree("data", "");
    proottree_->Branch("tof", &tof_, "tof/D");
    proottree_->Branch("ten", &tEnergy, "ten/D");
    ptvsge_ = new TH2D("tvsge", "", 1000, -100, 900, 16000, 0, 16000);
    ptsize_ = new TH1D("tsize", "", 40, 0, 40);
}

///We do nothing here since we're completely dependent on the results of others
bool TemplateExpProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);
    return (true);
}

///Main processing of data of interest
bool TemplateExpProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    ///Vectors to hold the information we will get from the various processors
    vector < ChanEvent * > geEvts, tEvts;
    vector <vector<AddBackEvent>> geAddback;

    ///Obtain the list of template events that were created
    ///in TemplateProcessor::PreProecess
    if (event.GetSummary("template")->GetList().size() != 0)
        tEvts = ((TemplateProcessor *) DetectorDriver::get()->GetProcessor("TemplateProcessor"))->GetTemplateEvents();

    ///Obtain the list of Ge events and addback events that were created
    ///in CloverProcessor::PreProcess
    if (event.GetSummary("clover")->GetList().size() != 0) {
        geEvts = ((CloverProcessor *) DetectorDriver::get()->GetProcessor("CloverProcessor"))->GetGeEvents();
        geAddback = ((CloverProcessor *) DetectorDriver::get()->GetProcessor("CloverProcessor"))->GetAddbackEvents();
    }

    ///Plot the size of the template events vector in two ways
    histo.Plot(D_TSIZE, tEvts.size());
    ptsize_->Fill(tEvts.size());

    ///Obtain some useful logic statuses
    bool isTapeMoving = TreeCorrelator::get()->place("TapeMove")->status();
    bool hasBeta = TreeCorrelator::get()->place("Beta")->status();
    double clockInSeconds = Globals::get()->GetClockInSeconds();

    ///Begin loop over template events
    for (vector<ChanEvent *>::iterator tit = tEvts.begin(); tit != tEvts.end(); ++tit) {
        ///Begin loop over ge events
        for (vector<ChanEvent *>::iterator it1 = geEvts.begin(); it1 != geEvts.end(); ++it1) {
            ChanEvent *chan = *it1;

            double gEnergy = chan->GetCalibratedEnergy();

            ///Plot the Template Energy vs. Ge Energy if tape isn't moving
            if (!isTapeMoving)
                histo.Plot(DD_TENVSGEN, gEnergy, (*tit)->GetEnergy());

            ///Output template and ge energy to text file if we had a beta along with the runtime in seconds.
            if (hasBeta)
                *poutstream_ << (*tit)->GetEnergy() << " " << gEnergy << " " << clockInSeconds << endl;
            ///Fill ROOT histograms and tree with the information

            ptvsge_->Fill((*tit)->GetEnergy(), gEnergy);
            tEnergy = (*tit)->GetEnergy();
            tof_ = (*tit)->GetTime() - chan->GetWalkCorrectedTime();
            proottree_->Fill();
            tEnergy = tof_ = -9999;

            ///Plot the Ge energy with a cut
            if (gEnergy > gCutoff_)
                histo.Plot(D_GEENERGY, gEnergy);
        }
    }
    EndProcess();
    return (true);
}
