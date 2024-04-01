#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>
#include <numeric>
#include <TLatex.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"


//*******************************************************************************************************
//*** Description: This script analyzes the magnetic field along various tracks and calculates the transmission
//*** probability and computation time for each track.
//***
//*** Track Definitions:
//*** - Center Point: Points near the center of the volume along each axis.
//*** - Extreme Point: Points at the extreme ends of the volume along each axis.
//*** - Random Points: Randomly selected points within the volume.
//*** - Outside Point: Physically imposible due to the alignment of the detector with the sun.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - m1: Axion mass in eV (default: 0.1).
//*** - m2: Axion mass in eV (default: 0.01).
//*** - dL: The differential element in mm (default: 10)
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`,
//*** TRestAxionMagneticField::DrawTracks' extension of my own, `TRestAxionField::GammaTransmissionProbability` 
//*** and `TRestAxionBufferGas::GetPhotonMass`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************



struct FieldTrack {
    TVector3 startPoint;
    TVector3 endPoint;
    std::vector<double> magneticValues;
    std::vector<double> probability;
    std::vector<double> timeComputationProb;
    double meanProbability = 0.0;
    double meanTimeProb = 0.0;
    double timeGet = 0.0;
};

Int_t REST_Axion_AnalysisMagenticField(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", Double_t m1 = 0.1, 
                                        Double_t m2 = 0.01, Double_t dL = 10.0) {
    const bool fDebug = true;
    const bool fPlot = true;
    const bool fSave = true;

    // Create Variables
    const std::string fieldName = "babyIAXO_2024_cutoff";
    const Double_t gasDensity = 2.9836e-10;

    // Create an instance of TRestAxionMagneticField
    TRestAxionMagneticField *field = new TRestAxionMagneticField("fields.rml", fieldName);

    //Save Plot of Tracks
    if(fPlot)
        field->DrawTracks(startPoints, endPoints, 100, fSave);

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
        auto start_time = std::chrono::high_resolution_clock::now();
        fieldTrack.second.magneticValues = field->GetTransversalComponentAlongPath(fieldTrack.second.startPoint, fieldTrack.second.endPoint, dL);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        fieldTrack.second.timeGet = duration.count();

        if(fDebug){
            std::cout << "Time: " << duration.count() << " ms" << std::endl;    
            std::cout << fieldTrack.first << " magneticValues:" << std::endl;
            for (const auto& value : fieldTrack.second.magneticValues) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
    }

    // Determine masses based on resonance condition
    std::vector<Double_t> masses;
    for (unsigned i = 0; i < 3; ++i) {
        if (i < 2)
            masses.push_back(i == 0 ? m1 : m2);
        else
            masses.push_back(gas != nullptr ? gas->GetPhotonMass(Ea) : 0);
    }

    // Iterate over each mass and field track
    for (const auto& ma : masses) {
        // Reset probability and timeComputationProb vectors to empty for each mass
        for (auto& field : fieldTracks) {
            field.second.probability.clear();
            field.second.timeComputationProb.clear();
        }
        for (unsigned j = 0; j < nData; j++) {
            for (auto& field : fieldTracks) {
                auto start_time = std::chrono::high_resolution_clock::now();
                Double_t probField = axionField->GammaTransmissionProbability(field.second.magneticValues, dL, Ea, ma);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

                field.second.probability.push_back(probField);
                field.second.timeComputationProb.push_back(duration.count());

                if (fDebug) {
                    std::cout << field.first << std::endl;
                    std::cout << "Probability: " << probField << std::endl;
                    std::cout << "Runtime: " << duration.count() << " μs" << std::endl;
                    std::cout << std::endl;
                }
            }
        }

        // Calculate means
        for (auto& field : fieldTracks) {
            field.second.meanProbability = std::accumulate(field.second.probability.begin(), field.second.probability.end(), 0.0) / nData;
            field.second.meanTimeProb = std::accumulate(field.second.timeComputationProb.begin(), field.second.timeComputationProb.end(), 0.0) / nData;
        }

        // Open the file for writing
        std::string filename;
        if (ma == (gas != nullptr ? gas->GetPhotonMass(Ea) : 0)) {
            filename = "REST_AXION_Magnetic_results_OnResonance.txt";
        } else {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << ma;
            filename = "REST_AXION_Magnetic_results_OffResonance_Mass_" + oss.str() + ".txt";
        }

        // Debug message: Opening file
        if (fDebug) {
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << "Opening file: " << filename << std::endl;
        }

        std::ofstream outputFile(filename);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Unable to open the file for writing!" << std::endl;
            return 1;
        }

        outputFile << (ma == 0 ? "On resonance, dL : " : "Off resonance, dL : ") << dL << (ma == 0 ? "" : ", Axion-Mass :") << ma << std::fixed << std::setprecision(2) << std::endl;
        outputFile << "Direction\tProbability\tTimeProb (μs)\t TimeGet (ms)\n";
        for (const auto& field : fieldTracks)
            outputFile << field.first << "\t" << field.second.meanProbability << "\t" << field.second.meanTimeProb << "\t" << field.second.timeGet << "\n";
            
        // Debug message: Closing file
        if (fDebug) {
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << "Closing file: " << filename << std::endl;
        }

        outputFile.close();
    }

    return 0;
}