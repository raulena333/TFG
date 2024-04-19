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
//*** (20, 20, 100), (30, 30, 150), (50, 50, 250), (100, 100, 500)
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 125).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial axion mass in eV (default: 0.2).
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

Int_t REST_Axion_GridAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", 
                                  Double_t mi = 0.2, Double_t mf = 0.5, Bool_t useLogScale = true) {
    // Mesh Map Definitions in mm
    std::vector<TVector3> meshSizes = {
        TVector3(10,10,50),
        TVector3(20, 20, 100),
        TVector3(30, 30, 150),
        TVector3(50, 50, 250),
        TVector3(100, 100, 500)
    };

    // Create Variables
    // std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff"};
    const TVector3 position(-5, 5, -11000);
    const TVector3 direction = (position - TVector3(5, -5, 11000)).Unit();
    const Double_t gasDensity = 2.9868e-10;

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

        // Iterate for saome accuracys to test the runTime
        // std::vector<Double_t> accuracyValues = {0.5, 1.};
        std::vector<Double_t> accuracyValues = {0.25};
        for(const auto &accuracy : accuracyValues){
            for (auto& field : fields) {
                field.second.probability.clear();
                field.second.timeComputation.clear();
                field.second.error.clear();
            }
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Accuracy value: " << accuracy << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            // Iterate for all masses between mi and mf
            for (const auto& ma : masses) {
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Mass: " << ma << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }
                for(auto& field : fields) {
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

                    field.second.probability.push_back(probField.first);
                    field.second.error.push_back(probField.second);
                    field.second.timeComputation.push_back(duration.count());

                    if (kDebug) {
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << field.first << std::endl;
                        std::cout << "Probability: " << probField.first << std::endl;
                        std::cout << "Error: " << probField.second << std::endl;
                        std::cout << "Runtime (s): " << duration.count() << std::endl;
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << std::endl;
                    }
                }
            }
            /// PLOT ///
            if (kPlot) {
                // Create an array of 5 distinct colors excluding yellow
                std::vector<Color_t> colors = {kMagenta+2, kCyan-3, kGreen+2, kYellow-2, kRed-3};

                // Create canvas to plot the probabilities against the mass
                TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbability" + std::to_string(accuracy)).c_str(), (fieldName + "_MassProb").c_str(), 800, 600);
                canvasProb->cd();

                Int_t colorIndex = 0;
                TLegend *legendProb = new TLegend(0.7, 0.7, 0.9, 0.9);
                std::vector<TGraphErrors*> graphsProb;

                for (const auto &field : fields) {
                    TGraphErrors *graph = new TGraphErrors(masses.size(), masses.data(), field.second.probability.data(), nullptr, field.second.error.data());
                    graph->SetLineColor(colors[colorIndex]);
                    graph->SetLineWidth(1);
                    graph->GetYaxis()->SetRangeUser(1e-32, 1e-18);
                    graph->SetTitle("");
                    if (colorIndex == 0) {
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

                graphsProb[0]->SetTitle("");
                graphsProb[0]->GetYaxis()->SetTitle("Probabilidad");
                graphsProb[0]->GetXaxis()->SetTitle("Masa Axion (eV)");
                graphsProb[0]->GetXaxis()->SetRange(mi, mf);
                graphsProb[0]->GetYaxis()->SetRangeUser(1e-32, 1e-19);
                graphsProb[0]->GetXaxis()->SetTitleSize(0.03); 
                graphsProb[0]->GetXaxis()->SetTitleFont(40);  
                graphsProb[0]->GetXaxis()->SetLabelSize(0.025); 
                graphsProb[0]->GetXaxis()->SetLabelFont(40);  
                graphsProb[0]->GetYaxis()->SetTitleSize(0.03); 
                graphsProb[0]->GetYaxis()->SetTitleFont(40);  
                graphsProb[0]->GetYaxis()->SetLabelSize(0.025); 
                graphsProb[0]->GetYaxis()->SetLabelFont(40); 
                legendProb->Draw();

                // Set logarithmic scale if required
                if (useLogScale)
                    canvasProb->SetLogy();

                // Create the canvas to plot the runTime of each grid
                TCanvas *canvasRun = new TCanvas((fieldName + "_MassRunTime" + std::to_string(accuracy)).c_str(), (fieldName + "_MassRun").c_str(), 800, 600);
                canvasRun->cd();

                colorIndex = 0;
                TLegend *legendRun = new TLegend(0.7, 0.7, 0.9, 0.9);
                std::vector<TGraph*> graphsRun;

                for (const auto &field : fields) {
                    TGraph *graph = new TGraph(masses.size(), masses.data(), field.second.timeComputation.data());
                    graph->SetLineColor(colors[colorIndex]);
                    graph->SetLineWidth(1);
                    graph->SetTitle("");
                    if (colorIndex == 0) {
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

                graphsRun[0]->SetTitle("");
                graphsRun[0]->GetYaxis()->SetTitle("Tiempo computacional (s)");
                graphsRun[0]->GetXaxis()->SetTitle("Masa Axion (eV)");
                graphsRun[0]->GetXaxis()->SetRange(mi, mf);
                graphsRun[0]->GetXaxis()->SetTitleSize(0.03); 
                graphsRun[0]->GetXaxis()->SetTitleFont(40);  
                graphsRun[0]->GetXaxis()->SetLabelSize(0.025); 
                graphsRun[0]->GetXaxis()->SetLabelFont(40);  
                graphsRun[0]->GetYaxis()->SetTitleSize(0.03); 
                graphsRun[0]->GetYaxis()->SetTitleFont(40);  
                graphsRun[0]->GetYaxis()->SetLabelSize(0.025); 
                graphsRun[0]->GetYaxis()->SetLabelFont(40); 
                legendRun->Draw();

                if (useLogScale)
                    canvasRun->SetLogy();

                if (kSave) {
                    std::string folder = "GridAnalysis/";
                    if (!std::filesystem::exists(folder)) {
                        std::filesystem::create_directory(folder);
                    }
                    std::ostringstream ossAccuracy;
                    ossAccuracy << std::fixed << std::setprecision(2) << accuracy;

                    std::string fileNameProb = fieldName + "_GridAnalysis_Probability_Acc" + ossAccuracy.str();
                    std::string fileNameRun = fieldName + "_GridAnalysis_RunTime_Acc" + ossAccuracy.str();

                    if (useLogScale) {
                        fileNameProb += "_log.pdf";
                        fileNameRun += "_log.pdf";
                    } else {
                        fileNameProb += ".pdf";
                        fileNameRun += ".pdf";
                    }

                    canvasProb->SaveAs((folder + fileNameProb).c_str());
                    canvasRun->SaveAs((folder + fileNameRun).c_str());
                }

                // Calculate residuals for grids 2, 3, and 5
                std::map<std::string, std::vector<Double_t>> residuals;
                for (const auto &field : fields) {
                    if (field.first != "Grid1") {
                        std::vector<Double_t> gridResiduals;
                        for (size_t i = 0; i < nData; ++i) {
                            Double_t residual = std::abs(fields["Grid1"].probability[i] - field.second.probability[i]) / fields["Grid1"].probability[i] * 100.0 ;
                            gridResiduals.push_back(residual);
                        }
                        residuals[field.first] = gridResiduals;
                    }
                }

                // Create canvas to plot residuals for Grid2 and Grid5
                TCanvas *canvasResiduals = new TCanvas((fieldName + "_Residuals_" + std::to_string(accuracy)).c_str(), "Residuals", 1000, 300);
                canvasResiduals->Divide(2, 1);

                // Plot residuals for Grid2
                canvasResiduals->cd(1);
                TGraph *graphGrid2 = nullptr;
                for (const auto &residual : residuals) {
                    if (residual.first == "Grid2") {
                        graphGrid2 = new TGraph(nData, masses.data(), residual.second.data());
                        graphGrid2->SetMarkerStyle(8);
                        graphGrid2->SetMarkerSize(0.4);
                        graphGrid2->SetMarkerColor(kCyan-3);
                        graphGrid2->SetTitle("");
                        graphGrid2->GetXaxis()->SetTitle("Masa Axion (eV)");
                        graphGrid2->GetYaxis()->SetTitle("Residuos (%)");
                        graphGrid2->GetXaxis()->SetTitleSize(0.04);
                        graphGrid2->GetXaxis()->SetLabelSize(0.03);
                        graphGrid2->GetYaxis()->SetTitleSize(0.04);
                        graphGrid2->GetYaxis()->SetLabelSize(0.03);
                        graphGrid2->GetYaxis()->SetTitleFont(62);
                        graphGrid2->GetYaxis()->SetTitleOffset(1.0);
                        graphGrid2->GetXaxis()->SetTitleFont(62);
                        graphGrid2->GetYaxis()->SetLabelFont(62);
                        graphGrid2->GetXaxis()->SetLabelFont(62);
                        graphGrid2->Draw("AP");
                        break;
                    }
                }
                gPad->SetLogy();

                // Plot residuals for Grid5
                canvasResiduals->cd(2);
                TGraph *graphGrid5 = nullptr; 
                for (const auto &residual : residuals) {
                    if (residual.first == "Grid5") {
                        graphGrid5 = new TGraph(nData, masses.data(), residual.second.data());
                        graphGrid5->SetMarkerStyle(8);
                        graphGrid5->SetMarkerSize(0.4);
                        graphGrid5->SetMarkerColor(kRed-3);
                        graphGrid5->SetTitle("");
                        graphGrid5->GetXaxis()->SetTitle("Masa Axion (eV)");
                        graphGrid5->GetYaxis()->SetTitle("Residuos (%)");
                        graphGrid5->GetXaxis()->SetTitleSize(0.04);
                        graphGrid5->GetXaxis()->SetLabelSize(0.03);
                        graphGrid5->GetYaxis()->SetTitleSize(0.04);
                        graphGrid5->GetYaxis()->SetLabelSize(0.03);
                        graphGrid5->GetYaxis()->SetTitleFont(62);
                        graphGrid5->GetYaxis()->SetTitleOffset(1.0);
                        graphGrid5->GetXaxis()->SetTitleFont(62);
                        graphGrid5->GetYaxis()->SetLabelFont(62);
                        graphGrid5->GetXaxis()->SetLabelFont(62);
                        graphGrid5->Draw("AP"); 
                        break; 
                    }
                }
                gPad->SetLogy();

                // Save canvas if required
                if (kSave) {
                    std::string folder = "GridAnalysis/";
                    if (!std::filesystem::exists(folder)) {
                        std::filesystem::create_directory(folder);
                    }
                    std::ostringstream ossAccuracy;
                    ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
                    std::string fileNameResiduals = folder + "Residuals_" + fieldName + "_Accuracy_" + ossAccuracy.str() + ".pdf";
                    canvasResiduals->SaveAs(fileNameResiduals.c_str());
                }

                // Clean up
                delete legendProb;
                delete legendRun;
                for (auto &graph : graphsProb) {
                    delete graph;
                }
                for (auto &graph : graphsRun) {
                    delete graph;
                }
                delete canvasProb;
                delete canvasRun;
                delete canvasResiduals;
            }
        }
    }
    return 0;
}