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
//*** Description: This script analyzes the magnetic field along various tracks and calculates the transmission
//*** probability and computation time for each track.
//***
//*** Track Definitions:
//*** - Center Points: Points near the center of the volume along each axis.
//*** - Extreme Points: Points at the extreme ends of the volume along each axis.
//*** - Random Points: Randomly selected points within the volume.
//*** - Symmetric Points: Symmetrically positioned points with respect to the center along each axis.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Axion mass in eV (default: 0.1).
//*** - dL: The differential element in mm (default: 10)
//*** - OnResonance: Flag indicating whether to calculate transmission probability on resonance (default: false).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`,
//*** `TRestAxionField::GammaTransmissionProbability` and `TRestAxionBufferGas::GetPhotonMass`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************



struct FieldTrack {
    TVector3 StartPoint;
    TVector3 EndPoint;

    std::vector<double> magneticValues;
    std::vector<double> probability;
    std::vector<double> timeComputation;

    double meanProbability;
    double meanTime;
};

Int_t REST_Axion_AnalysisMagenticField(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.01, 
                                        Double_t dL = 10, Bool_t OnResonance = true) {
    Bool_t fDebug = true;

    // Create Variables
    std::string fieldName = "babyIAXO_2024_cutoff";
    Double_t gasDensity = 2.9836e-10;
    std::vector<Double_t> mass;

    // Define all tracks
    std::map<std::string, FieldTrack> fieldTracks = {
        {"Center", {TVector3(0, 0, 0), TVector3(0, 0, 1000)}},
        {"Extreme", {TVector3(-350, -350, -6000), TVector3(350, 350, 6000)}},
        {"Symmetric", {TVector3(200, -200, -4000), TVector3(-200, 200, 4000)}},
        {"Random", {TVector3(-150, 200, -3000), TVector3(250, -100, 2000)}},
        {"Random1", {TVector3(-300, 100, -500), TVector3(100, -250, 1000)}},
        {"Random2", {TVector3(-50, -400, -2000), TVector3(150, 300, 3000)}},
        {"Random3", {TVector3(400, -150, -1000), TVector3(-200, 50, 1500)}},
        {"Random4", {TVector3(-250, 350, -400), TVector3(200, -300, 500)}},
        {"Random5", {TVector3(50, -250, -1500), TVector3(-300, 400, 2000)}}
    };

    // Create an instance of TRestAxionMagneticField
    TRestAxionMagneticField *field = new TRestAxionMagneticField("fields.rml", fieldName);

    // Create an instance of TRestAxionBufferGas if gasName is provided
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Create an instance of TRestAxionField and assign magnetic field and gas (if provided)
    TRestAxionField* axionField = new TRestAxionField();
    axionField->AssignMagneticField(field);
    if (gas != nullptr) {
        axionField->AssignBufferGas(gas);
    }

    // Calculate magnetic field values for each track
    for (auto& fieldTrack : fieldTracks) {
        fieldTrack.second.magneticValues = field->GetTransversalComponentAlongPath(fieldTrack.second.StartPoint, fieldTrack.second.EndPoint, dL);
        if(fDebug)
        {
        std::cout << fieldTrack.first << " magneticValues:" << std::endl;
        for (const auto& value : fieldTrack.second.magneticValues) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
        }
    }

    // Determine mass based on resonance condition
    Double_t ma;
    if (OnResonance) {
        if (gas != nullptr)
            ma = gas->GetPhotonMass(Ea);
        else
            ma = 0;
    } else
        ma = mi;

    // Iterate over each field track
    for (unsigned j = 0; j < nData; j++) {
        for (auto& field : fieldTracks) {
            auto start_time = std::chrono::high_resolution_clock::now();
            Double_t probField = axionField->GammaTransmissionProbability(field.second.magneticValues, 10, Ea, ma);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            field.second.probability.push_back(probField);
            field.second.timeComputation.push_back(duration.count());

            if (fDebug) {
                std::cout << field.first << std::endl;
                std::cout << "Probability: " << probField << std::endl;
                std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
                std::cout << std::endl;
            }
        }
    }

    // Calculate means
    for (auto& field : fieldTracks) {
        field.second.meanProbability = std::accumulate(field.second.probability.begin(), field.second.probability.end(), 0.0) / nData;
        field.second.meanTime = std::accumulate(field.second.timeComputation.begin(), field.second.timeComputation.end(), 0.0) / nData;
    }

    // Open the file for writing
    std::ofstream outputFile("REST_AXION_Magnetic_results.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open the file for writing!" << std::endl;
        return 1;
    }

    if(OnResonance)
        outputFile << "On resonance, dL : " << dL << std::endl;
    else
        outputFile << "Off resonance, dL : " << dL << ", Axion-Mass :" << ma <<std::endl;

    outputFile << "Direction\tProbability\tTime(ms)\n";
    for (const auto& field : fieldTracks)
        outputFile << field.first << "\t" << field.second.meanProbability << "\t" << field.second.meanTime << "\n";
    outputFile.close();

    delete gas;
    delete field;
    delete axionField;

    return 0;
}
