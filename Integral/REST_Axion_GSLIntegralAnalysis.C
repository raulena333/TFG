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
#include <TH3D.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** This function performs an integral analysis using GSL integration for a given set of parameters.
//*** It calculates the runtime, probability, and error for each combination of parameters and stores
//*** the results in 2D histograms. It also optionally saves the histograms as images.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 20).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the buffer gas (default: "He").
//*** - m: Axion mass (default: 0.01).
//*** - num_intervals_max: Maximum number of intervals for GSL integration (default: 500).
//*** - num_intervals_min: Minimum number of intervals for GSL integration (default: 50).
//*** - qawo_levels_max: Maximum number of QAWO levels for GSL integration (default: 150).
//*** - qawo_levels_min: Minimum number of QAWO levels for GSL integration (default: 10).

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
constexpr int kNumBins = 100;

Int_t REST_Axion_GSLIntegralAnalysis(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", Double_t m = 0.01,
                                     Int_t num_intervals_max = 500, Int_t num_intervals_min = 50,
                                     Int_t qawo_levels_max = 150, Int_t qawo_levels_min = 10) {
    auto start_time_final = std::chrono::high_resolution_clock::now();

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", //"babyIAXO_2024"
    };
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-5, 5, -11000);
    TVector3 direction = (position - TVector3(5, -5, 11000)).Unit();

    std::vector<Int_t> num_intervals;
    std::vector<Int_t> qawo_levels;

    for (Int_t j = 0; j < nData; j++) {
        Int_t intervals = num_intervals_min + j * (num_intervals_max - num_intervals_min) / nData;
        num_intervals.push_back(intervals);
        Int_t level = qawo_levels_min + j * (qawo_levels_max - qawo_levels_min) / nData;
        qawo_levels.push_back(level);
    }

    std::vector<Double_t> mass;
    // Create instance of TRestAxionBufferGas and fill the mass vector depending on gas pointer
    std::unique_ptr<TRestAxionBufferGas> gasPtr;
    if (!gasName.empty()) {
        gasPtr = std::make_unique<TRestAxionBufferGas>();
        gasPtr->SetGasDensity(gasName, gasDensity);
        mass.push_back(gasPtr->GetPhotonMass(Ea));
    } else {
        mass.push_back(0);
    }
    mass.push_back(m);

    for(const auto &fieldName : fieldNames){
        
        // Create instance of magnetic field, axion field and buffer gas and then assign it.
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        if (!gasName.empty())
            axionField->AssignBufferGas(gasPtr.get());
        axionField->AssignMagneticField(magneticField.get());
        magneticField->SetTrack(position, direction);

        for(const auto &ma : mass){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << " Mass : " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            // Create TCanvas for plotting
            auto canvasRuntime = std::make_unique<TCanvas>((fieldName + "_Runtime").c_str(), (fieldName + "_Runtime").c_str(), 850, 673);
            auto canvasProbability = std::make_unique<TCanvas>((fieldName + "_Probability").c_str(), (fieldName + "_Probability").c_str(), 850, 673);
            auto canvasError = std::make_unique<TCanvas>((fieldName + "_Error").c_str(), (fieldName + "_Error").c_str(), 850, 673);

            // Create 2D histograms
            auto histRuntime = std::make_unique<TH2D>("histRuntime", "Runtime vs Num_intervals vs Qawo_levels",
                                                       kNumBins, num_intervals_min, num_intervals_max,
                                                       kNumBins, qawo_levels_min, qawo_levels_max);
            auto histProbability = std::make_unique<TH2D>("histProbability", "Probability vs  Num_intervals vs Qawo_levels",
                                                           kNumBins, num_intervals_min, num_intervals_max,
                                                           kNumBins, qawo_levels_min, qawo_levels_max);
            auto histError = std::make_unique<TH2D>("histError", "Error vs  Num_intervals vs Qawo_levels",
                                                     kNumBins, num_intervals_min, num_intervals_max,
                                                     kNumBins, qawo_levels_min, qawo_levels_max);

            for(const auto &num_interval : num_intervals){
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << ": Number of intervals " << num_interval << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }
                for(const auto &qawo_level : qawo_levels){
                    if(kDebug){
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << "Number of qawo levels: " << qawo_level << std::endl;
                        std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        std::cout << std::endl;
                    }

                    // GSL Integration
                    auto start_time = std::chrono::high_resolution_clock::now();
                    std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, num_interval, qawo_level);
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                    if(kDebug){
                        std::cout << "Probability: " << probField.first << std::endl;
                        std::cout << "Error: " << probField.second << std::endl;
                        std::cout << "Runtime (ms): " << duration.count() << std::endl;
                        std::cout << std::endl;
                    }

                    // Fill the histograms
                    histRuntime->Fill(num_interval, qawo_level, duration.count());
                    histProbability->Fill(num_interval, qawo_level, probField.first);
                    histError->Fill(num_interval, qawo_level, probField.second);
                }
            }

            // Draw histograms on canvases
            canvasRuntime->cd();
            histRuntime->SetStats(0);
            histRuntime->GetXaxis()->SetTitle("Number of intervals");
            histRuntime->GetYaxis()->SetTitle("Qawo levels");
            histRuntime->GetZaxis()->SetTitle("Runtime (ms)");
            histRuntime->Draw("COLZ");

            canvasProbability->cd();
            histProbability->SetStats(0);
            histProbability->GetXaxis()->SetTitle("Number of intervals");
            histProbability->GetYaxis()->SetTitle("Qawo levels");
            histProbability->GetZaxis()->SetTitle("Probability");
            histProbability->Draw("COLZ");

            canvasError->cd();
            histError->SetStats(0);
            histError->GetXaxis()->SetTitle("Number of intervals");
            histError->GetYaxis()->SetTitle("Qawo levels");
            histError->GetZaxis()->SetTitle("Error");
            histError->Draw("COLZ");


            // Save the plots if required
            if (kSave) {
                std::string folder = "IntegralAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameRuntime = fieldName + std::to_string(ma) + "_RuntimeGSL.png";
                std::string fileNameProbability = fieldName +  std::to_string(ma) + "_ProbabilityGSL.png";
                std::string fileNameError = fieldName +  std::to_string(ma) + "_ErrorGSL.png"; 
                canvasRuntime->SaveAs((folder + fileNameRuntime).c_str());
                canvasProbability->SaveAs((folder + fileNameProbability).c_str());
                canvasError->SaveAs((folder + fileNameError).c_str());
            } 
        }
    }

    auto end_time_final = std::chrono::high_resolution_clock::now();
    auto duration_final = std::chrono::duration_cast<std::chrono::seconds>(end_time_final - start_time_final);

    // Open an output file stream
    std::ofstream outputFile("DurationOutput_GSL.txt");

    if (outputFile.is_open()) {
        outputFile << "Final duration: " << duration_final.count() << " seconds" << std::endl;

        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open the output file." << std::endl;
    }

    return 0;
}