#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>
#include <numeric>
#include <iomanip>
#include <sstream>
#include <memory>
#include <filesystem>

#include <TCanvas.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** This script performs an analysis of axion detection probability and runtime in a magnetic field 
//*** environment, comparing the effects of using different gases or a vacuum. It calculates the 
//*** probability of axion transmission through the field at various axion masses and measures the 
//*** computation time required for each scenario.
//*** 
//*** Gas Definitions:
//*** - 'He': Helium gas defined by its density.
//*** - '': Vacuum.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - mi: Initial axion mass in eV (default: 0.).
//*** - mf: Final axion mass in eV (default: 0.2).
//*** - useLogScale. Bool to set the y-axis to log scale for plotting (default: false).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability` and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct GasTrack {
    std::unique_ptr<TRestAxionField> axionField;
    std::unique_ptr<TRestAxionBufferGas> gas;
    std::string gasName;

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<double> timeComputation;
};

    constexpr bool kDebug = true;
    constexpr bool kPlot = true;
    constexpr bool kSave = true;

Int_t REST_Axion_GasAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, Double_t mi = 0., Double_t mf = 0.2, Bool_t useLogScale = false) {

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    Double_t gasDensity = 2.6e-11;
    TVector3 position(-5, 5, -9000);
    TVector3 direction = (position - TVector3(5, -5 , 9000));
    std::vector<Double_t> mass;

    for(Int_t j = 0; j < nData; j++) {
        mass.push_back(mi + j * (mf - mi) / nData);
    }

    // Loop for both magnetic field maps
    for (const auto& fieldName : fieldNames) {
        // Create magnetic field
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);

        // Create gas tracks
        std::map<std::string, GasTrack> gasTracks;
        gasTracks["He-Gas"] = {std::make_unique<TRestAxionField>(), std::make_unique<TRestAxionBufferGas>(), "He"};
        gasTracks["Vacuum"] = {std::make_unique<TRestAxionField>(), nullptr, ""};

        // Assign gas and magnetic field to axion field to each gas track
        for (auto &track : gasTracks) {
            if (track.second.gas != nullptr) {
                track.second.gas->SetGasDensity(track.second.gasName, gasDensity);
                track.second.axionField->AssignBufferGas(track.second.gas.get());
            }
            track.second.axionField->AssignMagneticField(magneticField.get());
            magneticField->SetTrack(position, direction);
        }

        // Simulation loop
        for (const auto& ma : mass) {
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            for (auto &track : gasTracks){
                auto start_time = std::chrono::high_resolution_clock::now();
                std::pair<Double_t, Double_t> probField = track.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 100, 20);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                track.second.probability.push_back(probField.first);
                track.second.error.push_back(probField.second);
                track.second.timeComputation.push_back(duration.count());

                if (kDebug) {
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << track.first << std::endl;
                    std::cout << "Probability: " << probField.first << std::endl;
                    std::cout << "Error: " << probField.second << std::endl;
                    std::cout << "Runtime (ms): " << duration.count() << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }
            }
        }

        /// PLOT ///
        if (kPlot) {
            // Create TCanvas for plotting probability vs. mass
            TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbability").c_str(), (fieldName + "_MassProb").c_str(), 850, 673);
            canvasProb->cd();

            Int_t colorIndex = 1;
            TLegend *legendProb = new TLegend(0.1, 0.7, 0.3, 0.9);
            std::vector<TGraphErrors*> graphsProb;
            for (const auto &track : gasTracks) {
                TGraphErrors *graph = new TGraphErrors(mass.size(), mass.data(), track.second.probability.data(), nullptr, track.second.error.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendProb->AddEntry(graph, track.first.c_str(), "l");
                colorIndex++;

                graphsProb.push_back(graph);
            }

            graphsProb[0]->SetTitle("Axion Mass vs Probability");
            graphsProb[0]->GetYaxis()->SetTitle("Probability");
            graphsProb[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphsProb[0]->GetXaxis()->SetTitleSize(0.03);
            graphsProb[0]->GetYaxis()->SetTitleSize(0.03);
            graphsProb[0]->GetXaxis()->SetLabelSize(0.03);
            graphsProb[0]->GetYaxis()->SetLabelSize(0.03);
            legendProb->Draw();

            // Set logarithmic scale if required
            if (useLogScale)
                canvasProb->SetLogy();


            // Create the canvas to plot the runTime of each gas track
            TCanvas *canvasRun = new TCanvas((fieldName + "_MassRunTime").c_str(), (fieldName + "_MassRun").c_str(), 850, 673);
            canvasRun->cd();

            colorIndex = 1;
            TLegend *legendRun = new TLegend(0.1, 0.7, 0.3, 0.9);
            std::vector<TGraph*> graphsRun;

            for (const auto &track : gasTracks) {
                TGraph *graph = new TGraph(mass.size(), mass.data(), track.second.timeComputation.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendRun->AddEntry(graph, track.first.c_str(), "l");
                colorIndex++;

                graphsRun.push_back(graph);
            }

            graphsRun[0]->SetTitle("Axion Mass vs RunTime");
            graphsRun[0]->GetYaxis()->SetTitle("RunTime (ms)");
            graphsRun[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphsRun[0]->GetXaxis()->SetTitleSize(0.03);
            graphsRun[0]->GetYaxis()->SetTitleSize(0.03);
            graphsRun[0]->GetXaxis()->SetLabelSize(0.03);
            graphsRun[0]->GetYaxis()->SetLabelSize(0.03);
            legendRun->Draw();

            if (kSave) {
                std::string folder = "GasAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameProb = fieldName + "_ProbabilityGas.png";
                std::string fileNameRun = fieldName + "_RunTimeGas.png";
                canvasProb->SaveAs((folder + fileNameProb).c_str());
                canvasRun->SaveAs((folder + fileNameRun).c_str());
            }
        }
    }

    return 0;
}
