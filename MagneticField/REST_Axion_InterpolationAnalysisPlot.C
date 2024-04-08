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
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"
#include <TLatex.h>

//*******************************************************************************************************
//*** Description: This script analyzes the MagneticField class to determine whether it is beneficial to enable or 
//*** disable trilinear interpolation when calculating the magnetic field at an arbitrary point. It plots the 
//*** probability transmission for each mass at a specified accuracy and also plots the runtime for each mass.
//*** 
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial axion mass in eV (default: 0.2).
//*** - mf: Final axion mass in eV (default: 0.5).
//*** - Accuracy: Accuracy value for intrgration in GSL, depends on the axion mass (default 1.).
//*** - useLogSCale: set log scale in y-axis for plots (default: false)
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetInterpolation'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************


struct FieldTrack {
    bool interpolation;

    std::vector<double> probability;
    //std::vector<double> error;
    std::vector<double> timeComputation;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_InterpolationAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", 
                    Double_t mi = 0.2, Double_t mf = 0.5, Double_t accuracy = 1., Bool_t useLogScale =  true){
    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-10, 10, -11000);
    TVector3 fposition(10, -10 , 11000);
    TVector3 direction = (position - fposition).Unit();
    std::vector<Double_t> masses;

    std::map<std::string, FieldTrack> fields = {
        {"Interpolation", {true}},
        {"No-Interpolation", {false}}
    };

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine mass values
    for(unsigned j = 0; j< nData; j++){
        masses.push_back(mi + j *(mf - mi)/ nData);
    }

    for(const auto &fieldName : fieldNames){
        // Create an instance of TRestAxionField and assign magnetic field and gas (if provided).
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        if (gas != nullptr) 
            axionField->AssignBufferGas(gas.get());

        //magneticField->SetTrack(position, direction);
        std::vector<Double_t> magneticValues =  magneticField->GetTransversalComponentAlongPath(position, fposition, 10); 
        axionField->AssignMagneticField(magneticField.get()); 

        for(const auto &ma : masses){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            for(auto &field : fields){
                magneticField->SetInterpolation(field.second.interpolation);
                auto start_time = std::chrono::high_resolution_clock::now();
                // GSL
                //std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
                // Standard
                Double_t probField = axionField->GammaTransmissionProbability(magneticValues, 10, Ea, ma);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

                field.second.probability.push_back(probField);
                //field.second.probability.push_back(probField.first);
                //field.second.error.push_back(probField.second);
                field.second.timeComputation.push_back(duration.count());

                if(kDebug){
                    std::cout << "Mass: " << ma << std::endl;
                    std::cout << field.first << std::endl;
                    std::cout << "Probability: " << probField << std::endl;
                    //std::cout << "Probability: " << probField.first << std::endl;
                    //std::cout << "Error: " << probField.second << std::endl;
                    std::cout << "Runtime: " << duration.count() << std::endl;
                    std::cout << std::endl;
                }

            }
        }

        /// PLOT ///
        if(kPlot){
            // Create canvas to plot the probabilities against the mass
            TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbability").c_str(), (fieldName + "_MassProb").c_str(), 850, 673);
            canvasProb->cd();

            Int_t colorIndex = 1;
            TLegend *legendProb = new TLegend(0.1, 0.8, 0.3, 0.9);
            //std::vector<TGraphErrors*> graphsProb;
            std::vector<TGraph*> graphsProb;

            for (const auto &field : fields) {
                // TGraphErrors *graph = new TGraphErrors(masses.size(), masses.data(), field.second.probability.data(), nullptr, field.second.error.data());
                TGraph *graph = new TGraph(masses.size(), masses.data(), field.second.probability.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendProb->AddEntry(graph, field.first.c_str(), "l");
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

            // Create the canvas to plot the runTime of each 
            TCanvas *canvasRun = new TCanvas((fieldName + "_MassRunTime").c_str(), (fieldName + "_MassRun").c_str(), 850, 673);
            canvasRun->cd();

            colorIndex = 1;
            TLegend *legendRun = new TLegend(0.1, 0.8, 0.3, 0.9);
            std::vector<TGraph*> graphsRun;

            for (const auto &field : fields) {
                TGraph *graph = new TGraph(masses.size(), masses.data(), field.second.timeComputation.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendRun->AddEntry(graph, field.first.c_str(), "l");
                colorIndex++;

                graphsRun.push_back(graph);
            }

            graphsRun[0]->SetTitle("Axion Mass vs RunTime");
            //graphsRun[0]->GetYaxis()->SetTitle("RunTime (ms)");
            graphsRun[0]->GetYaxis()->SetTitle("RunTime ( #mu s)");
            graphsRun[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphsRun[0]->GetXaxis()->SetTitleSize(0.03);
            graphsRun[0]->GetYaxis()->SetTitleSize(0.03);
            graphsRun[0]->GetXaxis()->SetLabelSize(0.03);
            graphsRun[0]->GetYaxis()->SetLabelSize(0.03);
            legendRun->Draw();

            if constexpr (kSave) {
                std::string folder = "InterpolationAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameProb = fieldName + "_ProbabilityInterpolation.png";
                std::string fileNameRun = fieldName + "_RunTimeInterpolation.png";
                canvasProb->SaveAs((folder + fileNameProb).c_str());
                canvasRun->SaveAs((folder + fileNameRun).c_str());
            }
        }
    }
    return 0;
}
