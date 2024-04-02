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

//*******************************************************************************************************
//*** Description: This script analyzes the MagneticField class to determine whether it is beneficial to enable or 
//*** disable trilinear interpolation when calculating the magnetic field at an arbitrary point.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 30).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Axion mass in eV (default: 0.1).
//*** - OnResonance: Flag indicating whether to calculate transmission probability on resonance (default: false).
//*** - Accuracy: Accuracy value for intrgration in GSL, depends on the axion mass (default 0.15).
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


Int_t REST_Axion_InterpolationAnalysis(Int_t nData = 5, Double_t Ea = 4.2, std::string gasName = "He", 
                    Double_t mi = 0.1, Bool_t OnResonance = false, Double_t accuracy = 0.25){

    Bool_t fDebug = true;

    // Create Variables
    std::string fieldName = "babyIAXO_2024_cutoff";
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-100, -100 ,-11000);
    TVector3 direction(0.01, 0.01 ,1);

    std::map<std::string, FieldTrack> fields = {
        {"Interpolation", {true}},
        {"No-Interpolation", {false}}
    };

    // Create an instance of TRestAxionBufferGas if gasName is provided and TRestAxionMagneticField
    TRestAxionMagneticField *magneticField = new TRestAxionMagneticField("fields.rml", fieldName);
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Create an instance of TRestAxionField and assign magnetic field and gas (if provided)
    TRestAxionField *axionField = new TRestAxionField();
    if (gas != nullptr) {
        axionField->AssignBufferGas(gas);
    }
    magneticField->SetTrack(position,direction);
    axionField->AssignMagneticField(magneticField);  

    // Determine mass based on resonance condition
    Double_t ma;
    if (OnResonance) {
        if (gas != nullptr)
            ma = gas->GetPhotonMass(Ea);
        else
            ma = 0;
    } else
        ma = mi;

    // Iterate over each bool, setting it on and off    
    for(Int_t i = 0; i<nData; i++){
        for(auto& field : fields){
            magneticField->SetInterpolation(field.second.interpolation);
            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            field.second.probability.push_back(probField.first);
            field.second.error.push_back(probField.second);
            field.second.timeComputation.push_back(duration);

            if(fDebug){
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
    std::ofstream outputFile("REST_AXION_Interpolation_results.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open the file for writing!" << std::endl;
        return 1;
    }

    if(OnResonance)
        outputFile << "On resonance, Accuracy : " << accuracy << std::endl;
    else
        outputFile << "Off resonance, ma : " << ma << ", Accuracy : " << accuracy << std::endl;
    outputFile << "Interpolation\tProbability\tError\tTime(ms)\n";
    for (const auto& field : fields) {
        outputFile << field.first << "\t" << field.second.meanProbability << "\t" << field.second.meanError << "\t" << field.second.meanTime << "\n";
    }
    outputFile.close();

    delete gas;
    delete magneticField;
    delete axionField;

    return 0;
}