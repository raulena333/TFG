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

    std::unique_ptr<TCanvas> canvasHeatMap;
    std::unique_ptr<TH2D> heatmapRunTime;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_GridAnalysisPlot(Int_t nData = 2, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0., Double_t mf = 0.5,
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
                std::make_unique<TCanvas>((fieldName + "_" + gridName + "_Heatmap").c_str(), (fieldName + " " + gridName + " Heatmap").c_str(), 900, 700),
                std::make_unique<TH2D>((fieldName + "_" + gridName + "_RunTime_Heatmap").c_str(), (fieldName + " " + gridName + " Heatmap Accuracy RunTime").c_str(), kNumBins, mi, mf, kNumBins, initialAccuracy, finalAccuracy)
            });
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

        // Plot heatmaps
        if (kPlot) {
            for (auto& field : fields) {
                field.second.canvasHeatMap->cd();
                field.second.heatmapRunTime->Draw("COLZ");

                if (kSave) {
                    std::string folder = "GridAnalysis/";
                    std::string fileNameHeatmap = fieldName + "_" + field.first + "_RunTime_Heatmap.png";
                    field.second.canvasHeatMap->SaveAs((folder + fileNameHeatmap).c_str());
                }
            }
        }
    }
    return 0;
}

