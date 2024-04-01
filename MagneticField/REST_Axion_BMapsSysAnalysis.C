#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>
#include <numeric>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"
#include <TLatex.h>

//*******************************************************************************************************
//*** Description: This script analyzes and calculates the probability and error for each magnetic field map definition in the specified configuration file.
//***
//*** Field Map Definitions:
//*** - MentiskCut: Magnetic field map with 1cm precision in X and Y axes and 5cm in Z axis for the babyIAXO magnet. Cut-off in Z-range (-6,6)m.
//*** - Mentisk: Magnetic field map with 1cm precision in X and Y axes and 5cm in Z axis for the babyIAXO magnet. Full length.
//*** - Bykovskiy2019: Magnetic field map with 5cm precision in X, Y, and Z axes for the babyIAXO magnet.
//*** - Bykovskiy2020: Magnetic field map with 1cm precision in XY axes and 5cm in Z axis for the babyIAXO magnet.
//***
//*** Default Arguments:
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - mi: Axion mass in eV (default: 0.1).
//*** - gasName: Gas name (default: "He").
//*** - accuracy: The desired accuracy for calculating the transmission probability (default: 0.1).
//*** - num_intervals: Number of intervals for integration (default: 100).
//*** - qawo_levels: Number of levels for QAWO integration (default: 20).
//*** - OnResonance: Flag indicating whether to calculate transmission probability on resonance (default: true).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability` and `TRestAxionBufferGas::GetPhotonMass`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct FieldInfo {
    TRestAxionMagneticField* magneticField;
    TRestAxionField* axionField;

    std::vector<std::chrono::milliseconds> timeComputation;
    std::vector<double> probability;
    std::vector<double> error;

    double meanProbability;
    double meanError;
    double meanTime;
};

Int_t REST_Axion_BMapsSysAnalysis(Int_t nData = 50, Double_t Ea = 4.2, Double_t mi = 0.01, std::string gasName = "He",
                                  Double_t accuracy = 0.1, Int_t num_intervals = 100, Int_t qawo_levels = 20, Bool_t OnResonance = false) {
    Bool_t fDebug = true;

    // Create Variables
    const char* cfgFileName = "fields.rml";
    TVector3 position(-100, -100, -11000);
    TVector3 direction(0.01, 0.01, 1);
    Double_t gasDensity = 2.9868e-9;

    // Define all four fields
    std::map<std::string, FieldInfo> fields = {
        {"MentiskCut", {new TRestAxionMagneticField(cfgFileName, "babyIAXO_2024_cutoff"), new TRestAxionField()}},
        {"Mentisk", {new TRestAxionMagneticField(cfgFileName, "babyIAXO_2024"), new TRestAxionField()}},
        {"Bykovskiy2019", {new TRestAxionMagneticField(cfgFileName, "babyIAXO"), new TRestAxionField()}},
        {"Bykovskiy2020", {new TRestAxionMagneticField(cfgFileName, "babyIAXO_HD"), new TRestAxionField()}}
    };

    // Set up buffer gas
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Set up Axion field
    for (const auto& field : fields) {
        if (gas != nullptr) {
            field.second.axionField->AssignBufferGas(gas);
        }
        field.second.magneticField->SetTrack(position, direction);
        field.second.axionField->AssignMagneticField(field.second.magneticField);
    } 

    // Calculate mass for Axion
    Double_t ma;
    if (OnResonance) {
        if (gas != nullptr)
            ma = gas->GetPhotonMass(Ea);
        else
            ma = 0;
    } else
        ma = mi; 

    // Calculate transmission probability and computation time for each field map
    for (Int_t i = 0; i < nData; i++) {
        for (auto& field : fields) {
            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, num_intervals, qawo_levels);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            field.second.timeComputation.push_back(duration);
            field.second.probability.push_back(probField.first);
            field.second.error.push_back(probField.second);

            if (fDebug) {
                std::cout << field.first << std::endl;
                std::cout << "Probability: " << probField.first << std::endl;
                std::cout << "Error: " << probField.second << std::endl;
                std::cout << "Runtime (ms): " << duration.count() << std::endl;
                std::cout << std::endl;
            }
        }
    }
    
    // Calculate means
    for (auto& field : fields) {
        field.second.meanError = std::accumulate(field.second.error.begin(), field.second.error.end(), 0.0) / nData;
        field.second.meanProbability = std::accumulate(field.second.probability.begin(), field.second.probability.end(), 0.0) / nData;
        field.second.meanTime = std::accumulate(field.second.timeComputation.begin(), field.second.timeComputation.end(), std::chrono::milliseconds(0)).count() / nData;
    }

    // Open the file for writing
    std::ofstream outputFile("REST_AXION_FieldBMaps_results.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open the file for writing!" << std::endl;
        return 1;
    }

    // Write metadata to the output file, calculated probabilities and computation times for each field map to the output file
    if(OnResonance)
        outputFile << "On Resonance "<<std::endl;
    else
        outputFile << "Off Resonance, ma :" << ma <<std::endl;
    outputFile << "FieldName\tProbability\tError\tTime(ms)\n";
    for (const auto& field : fields) {
        outputFile << field.first << "\t" << field.second.meanProbability << "\t" << field.second.meanError << "\t" << field.second.meanTime << "\n";
    }
    outputFile.close();

    delete gas;
    for (auto& field : fields) {
        delete field.second.magneticField;
        delete field.second.axionField;
    }

    return 0;
}