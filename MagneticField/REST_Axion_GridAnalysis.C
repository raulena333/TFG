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
//*** Description: This script performs analysis on different magnetic field maps represented as grids with varying 
//*** mesh sizes. It also calculates the transmission probability, error, and runtime for each grid configuration at 
//*** different axion masses 
//*** 
//*** Mesh Map Definitions in mm:
//*** (10, 10, 50), (20, 20, 100), (30, 30, 150), (50, 50, 250), (50, 50, 500)
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 10).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - m1: Axion mass in eV (default: 0.01).
//*** - m2: Axion mass in eV (default: 0.3).
//*** - num_intervals: defaul number of intervals (100).
//*** - qawo_levels: default number of qawo levels (20).
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
    std::vector<std::chrono::milliseconds> timeComputation;
    double meanError;
    double meanProbability;
    double meanTime;
};

constexpr bool kDebug = true;

Int_t REST_Axion_GridAnalysis(Int_t nData = 10, Double_t Ea = 4.2, std::string gasName = "He", Double_t m1 = 0.01, Double_t m2 = 0.1,
                         Int_t num_intervals = 100, Int_t qawo_levels = 20) {
    // Mesh Map Definitions in mm
    std::vector<TVector3> meshSizes = {
        TVector3(10, 10, 50),
        TVector3(20, 20, 100),
        TVector3(30, 30, 150),
        TVector3(50, 50, 250),
        TVector3(50, 50, 500)
    };

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    const TVector3 position(-5, 5, -11000);
    const TVector3 direction = (position - TVector3(5, -5, 11000)).Unit();
    const Double_t gasDensity = 2.9868e-10;

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine masses based on resonance condition
    std::vector<Double_t> masses;
    for (unsigned i = 0; i < 3; ++i) {
        if (i < 2)
            masses.push_back(i == 0 ? m1 : m2);
        else
            masses.push_back(gas != nullptr ? gas->GetPhotonMass(Ea) : 0);
    }

    for(const auto &fieldName : fieldNames) {
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
            if(field.second.mapSize.X() != 10){
                for (size_t n = 0; n < field.second.magneticField->GetNumberOfVolumes(); n++) 
                    field.second.magneticField->ReMap(n, field.second.mapSize);
            }
            field.second.magneticField->SetTrack(position, direction);
            field.second.axionField->AssignMagneticField(field.second.magneticField.get());
        }  

        for(const auto &accuracy : {0.05, 0.1, 0.5}){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Accuracy value: " << accuracy << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            for (const auto &ma : masses) {
                for (auto& field : fields) {
                    field.second.probability.clear();
                    field.second.timeComputation.clear();
                    field.second.error.clear();
                }
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Mass: " << ma << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }

                // Iterate over each grid map mesh
                for(unsigned i = 0; i < nData; i++) {
                    if(kDebug){
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << "Data: " << i << std::endl;
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << std::endl;
                    }
                    for(auto &field : fields) {
                        auto start_time = std::chrono::high_resolution_clock::now();
                        std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, num_intervals, qawo_levels);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                        field.second.probability.push_back(probField.first);
                        field.second.error.push_back(probField.second);
                        field.second.timeComputation.push_back(duration);

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

                // Calculate means
                for (auto &field : fields) {
                    field.second.meanError = std::accumulate(field.second.error.begin(), field.second.error.end(), 0.0) / nData;
                    field.second.meanProbability = std::accumulate(field.second.probability.begin(), field.second.probability.end(), 0.0) / nData;
                    field.second.meanTime = std::accumulate(field.second.timeComputation.begin(), field.second.timeComputation.end(), std::chrono::milliseconds(0)).count() / nData;
                }

                // Open the file for writing
                std::string filename;
                std::string folder = "GridAnalysis/";
                std::ostringstream ossMass, ossAccuracy;
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }
                if (ma == (gas != nullptr ? gas->GetPhotonMass(Ea) : 0)) {
                    ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
                    filename = folder + "REST_AXION_" + fieldName + "_GridAnalysis_Accuracy_" + ossAccuracy.str() + "_OnResonance.txt";
                } else {
                    ossMass << std::fixed << std::setprecision(2) << ma;
                    ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
                    filename = folder + "REST_AXION_" + fieldName + "_GridAnalysis_Accuracy_" + ossAccuracy.str() + "_Mass_" + ossMass.str() + ".txt";
                }

                // Debug message: Opening file
                if (kDebug) {
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Opening file: " << filename << std::endl;
                }

                std::ofstream outputFile(filename);
                if (!outputFile.is_open()) {
                    std::cerr << "Error: Unable to open the file for writing!" << std::endl;
                    return 1;
                }

                outputFile << (ma != (gas != nullptr ? gas->GetPhotonMass(Ea) : 0) ? "Off resonance, ma: " : "On resonance, ma: ") << ma << "Accuracy: " << accuracy << std::endl;
                outputFile << "Grid\tSize\tProbability\tError\tTime(ms)\n";
                for (const auto& field : fields) {
                    outputFile << field.first << "\t (" << field.second.mapSize.X() << "," << field.second.mapSize.Y() << "," << field.second.mapSize.Z()<< ")\t "
                                << field.second.meanProbability << "\t" << field.second.meanError << "\t" << field.second.meanTime << "\n";
                }
                // Debug message: Closing file
                if (kDebug) {
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Closing file: " << filename << std::endl;
                }
                outputFile.close();
            }
        }
    }
    return 0;
}