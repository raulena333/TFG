#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>

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
//*** - nData: Number of data points to generate (default: 30).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - mi: Axion mass in eV (default: 0.1).
//*** - OnResonance: Flag indicating whether to calculate transmission probability on resonance (default: true).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability` and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct gasTrack
{
    TRestAxionField *axionField;
    TRestAxionBufferGas *gas;
    std::string gasName;    

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<std::chrono::milliseconds> timeComputation;

    double meanError;
    double meanProbability;
    double meanTime;
};

Int_t REST_Axion_GasAnalysis(Int_t nData = 30, Double_t Ea = 4.2, Double_t mi = 0.01, Bool_t onResonance = false){
    Bool_t fDebug = true;

    // Create Variables
    const char* cfgFileName = "fields.rml";
    std::string fieldName = "babyIAXO_2024_cutoff";
    TVector3 position(-100, -100, -11000);
    TVector3 direction(0.01, 0.01, 1);
    Double_t gasDensity = 2.9868e-9;    

    // Create magnetic field
    TRestAxionMagneticField *magneticField = new TRestAxionMagneticField(cfgFileName, fieldName);

    // Create gas tracks
    std::map<std::string, gasTrack> gasTracks = {
        {"He-Gas", {new TRestAxionField(), new TRestAxionBufferGas(), "He"}},
        {"Vacuum", {new TRestAxionField(), nullptr, ""}}
    };

    // Assign gas and magnetic field to axion field to each gas track
    for (auto &track : gasTracks)
    {
        if (track.second.gas != nullptr)
        {
            track.second.gas->SetGasDensity(track.second.gasName, gasDensity);
            track.second.axionField->AssignBufferGas(track.second.gas);
        }
        track.second.axionField->AssignMagneticField(magneticField);
        magneticField->SetTrack(position, direction);
    }

    // Simulation loop
    for (Int_t i = 0; i < nData; i++)
    {
        for (auto &track : gasTracks)
        {   
            // Calculate mass for Axion
            Double_t ma = onResonance ? (track.second.gas != nullptr ? track.second.gas->GetPhotonMass(Ea) : 0) : mi;

            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = track.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 100, 20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            track.second.probability.push_back(probField.first);
            track.second.error.push_back(probField.second);
            track.second.timeComputation.push_back(duration);

            if (fDebug)
            {
                std::cout << track.first << std::endl;
                std::cout << "Probability: " << probField.first << std::endl;
                std::cout << "Error: " << probField.second << std::endl;
                std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
                std::cout << std::endl;
            }
        }
    }

    // Calculate means
    for (auto &track : gasTracks)
    {
        track.second.meanError = std::accumulate(track.second.error.begin(), track.second.error.end(), 0.0) / nData;
        track.second.meanProbability = std::accumulate(track.second.probability.begin(), track.second.probability.end(), 0.0) / nData;
        track.second.meanTime = std::accumulate(track.second.timeComputation.begin(), track.second.timeComputation.end(), std::chrono::milliseconds(0)).count() / nData;
    }

    // Open the file for writing
    std::ofstream outputFile("REST_AXION_Gas_results.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open the file for writing!" << std::endl;
        return 1;
    }

    // Write results to the file
    if(onResonance)
        outputFile << "On resonance " << std::endl;
    else
        outputFile << "Off resonance, ma: " << mi << std::endl;
    outputFile << "Gas\tProbability\tError\tTime(ms)\n";
    for (const auto &track : gasTracks)
    {
        outputFile << track.first << "\t" << track.second.meanProbability << "\t" << track.second.meanError << "\t" << track.second.meanTime << "\n";
    }
    outputFile.close();

        // Clean up memory
    delete magneticField;
    for (auto &track : gasTracks)
    {
        delete track.second.axionField;
        if (track.second.gas != nullptr)
            delete track.second.gas;
    }

    return 0;
}