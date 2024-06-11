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
//*** - nData: Number of data points to generate (default: 20).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - m1: Axion mass in eV (default: 0.01).
//*** - Accuracy: Accuracy value for integration in GSL, depends on the axion mass (default 0.5).
//*** - dL: differential element in mm (default 10).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetInterpolation'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability' and `TRestAxionField::GammaTransmissionProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct FieldTrack {
    bool interpolation;
    std::unique_ptr<TRestAxionMagneticField> magneticField;
    std::unique_ptr<TRestAxionField> axionField;
    std::vector<double> mValues;

    std::vector<double> probabilityG;
    std::vector<double> errorG;
    std::vector<std::chrono::milliseconds> timeComputationG;

    std::vector<double> probabilityS;
    std::vector<std::chrono::microseconds> timeComputationS;

    double meanErrorG;
    double meanProbabilityG;
    double meanTimeG;

    double meanProbabilityS;
    double meanTimeS;
};

constexpr bool kDebug = true;

Int_t REST_Axion_InterpolationAnalysisWithStandard(Int_t nData = 5, Double_t Ea = 4.2, std::string gasName = "He", 
                    Double_t m1 = 0.1, Double_t accuracy = 0.25, Double_t dL = 1){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 initialPosition(-5, 5 , -11000);
    TVector3 finalPosition(5, -5, 11000);
    TVector3 direction = (finalPosition - initialPosition).Unit();

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine masses based on resonance condition
    std::vector<Double_t> masses;
    for (unsigned i = 0; i < 2; ++i) {
        if (gas) {
            masses.push_back(i == 0 ? m1 : gas->GetPhotonMass(Ea));
        } else {
            masses.push_back(i == 0 ? m1 : 0);
        }
    }

    std::string folder = "InterpolationAnalysis/";
    for(const auto &fieldName : fieldNames){
        std::map<std::string, FieldTrack> fields;
        fields["Interpolation"] = {true, std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName), std::make_unique<TRestAxionField>()};
        fields["No-Interpolation"] = {false, std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName), std::make_unique<TRestAxionField>()};

        for(auto &field: fields){
            if (gas != nullptr) 
                field.second.axionField->AssignBufferGas(gas.get());

            field.second.axionField->AssignMagneticField(field.second.magneticField.get());
            field.second.magneticField->SetInterpolation(field.second.interpolation);

            field.second.magneticField->SetTrack(initialPosition, direction);
            std::vector<Double_t> magneticValues = field.second.magneticField->GetTransversalComponentAlongPath(initialPosition, finalPosition, dL);
            field.second.mValues = magneticValues;
        }

   
        for(const auto &ma : masses){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            // Set interpolation on or off
                for(auto &field : fields){
                for(Int_t i = 0; i<nData; i++){
                    if(kDebug){
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << "Data: " << i << std::endl;
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << std::endl;
                    }

                    // GSL integration
                    auto start_time_gsl = std::chrono::high_resolution_clock::now();
                    std::pair<Double_t, Double_t> probFieldG = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
                    auto end_time_gsl = std::chrono::high_resolution_clock::now();
                    auto duration_gsl = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_gsl - start_time_gsl);
                    
                    field.second.probabilityG.push_back(probFieldG.first);
                    field.second.errorG.push_back(probFieldG.second);
                    field.second.timeComputationG.push_back(duration_gsl);

                    if(kDebug){
                        std::cout << "GSL Integration" << std::endl;
                        std::cout << field.first << std::endl;
                        std::cout << "Probability: " << probFieldG.first << std::endl;
                        std::cout << "Error: " << probFieldG.second << std::endl;
                        std::cout << "Runtime (ms): " << duration_gsl.count() << std::endl;
                        std::cout << std::endl;
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    }

                    // Standard integration
                    auto start_time_s = std::chrono::high_resolution_clock::now();
                    Double_t probFieldS = field.second.axionField->GammaTransmissionProbability(field.second.mValues, dL, Ea, ma);
                    auto end_time_s = std::chrono::high_resolution_clock::now();
                    auto duration_s = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_s - start_time_s);

                    field.second.probabilityS.push_back(probFieldS);
                    field.second.timeComputationS.push_back(duration_s);

                    if(kDebug){
                        std::cout << "Standard Integration" << std::endl;
                        std::cout << field.first << std::endl;
                        std::cout << "Probability: " << probFieldS << std::endl;
                        std::cout << "Runtime (μs): " << duration_s.count() << std::endl;
                        std::cout << std::endl;
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    }

                }
            }

            // Calculate means
            for (auto &field : fields) {
                // GSL means
                field.second.meanErrorG = std::accumulate(field.second.errorG.begin(), field.second.errorG.end(), 0.0) / nData;
                field.second.meanProbabilityG = std::accumulate(field.second.probabilityG.begin(), field.second.probabilityG.end(), 0.0) / nData;
                field.second.meanTimeG = std::accumulate(field.second.timeComputationG.begin(), field.second.timeComputationG.end(), std::chrono::milliseconds(0)).count() / nData;

                // Standard means
                field.second.meanProbabilityS = std::accumulate(field.second.probabilityS.begin(), field.second.probabilityS.end(), 0.0) / nData;
                field.second.meanTimeS = std::accumulate(field.second.timeComputationS.begin(), field.second.timeComputationS.end(), std::chrono::microseconds(0)).count() / nData;
            }

            // Open the file for writing
            std::string folder = "InterpolationAnalysis/";
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }
            std::string filenameG, filenameS;
            // Open file for GSL Integration
            if (ma == (gas ? gas->GetPhotonMass(Ea) : 0)) {
                filenameG = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysisGSL_results_OnResonance.txt";
            } else {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << ma;
                filenameG = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysisGSL_results_OffResonance_Mass_" + oss.str() + ".txt";
            }

            // Open file for Standard Integration
            if (ma == (gas ? gas->GetPhotonMass(Ea) : 0)) {
                filenameS = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysisStandard_results_OnResonance.txt";
            } else {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << ma;
                filenameS = folder + "REST_AXION_" + fieldName + "_InterpolationAnalysisStandard_results_OffResonance_Mass_" + oss.str() + ".txt";
            }

            // Debug message: Opening file GSL
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Opening file: " << filenameG  << std::endl;
            }

            std::ofstream outputFileG(filenameG);
            if (!outputFileG.is_open()) {
                std::cerr << "Error: Unable to open the file for writing!" << std::endl;
                return 1;
            }

            outputFileG << "GSL Integration " << std::endl;
            outputFileG << (ma != (gas ? gas->GetPhotonMass(Ea) : 0) ? "Off resonance, ma: " : "On resonance, ma: ") << ma << "  Accuracy: " << accuracy <<  std::endl;
            outputFileG << "Interpolation\tProbability\tError\tTime(ms)\n";
            for (const auto& field : fields) {
                outputFileG << field.first << "\t" << field.second.meanProbabilityG << "\t" << field.second.meanErrorG << "\t" << field.second.meanTimeG << "\n";
            }

            // Debug message: Closing file
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Closing file: " << filenameG << std::endl;
            }
            outputFileG.close();

            // Debug message: Opening file Standard
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Opening file: " << filenameS  << std::endl;
            }

            std::ofstream outputFileS(filenameS);
            if (!outputFileS.is_open()) {
                std::cerr << "Error: Unable to open the file for writing!" << std::endl;
                return 1;
            }

            outputFileS << "Standard Integration " << std::endl;
            outputFileS << (ma != (gas ? gas->GetPhotonMass(Ea) : 0) ? "Off resonance, ma: " : "On resonance, ma: ") << ma << " dL: " << dL << std::endl;
            outputFileS << "Interpolation\tProbability\tTime(μs)\n";
            for (const auto &field : fields) {
                outputFileS << field.first << "\t" << field.second.meanProbabilityS << "\t" << "\t" << field.second.meanTimeS << "\n";
            }

            // Debug message: Closing file
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Closing file: " << filenameS << std::endl;
            }
            outputFileS.close();
       }
    }
    return 0;
}