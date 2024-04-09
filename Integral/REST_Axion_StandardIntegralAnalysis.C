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
#include <TGraph.h> // Add this include for TGraph
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
//*** - nData: Number of mass points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the buffer gas (default: "He").
//*** - mi: Initial axion mass in eV (default: 0.25).
//*** - mf: Final axion mass in eV (default: 0.45).
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
constexpr int kNumBins = 100;

Int_t REST_Axion_StandardIntegralAnalysis(Int_t nData = 200, Int_t nMass = 100, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.25, 
            Double_t mf = 0.45, Int_t dLinitial = 1, Int_t dLfinal = 1000){
    
    auto start_time_final = std::chrono::high_resolution_clock::now();
    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024" };
    Double_t gasDensity = 2.9836e-10;
    TVector3 initialPosition(-5, 5, -11000);
    TVector3 finalPosition(5,-5, 11000);
    std::vector<Double_t> mass;
    std::vector<Int_t> dLvec;

    std::vector<Int_t> dLvalues = {1, 10 ,100, 1000};

    for (Int_t j = 0; j < nData; j++) {
        Int_t dL = dLinitial + j * (dLfinal - dLinitial) / nData;
        dLvec.push_back(dL);
    }

    if (std::find(dLvec.begin(), dLvec.end(), 10) == dLvec.end()) {
        dLvec.push_back(10);
    }
    if (std::find(dLvec.begin(), dLvec.end(), 100) == dLvec.end()) {
        dLvec.push_back(100);
    }
    if (std::find(dLvec.begin(), dLvec.end(), 1000) == dLvec.end()) {
        dLvec.push_back(1000);
    }

    // Sort the dLvec vector
    std::sort(dLvec.begin(), dLvec.end());
    
    for (Int_t j = 0; j< nMass; j++){
        mass.push_back(mi + j * (mf - mi) / nMass);
    }

    for(const auto &fieldName : fieldNames){
        
        // Create instance of magnetic field, axion field and buffer gas and then assign it.
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
        auto histRuntime = std::make_unique<TH2D>("histRuntime", "Runtime vs Mass vs dL", kNumBins, mi, mf, kNumBins, dLinitial, dLfinal);
        auto histProbability = std::make_unique<TH2D>("histProbability", "Probability vs Mass vs dL", kNumBins, mi, mf, kNumBins, dLinitial, dLfinal);

        std::vector<TGraph*> graphProb;
        std::vector<std::vector<Double_t>> probabilityValues(dLvalues.size());

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

            for(const auto &ma : mass){
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Mass: " << ma << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }
                // Standard Integration
                auto start_time = std::chrono::high_resolution_clock::now();
                Double_t probField = axionField->GammaTransmissionProbability(magneticValues, dL, Ea, ma);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

                if(kDebug){
                    std::cout << "Probability: " << probField << std::endl;
                    std::cout << "Runtime (μs): " << duration.count() << std::endl;
                    std::cout << std::endl;
                }

                // Fill the histograms
                histRuntime->Fill(ma, dL, duration.count() + durationStandard.count());
                histProbability->Fill(ma, dL, probField);

                // Fill the probability values vector
                for(size_t i = 0; i < dLvalues.size(); i++) {
                    if(dL == dLvalues[i]) {
                        probabilityValues[i].push_back(probField);
                        break;
                    }
                }
            }
        }

        // Create TGraphs and save them in vectors
        for(const auto &probValues : probabilityValues) {
            auto graph = new TGraph(mass.size(), mass.data(), probValues.data());
            graphProb.push_back(graph);
        }

        // Draw histograms on canvases
        canvasRuntime->cd();
        histRuntime->SetStats(0);
        histRuntime->GetXaxis()->SetTitle("Axion Mass (eV)");
        histRuntime->GetYaxis()->SetTitle("dL (mm)");
        histRuntime->GetZaxis()->SetTitle("Runtime (#mu s)");
        histRuntime->Draw("COLZ");

        canvasProbability->cd();
        histProbability->SetStats(0);
        histProbability->GetXaxis()->SetTitle("Axion Mass (eV)");
        histProbability->GetYaxis()->SetTitle("dL (mm)");
        histProbability->GetZaxis()->SetTitle("Probability");
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

        if (kPlot) {
            // Create a canvas to hold the probability graphs
            TCanvas* canvasProb = new TCanvas((fieldName + "Probability").c_str(), (fieldName + " Probability Values").c_str(), 850, 680);
            canvasProb->Divide(2, 2); // Divide canvas into 2x2 grid for 4 plots

            // Loop over each set of probability graphs
            for (size_t i = 0; i < graphProb.size(); ++i) {
                canvasProb->cd(i + 1); // Select the i-th pad

                graphProb[i]->SetLineColor(i + 1);
                graphProb[i]->SetLineWidth(2);
                graphProb[i]->Draw("ACP");
                graphProb[i]->SetTitle(("Axion Mass vs Probability for dL = " + std::to_string(dLvalues[i])).c_str());
                graphProb[i]->GetYaxis()->SetTitle("Probability");
                graphProb[i]->GetXaxis()->SetTitle("Axion Mass (eV)");
                graphProb[i]->GetXaxis()->SetTitleSize(0.03);
                graphProb[i]->GetYaxis()->SetTitleSize(0.03);
                graphProb[i]->GetXaxis()->SetLabelSize(0.03);
                graphProb[i]->GetYaxis()->SetLabelSize(0.03);
            }

            canvasProb->Update();
            canvasProb->Draw();

            if (kSave) {
                std::string folder = "IntegralAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }
                canvasProb->SaveAs((folder + fieldName + "_ProbabilityPlotStandard.png").c_str());
            }
        }
    }

    auto end_time_final = std::chrono::high_resolution_clock::now();
    auto duration_final = std::chrono::duration_cast<std::chrono::seconds>(end_time_final - start_time_final);

    // Open an output file stream
    std::ofstream outputFile("DurationOutput_Standard.txt");

    if (outputFile.is_open()) {
        outputFile << "Final duration: " << duration_final.count() << " seconds" << std::endl;

        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open the output file." << std::endl;
    }

    return 0;
}