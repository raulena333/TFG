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
//*** Description: This script performs analysis on different magnetic field maps represented as grids with varying 
//*** mesh sizes. It also calculates the transmission probability, error and runtime for each grid configuration at 
//*** different axion masses 
//*** 
//*** Mesh Map Definitions in mm:
//*** (20, 20, 100), (30, 30, 150), (50, 50, 250), (50, 50, 500)
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Axion mass in eV (default: 0.1).
//*** - OnResonance: Flag indicating whether to calculate transmission probability on resonance (default: false).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::ReMap'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************


struct FieldTrack {
    TRestAxionMagneticField* magneticField;
    TRestAxionField* axionField;
    const TVector3 mapSize; 

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<std::chrono::milliseconds> timeComputation;

    double meanError;
    double meanProbability;
    double meanTime;
};


Int_t REST_Axion_GridAnalysis(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", 
                                        Double_t mi = 0.01, Bool_t OnResonance = true) {
    Bool_t fDebug = true;

    // Mesh Map Definitions in mm
    std::vector<TVector3> meshSizes = {
        TVector3(10,10,50),
        TVector3(20, 20, 100),
        TVector3(30, 30, 150),
        TVector3(50, 50, 250),
        TVector3(100, 100, 500)
    };

    // Create Variables
    std::string fieldName = "babyIAXO_2024_cutoff";
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-100, -100 ,-11000);
    TVector3 direction(0.01, 0.01 ,1);

    std::map<std::string, FieldTrack> fields;
    for (size_t i = 0; i < meshSizes.size(); ++i) {
        std::string gridName = "Grid" + std::to_string(i + 1);
        fields.emplace(gridName, FieldTrack{
            new TRestAxionMagneticField("fields.rml", fieldName),
            new TRestAxionField(),
            meshSizes[i]
        });
    }

    // Create an instance of TRestAxionBufferGas if gasName is provided
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Create an instance of TRestAxionField and assign magnetic field and gas (if provided). Remap the grid
    for (auto& field : fields) {
        if (gas != nullptr) {
            field.second.axionField->AssignBufferGas(gas);
        }
        for (size_t n = 0; n < field.second.magneticField->GetNumberOfVolumes(); n++)
            field.second.magneticField->ReMap(n, field.second.mapSize);
        field.second.magneticField->SetTrack(position, direction);
        field.second.axionField->AssignMagneticField(field.second.magneticField);
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

    // Iterate over each grid map mesh
    for(unsigned i = 0; i < nData; i++){
        for(auto& field : fields){
            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 100, 20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            field.second.probability.push_back(probField.first);
            field.second.error.push_back(probField.second);
            field.second.timeComputation.push_back(duration);

            if(fDebug){
                std::cout<<field.first<<std::endl;
                std::cout<<"Probability: "<<probField.first<<std::endl;
                std::cout<<"Error: "<<probField.second<<std::endl;
                std::cout<<"Runtime: "<<duration.count()<<std::endl;
                std::cout<<std::endl;
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
    std::ofstream outputFile("REST_AXION_Grid_results.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open the file for writing!" << std::endl;
        return 1;
    }

    if(OnResonance)
        outputFile << "On resonance" << std::endl;
    else
        outputFile << "Off resonance, ma :" << ma <<std::endl;
    outputFile << "Grid\tSize\tProbability\tError\tTime(ms)\n";
    for (const auto& field : fields) {
        outputFile << field.first << "\t (" << field.second.mapSize.X() << "," << field.second.mapSize.Y() << "," <<field.second.mapSize.Z()<< ")\t "
                    << field.second.meanProbability << "\t" << field.second.meanError << "\t" << field.second.meanTime << "\n";
    }
    outputFile.close();

    delete gas;
    for (auto& field : fields) {
        delete field.second.magneticField;
        delete field.second.axionField;
    }

    return 0;
}