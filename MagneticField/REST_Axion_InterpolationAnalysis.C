#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>

#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"
#include <TLatex.h>

#include <filesystem> 

//*******************************************************************************************************
//*** Description: This script analyzes the MagneticField class to determine whether it is beneficial to enable or 
//*** disable trilinear interpolation when calculating the magnetic field at an arbitrary point.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - m1: Axion mass in eV (default: 0.01).
//*** - m2: Axion mass in eV (default: 0.3).
//*** - Accuracy: Accuracy value for integration in GSL, depends on the axion mass (default 0.25).
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
    std::vector<double> error;
    std::vector<std::chrono::milliseconds> timeComputation;

    double meanError;
    double meanProbability;
    double meanTime;
};

constexpr bool kDebug = true;

Int_t REST_Axion_InterpolationAnalysis(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", 
                    Double_t m1 = 0.01, Double_t m2 = 0.3 , Double_t accuracy = 0.52){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-5, 5 , -9000);
    TVector3 direction = (position - TVector3(5, -5, 9000));

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

    // Determine masses based on resonance condition
    std::vector<Double_t> masses;
    for (unsigned i = 0; i < 3; ++i) {
        if (gas) {
            masses.push_back(i == 0 ? m1 : (i == 1 ? m2 : gas->GetPhotonMass(Ea)));
        } else {
            masses.push_back(i == 0 ? m1 : m2);
        }
    }

    for(const auto& fieldName : fieldNames){
        // Create an instance of TRestAxionField and assign magnetic field and gas (if provided).
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        if (gas != nullptr) 
            axionField->AssignBufferGas(gas.get());

        magneticField->SetTrack(position, direction);
        axionField->AssignMagneticField(magneticField.get()); 

        for(const auto& ma : masses){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            // Iterate over each bool, setting it on and off    
            for(Int_t i = 0; i<nData; i++){
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Data: " << i << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }

                for(auto& field : fields){
                    magneticField->SetInterpolation(field.second.interpolation);
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 400, 50);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                    field.second.probability.push_back(probField.first);
                    field.second.error.push_back(probField.second);
                    field.second.timeComputation.push_back(duration);

                    if(kDebug){ // Fixed typo: Changed fDebug to kDebug
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
            std::string folder = "InterpolationAnalysis/";
            std::filesystem::create_directory(folder); 
            std::string filename;
            if (ma == (gas ? gas->GetPhotonMass(Ea) : 0)) {
                filename = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysis_results_OnResonance.txt";
            } else {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << ma;
                filename = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysis_results_OffResonance_Mass_" + oss.str() + ".txt";
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

            outputFile << (ma != (gas ? gas->GetPhotonMass(Ea) : 0) ? "Off resonance, ma: " : "On resonance, ma: ") << ma << std::endl;;
            outputFile << "Interpolation\tProbability\tError\tTime(ms)\n";
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
