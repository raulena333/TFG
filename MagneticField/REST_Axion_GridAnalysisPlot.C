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
#include <TVector3.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description: This script performs analysis on different magnetic field maps represented as grids with varying 
//*** mesh sizes. It also calculates the transmission probability and error for each grid configuration at different 
//*** axion masses and energies,then plots it. ALso calculates the runtime for each esh and plots them.
//*** 
//*** Mesh Map Definitions in mm:
//*** (20, 20, 100), (30, 30, 150), (50, 50, 250), (50, 50, 500)
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial axion mass in eV (default: 0.).
//*** - mf: Final axion mass in eV (default: 0.5).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::ReMap'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct FieldTrack {
    std::unique_ptr<TRestAxionMagneticField> magneticField;
    std::unique_ptr<TRestAxionField> axionField; 
    const TVector3 mapSize;

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<double> timeComputation;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_GridAnalysisPlot(Int_t nData = 200, Double_t Ea = 4.2, std::string gasName = "He", 
                                  Double_t mi = 0., Double_t mf = 0.5, bool useLogScale = false) {
    // Mesh Map Definitions in mm
    std::vector<TVector3> meshSizes = {
        TVector3(10,10,50),
        TVector3(20, 20, 100),
        TVector3(30, 30, 150),
        TVector3(50, 50, 250),
        TVector3(100, 100, 500)
    };

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-100, -100 ,-11000);
    TVector3 direction = (position - TVector3(10, -10 , 9000)).Unit();

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine masses 
    std::vector<Double_t> masses;
    for(unsigned j = 0; j< nData; j++)
        masses.push_back(mi + j *(mf - mi)/ nData);
    
    for(const auto& fieldName : fieldNames) {
        // Fill the struct 
        std::map<std::string, FieldTrack> fields;
        for (size_t i = 0; i < meshSizes.size(); ++i) {
            std::string gridName = "Grid" + std::to_string(i + 1);
            fields.emplace(gridName, FieldTrack{
                std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName), 
                std::make_unique<TRestAxionField>(),
                meshSizes[i]
            });
        }

        // Create an instance of TRestAxionField and assign magnetic field and gas (if provided). Remap the grid
        for (auto& field : fields) {
            if (gas != nullptr) 
                field.second.axionField->AssignBufferGas(gas.get());
            for (size_t n = 0; n < field.second.magneticField->GetNumberOfVolumes(); n++) {
                field.second.magneticField->ReMap(n, field.second.mapSize);
            }
            field.second.magneticField->SetTrack(position, direction);
            field.second.axionField->AssignMagneticField(field.second.magneticField.get());
        }  

        for (const auto& ma : masses) {
 
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            for(auto& field : fields) {
                auto start_time = std::chrono::high_resolution_clock::now();
                std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.3, 100, 20);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                field.second.probability.push_back(probField.first);
                field.second.error.push_back(probField.second);
                field.second.timeComputation.push_back(duration.count());

                if (kDebug) {
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << field.first << std::endl;
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
            // Create canvas to plot the probabilities against the mass
            TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbability").c_str(), (fieldName + "_MassProb").c_str(), 850, 673);
            canvasProb->cd();

            Int_t colorIndex = 1;
            TLegend *legendProb = new TLegend(0.1, 0.7, 0.3, 0.9);
            std::vector<TGraphErrors*> graphsProb;

            for (const auto &field : fields) {
                TGraphErrors *graph = new TGraphErrors(masses.size(), masses.data(), field.second.probability.data(), nullptr, field.second.error.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                std::string meshSizeStr = "(" + (field.second.mapSize.X() == static_cast<int>(field.second.mapSize.X()) ? std::to_string(static_cast<int>(field.second.mapSize.X())) : std::to_string(field.second.mapSize.X())) +
                          "," + (field.second.mapSize.Y() == static_cast<int>(field.second.mapSize.Y()) ? std::to_string(static_cast<int>(field.second.mapSize.Y())) : std::to_string(field.second.mapSize.Y())) +
                          "," + (field.second.mapSize.Z() == static_cast<int>(field.second.mapSize.Z()) ? std::to_string(static_cast<int>(field.second.mapSize.Z())) : std::to_string(field.second.mapSize.Z())) + ")";
                legendProb->AddEntry(graph, meshSizeStr.c_str(), "l");
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

            // Create the canvas to plot the runTime of each grid
            TCanvas *canvasRun = new TCanvas((fieldName + "_MassRunTime").c_str(), (fieldName + "_MassRun").c_str(), 850, 673);
            canvasRun->cd();

            colorIndex = 1;
            TLegend *legendRun = new TLegend(0.1, 0.7, 0.3, 0.9);
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
                std::string meshSizeStr = "(" + (field.second.mapSize.X() == static_cast<int>(field.second.mapSize.X()) ? std::to_string(static_cast<int>(field.second.mapSize.X())) : std::to_string(field.second.mapSize.X())) +
                          "," + (field.second.mapSize.Y() == static_cast<int>(field.second.mapSize.Y()) ? std::to_string(static_cast<int>(field.second.mapSize.Y())) : std::to_string(field.second.mapSize.Y())) +
                          "," + (field.second.mapSize.Z() == static_cast<int>(field.second.mapSize.Z()) ? std::to_string(static_cast<int>(field.second.mapSize.Z())) : std::to_string(field.second.mapSize.Z())) + ")";
                legendRun->AddEntry(graph, meshSizeStr.c_str(), "l");
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
                std::string folder = "GridAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameProb = fieldName + "_ProbabilityGrid.png";
                std::string fileNameRun = fieldName + "_RunTimeGrid.png";
                canvasProb->SaveAs((folder + fileNameProb).c_str());
                canvasRun->SaveAs((folder + fileNameRun).c_str());
            }
        }
    }
    return 0;
}