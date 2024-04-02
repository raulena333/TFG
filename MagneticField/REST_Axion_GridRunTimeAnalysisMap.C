#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <memory>

#include <TCanvas.h>
#include <TH2D.h>
#include <TVector3.h>

#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description: 
//*** This code performs an analysis of runTime and error across different magnetic field grid configurations and 
//*** parameter values.
//*** 
//*** Mesh Map Definitions in mm:
//***  (30, 30, 150)
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 20).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial axion mass in eV (default: 0.).
//*** - mf: Final axion mass in eV (default: 0.5).
//*** - initialAccuracy: Initial accuracy for calculation transmissionProbability (default: 0.3).
//*** - finalAccuracy: Final accuracy for calculation transmissionProbability (default: 0.9).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::ReMap'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************

constexpr int kNumBins = 100;

struct FieldTrack {
    std::unique_ptr<TRestAxionMagneticField> magneticField;
    std::unique_ptr<TRestAxionField> axionField; 
    const TVector3 mapSize;

    std::unique_ptr<TCanvas> canvasHeatMapRun;
    std::unique_ptr<TH2D> heatmapRunTime;

    std::unique_ptr<TCanvas> canvasHeatMapError;
    std::unique_ptr<TH2D> heatmapError;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_GridRunTimeAnalysisMap(Int_t nData = 2, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0., Double_t mf = 0.5,
                                            Double_t initialAccuracy = 0.3, Double_t finalAccuracy = 0.9) {
    // Mesh Map Definitions in mm
    std::vector<TVector3> meshSizes = {
        TVector3(10,10,50),
        TVector3(30, 30, 150)
    };

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-100, -100, -11000);
    TVector3 direction = (position - TVector3(10, -10 , 9000)).Unit();

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine mass and accuracy values
    std::vector<Double_t> masses;
    std::vector<Double_t> accuracyValues;
    for(unsigned j = 0; j< nData; j++){
        masses.push_back(mi + j *(mf - mi)/ nData);
        accuracyValues.push_back(initialAccuracy + j * (finalAccuracy - initialAccuracy)/ nData);
    }
    
    for(const auto& fieldName : fieldNames) {
        // Fill the struct 
        std::map<std::string, FieldTrack> fields;
        for (size_t i = 0; i < meshSizes.size(); ++i) {
            std::string gridName = "Grid" + std::to_string(i + 1);
            fields.emplace(gridName, FieldTrack{
                std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName), 
                std::make_unique<TRestAxionField>(),
                meshSizes[i],
                std::make_unique<TCanvas>((fieldName + "_" + gridName + "_HeatmapRun").c_str(), (fieldName + " " + gridName + " Heatmap RunTime").c_str(), 900, 700),
                std::make_unique<TH2D>((fieldName + "_" + gridName + "_RunTime_Heatmap").c_str(), (fieldName + " " + gridName + " Heatmap Accuracy RunTime").c_str(), kNumBins, mi, mf, kNumBins, initialAccuracy, finalAccuracy),
                std::make_unique<TCanvas>((fieldName + "_" + gridName + "_HeatmapError").c_str(), (fieldName + " " + gridName + " Heatmap Error").c_str(), 900, 700),
                std::make_unique<TH2D>((fieldName + "_" + gridName + "_Error_Heatmap").c_str(), (fieldName + " " + gridName + " Heatmap Error").c_str(), kNumBins, mi, mf, kNumBins, initialAccuracy, finalAccuracy)
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

        for(const auto& accuracy : accuracyValues){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Accuracy: " << accuracy << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
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
                    std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                    field.second.heatmapRunTime->Fill(ma, accuracy, duration.count());
                    field.second.heatmapError->Fill(ma, accuracy, probField.second);

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
        }

        if (kPlot) {
            for(auto& field : fields) {
                auto& heatmapRunTime = field.second.heatmapRunTime;
                heatmapRunTime->SetStats(0);
                heatmapRunTime->GetXaxis()->SetLabelSize(0.03);
                heatmapRunTime->GetXaxis()->SetLabelFont(22); 
                heatmapRunTime->GetXaxis()->SetTitleSize(0.03); 
                heatmapRunTime->GetXaxis()->SetTitleFont(22);  
                heatmapRunTime->GetYaxis()->SetLabelSize(0.03);
                heatmapRunTime->GetYaxis()->SetLabelFont(22);
                heatmapRunTime->GetYaxis()->SetTitleSize(0.03);
                heatmapRunTime->GetYaxis()->SetTitleFont(22); 
                heatmapRunTime->GetZaxis()->SetLabelSize(0.03);

                field.second.canvasHeatMapRun->cd();
                heatmapRunTime->Draw("COLZ");


                auto& heatmapError = field.second.heatmapError;
                heatmapError->SetStats(0);
                heatmapError->GetXaxis()->SetLabelSize(0.03);
                heatmapError->GetXaxis()->SetLabelFont(22); 
                heatmapError->GetXaxis()->SetTitleSize(0.03); 
                heatmapError->GetXaxis()->SetTitleFont(22);  
                heatmapError->GetYaxis()->SetLabelSize(0.03);
                heatmapError->GetYaxis()->SetLabelFont(22);
                heatmapError->GetYaxis()->SetTitleSize(0.03);
                heatmapError->GetYaxis()->SetTitleFont(22); 
                heatmapError->GetZaxis()->SetLabelSize(0.03);

                field.second.canvasHeatMapError->cd();
                heatmapError->Draw("COLZ");
                if (kSave) {
                    std::string folderName = "HeatMapGrid/";
                    if (field.second.canvasHeatMapRun) {
                        std::string fileNameRunTime = folderName + fieldName + "_" + field.first + "_RunTime_Heatmap.png";
                        field.second.canvasHeatMapRun->SaveAs(fileNameRunTime.c_str());
                    }
                    if (field.second.canvasHeatMapError) {
                        std::string fileNameError = folderName + fieldName + "_" + field.first + "_Error_Heatmap.png";
                        field.second.canvasHeatMapError->SaveAs(fileNameError.c_str());
                    }
                }
            }
        }
    }
    return 0;
}