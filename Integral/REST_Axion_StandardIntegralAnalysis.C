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
#include <TGraph.h> 
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** This function performs standard integral analysis for axion experiments. It calculates the runtime and probability values
//*** for different axion masses and integration lengths (dL). The results are plotted on histograms and graphs for visualization.
//*** 
//*** Arguments by default are (in order):
//*** - nData: Number of dL points to generate (default: 200).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the buffer gas (default: "He").
//*** - m1: Initial axion mass in eV far from resonance (default: 0.01).
//*** - m3: Middle axion mass in eV far/close to resonance (default: 0.1).
//*** - m2: Final axion mass in eV close to resonance (default: 0.3).
//*** - dLinitial: Initial integration length (default: 1).
//*** - dLfinal: Final integration length (default: 1000).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`,
//*** `TRestAxionField::GammaTransmissionProbability`, and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_StandardIntegralAnalysis(Int_t nData = 200, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.25, 
            Double_t m1 = 0.01, Double_t m2 = 0.1, Double_t m3 = 0.3, Int_t dLinitial = 1, Int_t dLfinal = 1000){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024" };
    const Double_t gasDensity = 2.9836e-10;
    const TVector3 initialPosition(-5, 5, -11000);
    const TVector3 finalPosition(5,-5, 11000);
    std::vector<Double_t> masses;
    std::vector<Int_t> dLvec;

    for (Int_t j = 0; j < nData; j++) {
        Int_t dL = dLinitial + j * (dLfinal - dLinitial) / nData;
        dLvec.push_back(dL);
    }

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine masses based on resonance condition
    for (unsigned i = 0; i < 4; ++i) {
        if (i < 3)
            masses.push_back(i == 0 ? m1 : (i == 1 ? m2 : m3));
        else
            masses.push_back(gas != nullptr ? gas->GetPhotonMass(Ea) : 0);
    }
    
    for(const auto &fieldName : fieldNames){
        
        // Create instance of magnetic field, axion field and buffer gas and then assign it.
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        if (gas!= nullptr)
            axionField->AssignBufferGas(gas.get());
        axionField->AssignMagneticField(magneticField.get());

        for (const auto &ma : masses){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Axion Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            std::vector<Double_t> probValues;

            for(const auto &dL : dLvec){
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "dL: " << dL << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }

                // Runtime for filling the magnetic values
                auto start_timeStandard = std::chrono::high_resolution_clock::now();
                std::vector<Double_t> magneticValues = magneticField->GetTransversalComponentAlongPath(initialPosition, finalPosition, dL);
                auto end_timeStandard = std::chrono::high_resolution_clock::now();
                auto durationStandard = std::chrono::duration_cast<std::chrono::microseconds>(end_timeStandard - start_timeStandard);

                // Standard Integration
                auto start_time = std::chrono::high_resolution_clock::now();
                Double_t probField = axionField->GammaTransmissionProbability(magneticValues, dL, Ea, ma);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

                probValues.push_back(probField);

                if(kDebug){
                    std::cout << "Probability: " << probField << std::endl;
                    std::cout << "Runtime (μs): " << duration.count() << std::endl;
                    std::cout << std::endl;
                }
            }

            TCanvas *canvas = new TCanvas((fieldName + std::to_string(ma) + "Probability_vs_dL").c_str(), (fieldName + std::to_string(ma) + "Probability_vs_dL").c_str(), 800, 600);
            canvas->cd();

            TGraph *graph = new TGraph(nData, &dLvec[0], &probValues[0]);

            graph->SetTitle("Standard integration for mass: " + std::to_string(ma));
            graph->SetMarkerStyle(8); 
            graph->SetMarkerSize(0.4);  
            graph->SetTitle("");
            graph->GetXaxis()->SetTitle("dL (mm)");
            graph->GetYaxis()->SetTitle("Probability");
            graph->GetXaxis()->SetRange(dLinitial, dLfinal);
            graph->GetXaxis()->SetTitleSize(0.03); 
            graph->GetXaxis()->SetTitleFont(40);  
            graph->GetXaxis()->SetLabelSize(0.025); 
            graph->GetXaxis()->SetLabelFont(40);  
            graph->GetYaxis()->SetTitleSize(0.03); 
            graph->GetYaxis()->SetTitleFont(40);  
            graph->GetYaxis()->SetLabelSize(0.025); 
            graph->GetYaxis()->SetLabelFont(40); 

            graph->Draw("AP");

            if(kSave){
                std::string folder = "Standard_Integral_Analysis/";
                std::ostringstream ossMass;
                ossMass << std::fixed << std::setprecision(2) << ma;
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }
                canvas->SaveAs((folder + fieldName + "_ProbabilityPlot_Standard_Mass_" + ossMass.str() + ".png").c_str());
            }
        }     
    }
    return 0;
}