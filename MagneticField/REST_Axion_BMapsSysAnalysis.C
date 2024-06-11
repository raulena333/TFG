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
#include <TH2D.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description: This script analyzes and calculates the probability and error for each magnetic field map definition in the 
//*** specified configuration file.
//***
//*** Field Map Definitions:
//*** - MentiskCut: Magnetic field map with 1cm precision in X and Y axes and 5cm in Z axis for the babyIAXO magnet. Cut-off in Z-range (-6,6)m.
//*** - Mentisk: Magnetic field map with 1cm precision in X and Y axes and 5cm in Z axis for the babyIAXO magnet. Full length.
//*** - Bykovskiy2019: Magnetic field map with 5cm precision in X, Y, and Z axes for the babyIAXO magnet.
//*** - Bykovskiy2020: Magnetic field map with 1cm precision in XY axes and 5cm in Z axis for the babyIAXO magnet.
//***
//*** Default Arguments:
//*** - nData: Number of data points to generate (default: 10).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - m1: First axion mass in eV (default: 0.1).
//***  -m2: Second axion mass in eV (default: 0.01).
//*** - gasName: Gas name (default: "He").
//*** - num_intervals: Number of intervals for integration (default: 100).
//*** - qawo_levels: Number of levels for QAWO integration (default: 20).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability` and `TRestAxionBufferGas::GetPhotonMass`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct FieldInfo {
    std::unique_ptr<TRestAxionMagneticField> magneticField;
    std::unique_ptr<TRestAxionField> axionField;

    std::vector<std::chrono::milliseconds> timeComputation;
    std::vector<double> probability;
    std::vector<double> error;

    double meanProbability;
    double meanError;
    double meanTime;
};

constexpr bool kDebug = true;

Int_t REST_Axion_BMapsSysAnalysis(Int_t nData = 10, Double_t Ea = 4.2, Double_t m1 = 0.3, Double_t m2 = 0.01, std::string gasName = "He",
                                 Int_t num_intervals = 100, Int_t qawo_levels = 20) {
    // Create Variables
    const char* cfgFileName = "fields.rml";
    const TVector3 position(-5, 5, -11000);
    const TVector3 direction = (position - TVector3(5, -5, 11000)).Unit();
    const Double_t gasDensity = 2.9868e-10;

    // Define all four fields
    std::map<std::string, FieldInfo> fields;
    fields["MentinkCut"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_2024_cutoff"), std::make_unique<TRestAxionField>()};
    fields["Mentink"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_2024"), std::make_unique<TRestAxionField>()};
    fields["Bykovskiy2019"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO"), std::make_unique<TRestAxionField>()};
    fields["Bykovskiy2020"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_HD"), std::make_unique<TRestAxionField>()};

    //Just for plotting the profile of all four maps
    const std::vector<std::string> fieldNames = {"babyIAXO_2024", "babyIAXO", "babyIAXO_HD"};

    // Set up buffer gas
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    Int_t h = 1;
    // Set up Axion field and assign magentic field, just plot all the maps once
    for (auto& field : fields) {
        if (gas != nullptr) {
            field.second.axionField->AssignBufferGas(gas.get());
        }
        if(h == 1){
            field.second.magneticField->DrawTrackProfile(TVector3(0,0,11000), 100, fieldNames, true);
            h++;
        }
        field.second.magneticField->SetTrack(position, direction);
        field.second.axionField->AssignMagneticField(field.second.magneticField.get());
    } 

    // Determine masses based on resonance condition
    std::vector<Double_t> masses;
    for (unsigned i = 0; i < 3; ++i) {
        if (i < 2)
            masses.push_back(i == 0 ? m1 : m2);
        else
            masses.push_back(gas != nullptr ? gas->GetPhotonMass(Ea) : 0);
    }

    std::vector<Double_t> accuracyValues = {0.25};
    for (const auto &accuracy : accuracyValues){
        // Iterate over each mass and map field
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
                std::cout << "Mass value: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            // Calculate transmission probability and computation time for each field map
            for (Int_t i = 0; i < nData; i++) {                
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Data: " << i << std::endl;
                    std::cout << std::endl;
                }
                for (auto &field : fields) {
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, num_intervals, qawo_levels);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                    field.second.timeComputation.push_back(duration);
                    field.second.probability.push_back(probField.first);
                    field.second.error.push_back(probField.second);

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
            std::ostringstream ossMass, ossAccuracy;
            const std::string folder = "BMapsAnalysis/";
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }
            if (ma == (gas != nullptr ? gas->GetPhotonMass(Ea) : 0)) {   
                ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
                filename = folder + "REST_AXION_FieldBMaps_OnResonance_Accuracy" + ossAccuracy.str() + ".txt";
            } else {
                ossMass << std::fixed << std::setprecision(2) << ma;
                ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
                filename = folder + "REST_AXION_FieldBMaps_OffResonance_Accuracy_" + ossAccuracy.str() + "_Mass_" + ossMass.str() + ".txt";
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

            // Write metadata to the output file, calculated probabilities and computation times for each field map to the output file
            outputFile << (ma != gas->GetPhotonMass(Ea) ? "Off resonance, ma: " : "On resonance, ma: ") << ma << "Accuracy: " << accuracy << std::endl;
            outputFile << "FieldName\tProbability\tError\tTime(ms)\n";
            for (const auto& field : fields) {
                outputFile << field.first << "\t" << field.second.meanProbability << "\t" << field.second.meanError << "\t" << field.second.meanTime << "\n";
            }

            // Debug message: Closing file
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Closing file: " << filename << std::endl;
            }
            outputFile.close();
        }
    }
    return 0;
}