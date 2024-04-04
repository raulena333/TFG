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
//*** Description: This script analyzes and calculates the probability and error for the last updated magnetic field 
//*** by comparing the results between introducing gas in the chamber or keeping it in a vacuum.
//***
//*** Gas Definitions:
//*** - 'He': Helium gas defined by its density.
//*** - '': Vacuum.
//***
//*** Default Arguments:
//*** - nData: Number of data points to generate (default: 5).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - m1: Axion mass in eV close to resonance Vaccum (default: 0.01).
//*** - m2: Axion mass in eV close to resonance for density of He (default: 0.1).
//*** - m3: Axion mass in eV random in between (default: 0.15).
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
    std::vector<std::chrono::milliseconds> timeComputation;

    double meanError;
    double meanProbability;
    double meanTime;
};

constexpr bool kDebug = true;

Int_t REST_Axion_GasAnalysis(Int_t nData = 5, Double_t Ea = 4.2, Double_t m1 = 0.01, Double_t m2 = 0.1, Double_t m3 = 0.15) {
    // Create Variables
    const char* cfgFileName = "fields.rml";
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    TVector3 position(-5, 5, -9000);
    TVector3 direction = (position - TVector3(5, -5 , 9000));
    Double_t gasDensity = 2.6e-11;   

    // Loop for both magnetic field maps
    for (const auto& fieldName : fieldNames) {
        // Create magnetic field
        auto magneticField = std::make_unique<TRestAxionMagneticField>(cfgFileName, fieldName);

        // Create gas tracks
        std::map<std::string, GasTrack> gasTracks;

        gasTracks.insert(std::make_pair("He-Gas", GasTrack{std::make_unique<TRestAxionField>(), std::make_unique<TRestAxionBufferGas>(), "He"}));
        gasTracks.insert(std::make_pair("Vacuum", GasTrack{std::make_unique<TRestAxionField>(), nullptr, ""}));

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
        for (const auto &ma : {m1, m2, m3}) {
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            for (unsigned i = 0; i < nData; i++) {
                if (kDebug) {
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Data: " << i << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }

                // Iterate over each gas
                for (auto &track : gasTracks) {   
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::pair<double, double> probField = track.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 200, 20);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                    track.second.probability.push_back(probField.first);
                    track.second.error.push_back(probField.second);
                    track.second.timeComputation.push_back(duration);

                    if (kDebug) {
                        std::cout << track.first << std::endl;
                        std::cout << "Probability: " << probField.first << std::endl;
                        std::cout << "Error: " << probField.second << std::endl;
                        std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
                        std::cout << std::endl;
                    }
                }
            }

            // Calculate means
            for (auto &track : gasTracks) {
                track.second.meanError = std::accumulate(track.second.error.begin(), track.second.error.end(), 0.0) / nData;
                track.second.meanProbability = std::accumulate(track.second.probability.begin(), track.second.probability.end(), 0.0) / nData;
                track.second.meanTime = std::accumulate(track.second.timeComputation.begin(), track.second.timeComputation.end(), std::chrono::milliseconds(0)).count() / nData;
            }

            // Open the file for writing
            std::string folder = "GasAnalysis/";
            std::filesystem::create_directory(folder); 
            std::string filename;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << ma;
            filename = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysis_results_OffResonance_Mass_" + oss.str() + ".txt";

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

            outputFile << "Off resonance, ma: " << ma << std::endl;
            outputFile << "Gas\tProbability\tError\tTime(ms)\n";
            for (const auto &track : gasTracks) {
                outputFile << track.first << "\t" << track.second.meanProbability << "\t" << track.second.meanError << "\t" << track.second.meanTime << "\n";
            }

            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Closing file: " << filename << std::endl;
            }
            outputFile.close();
        }
    }

    return 0;
}