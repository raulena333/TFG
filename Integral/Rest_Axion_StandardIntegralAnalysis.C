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
#include <TH2D.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** 
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - mi: Initial axion mass in eV (default: 0.3).
//*** - mf: Final axion mass in eV (default: 0.4).
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

Int_t REST_Axion_IntegralAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.3, 
            Double_t mf = 0.4, Int_t dLinitial = 1, Int_t dLfinal = 500){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 initialPosition(-5, 5, -11000);
    TVector3 finalPosition(5,-5, 11000);
    std::vector<Double_t> mass;
    std::vector<Int_t> dLvec;

    for (Int_t j = 0; j < nData; j++) {
        mass.push_back(mi + j * (mf - mi) / nData);
        Int_t dL = dLinitial + j * (dLfinal - dLinitial) / nData;
        dLvec.push_back(dL);
    }

    for(const auto &fieldName : fieldNames){
        
        // Create instance of magentic field, axcion field and buffer gas and then assign it.
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        std::unique_ptr<TRestAxionBufferGas> gas;
        if (!gasName.empty()) {
            gas = std::make_unique<TRestAxionBufferGas>();
            gas->SetGasDensity(gasName, gasDensity);
            axionField->AssignBufferGas(gas.get());
        }
        axionField->AssignMagneticField(magneticField.get());

        // Create TCanvas for plotting
        auto canvasRuntime = std::make_unique<TCanvas>((fieldName + "_Runtime").c_str(), (fieldName + "_Runtime").c_str(), 850, 673);
        auto canvasProbability = std::make_unique<TCanvas>((fieldName + "_Probability").c_str(), (fieldName + "_Probability").c_str(), 850, 673);
        
        // Create 2D histograms
        auto histRuntime = std::make_unique<TH2D>("histRuntime", "Runtime vs Mass vs dL", mass.size(), mi, mf, dLvec.size(), dLinitial, dLfinal);
        auto histProbability = std::make_unique<TH2D>("histProbability", "Probability vs Mass vs dL", mass.size(), mi, mf, dLvec.size(), dLinitial, dLfinal);

        for(const auto &dL : dLvec){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "dL: " << dL << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            // Runtime for filling the magentic values
            auto start_timeStandard = std::chrono::high_resolution_clock::now();
            std::vector<Double_t> magneticValues = magneticField->GetTransversalComponentAlongPath(initialPosition, finalPosition, dL);
            auto end_timeStandard = std::chrono::high_resolution_clock::now();
            auto durationStandard = std::chrono::duration_cast<std::chrono::microseconds>(end_timeStandard - start_timeStandard);

            for(const auto &ma : mass){
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Mass: " << ma << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }
                //Standard Integration
                auto start_time = std::chrono::high_resolution_clock::now();
                Double_t probField = axionField->GammaTransmissionProbability(magneticValues, dL, Ea, ma);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

                if(kDebug){
                    std::cout << "Integration using standard" << std::endl;
                    std::cout << "Probability: " << probField << std::endl;
                    std::cout << "Runtime (μs): " << duration.count() << std::endl;
                    std::cout << std::endl;
                }

                // Fill the histograms
                histRuntime->Fill(ma, dL, duration.count() + durationStandard.count());
                histProbability->Fill(ma, dL, probField);
            }
        }

        // Draw histograms on canvases
        canvasRuntime->cd();
        histRuntime->SetStats(0);
        histRuntime->GetXaxis()->SetLabelSize(0.03);
        histRuntime->GetXaxis()->SetLabelFont(22); 
        histRuntime->GetXaxis()->SetTitleSize(0.03); 
        histRuntime->GetXaxis()->SetTitleFont(22);  
        histRuntime->GetYaxis()->SetLabelSize(0.03);
        histRuntime->GetYaxis()->SetLabelFont(22);
        histRuntime->GetYaxis()->SetTitleSize(0.03);
        histRuntime->GetYaxis()->SetTitleFont(22); 
        histRuntime->GetZaxis()->SetLabelSize(0.03);
        histRuntime->Draw("COLZ");

        canvasProbability->cd();
        histProbability->SetStats(0);
        histProbability->GetXaxis()->SetLabelSize(0.03);
        histProbability->GetXaxis()->SetLabelFont(22); 
        histProbability->GetXaxis()->SetTitleSize(0.03);
        histProbability->GetXaxis()->SetTitleFont(22); 
        histProbability->GetYaxis()->SetLabelSize(0.03); 
        histProbability->GetYaxis()->SetLabelFont(22);
        histProbability->GetYaxis()->SetTitleSize(0.03);
        histProbability->GetYaxis()->SetTitleFont(22); 
        histProbability->GetZaxis()->SetLabelSize(0.02);
        histProbability->Draw("COLZ");

        // Save the plots if required
        if (kSave) {
            std::string folder = "IntegralAnalysis/";
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }

            std::string fileNameRuntime = fieldName + "_RuntimeStandard.png";
            std::string fileNameProbability = fieldName + "_ProbabilityStandard.png";
            canvasRuntime->SaveAs((folder + fileNameRuntime).c_str());
            canvasProbability->SaveAs((folder + fileNameProbability).c_str());
        }
    }

    return 0;
}