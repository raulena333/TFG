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

Int_t REST_Axion_StandardIntegralAnalysis(Int_t nData = 1, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.3, 
            Double_t mf = 0.4, Int_t dLinitial = 1, Int_t dLfinal = 500){
    
    auto start_time_final = std::chrono::high_resolution_clock::now();
    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 initialPosition(-5, 5, -11000);
    TVector3 finalPosition(5,-5, 11000);
    std::vector<Double_t> mass;
    std::vector<Int_t> dLvec;

    std::vector<Int_t> dLvalues = {1, 500};

    for (Int_t j = 0; j < nData; j++) {
        mass.push_back(mi + j * (mf - mi) / nData);
        Int_t dL = dLinitial + j * (dLfinal - dLinitial) / nData;
        dLvec.push_back(dL);
    }

    // Check if 1 and 500 are present in dLvec, if not, add them
    if (std::find(dLvec.begin(), dLvec.end(), 1) == dLvec.end()) {
        dLvec.push_back(1);
    }
    if (std::find(dLvec.begin(), dLvec.end(), 500) == dLvec.end()) {
        dLvec.push_back(500);
    }
    std::sort(dLvec.begin(), dLvec.end());

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
        auto histRuntime = std::make_unique<TH2D>("histRuntime", "Runtime vs Mass vs dL", mass.size(), mi, mf, dLvec.size(), dLinitial, dLfinal);
        auto histProbability = std::make_unique<TH2D>("histProbability", "Probability vs Mass vs dL", mass.size(), mi, mf, dLvec.size(), dLinitial, dLfinal);

        std::vector<TGraph*> graphProb;
        std::vector<TGraph*> graphRun;
        for(const auto &dL : dLvec){
        //for(const auto &dL : {500}){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "dL: " << dL << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }

            std::vector<Double_t> probabilityValues, runTimeValues;

            // Runtime for filling the magnetic values
            auto start_timeStandard = std::chrono::high_resolution_clock::now();
            std::vector<Double_t> magneticValues = magneticField->GetTransversalComponentAlongPath(initialPosition, finalPosition, dL);
            for (const auto &value : magneticValues)
                std::cout << value << std::endl;
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

                if (dL == dLvalues[0] || dL == dLvalues[1]) {
                    probabilityValues.push_back(probField);
                    runTimeValues.push_back(duration.count() + durationStandard.count());

                    if(kDebug){
                        // Debug output to check if values are saved correctly
                        std::cout << "Values saved for dL = " << dL << std::endl;
                        std::cout << "Probability Values: ";
                        for (const auto& value : probabilityValues) {
                            std::cout << value << " ";
                        }
                        std::cout << std::endl;

                        std::cout << "Runtime Values: ";
                        for (const auto& value : runTimeValues) {
                            std::cout << value << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }

            if (dL == dLvalues[0] || dL == dLvalues[1]){
                // Create TGraphs and save them in vectors
                TGraph* graphProbField = new TGraph(probabilityValues.size(), &mass[0], &probabilityValues[0]);
                TGraph* graphRunTime = new TGraph(runTimeValues.size(), &mass[0], &runTimeValues[0]);

                graphProb.push_back(graphProbField);
                graphRun.push_back(graphRunTime);
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

        if (kPlot) {
            // Create a canvas to hold the probability graphs
            TCanvas* canvasProb = new TCanvas((fieldName + "Probability").c_str(), (fieldName + " Probability Values").c_str(), 800, 600);
            TLegend* legendProb = new TLegend(0.1, 0.8, 0.3, 0.9);

            // Loop over each set of probability graphs
            for (size_t i = 0; i < graphProb.size(); ++i) {
                graphProb[i]->SetLineColor(i + 1);
                graphProb[i]->SetLineWidth(1);
                if (i == 0) {
                    graphProb[i]->Draw("ACP"); 
                    legendProb->AddEntry(graphProb[i], "dL = 1", "l");
                } else {
                    graphProb[i]->Draw("ACP SAME"); 
                    legendProb->AddEntry(graphProb[i], "dL = 10", "l");
                }
               
            }

            graphProb[0]->SetTitle("Axion Mass vs Probability");
            graphProb[0]->GetYaxis()->SetTitle("Probability");
            graphProb[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphProb[0]->GetXaxis()->SetTitleSize(0.03);
            graphProb[0]->GetYaxis()->SetTitleSize(0.03);
            graphProb[0]->GetXaxis()->SetLabelSize(0.03);
            graphProb[0]->GetYaxis()->SetLabelSize(0.03);

            legendProb->Draw();
            canvasProb->Update();
            canvasProb->Draw();

            // Create a canvas to hold the runtime graphs
            TCanvas* canvasRun = new TCanvas((fieldName + "Runtime").c_str(), (fieldName + " Runtime Values").c_str(), 800, 600);
            TLegend* legendRun = new TLegend(0.1, 0.8, 0.3, 0.9);

            // Loop over each set of runtime graphs
            for (size_t i = 0; i < graphRun.size(); ++i) {
                graphRun[i]->SetLineColor(i + 1);
                graphRun[i]->SetLineWidth(1);
                if (i == 0) {
                    graphRun[i]->Draw("ACP"); 
                    legendRun->AddEntry(graphRun[i], "dL = 1", "l");
                } else {
                    graphRun[i]->Draw("ACP SAME");
                    legendRun->AddEntry(graphRun[i], "dL = 10", "l");
                }
            }

            graphRun[0]->SetTitle("Axion Mass vs RunTime");
            graphRun[0]->GetYaxis()->SetTitle("RunTime (μs)");
            graphRun[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphRun[0]->GetXaxis()->SetTitleSize(0.03);
            graphRun[0]->GetYaxis()->SetTitleSize(0.03);
            graphRun[0]->GetXaxis()->SetLabelSize(0.03);
            graphRun[0]->GetYaxis()->SetLabelSize(0.03);

            legendRun->Draw();
            canvasRun->Update();
            canvasRun->Draw();

            if (kSave) {
                canvasRun->SaveAs(("IntegralAnalysis/" + fieldName + "_RuntimePlot.png").c_str());
                canvasProb->SaveAs(("IntegralAnalysis/" + fieldName + "_ProbabilityPlot.png").c_str());
            }
        }
    }

    auto end_time_final = std::chrono::high_resolution_clock::now();
    auto duration_final = std::chrono::duration_cast<std::chrono::microseconds>(end_time_final - start_time_final);

    // Open an output file stream
    std::ofstream outputFile("duration_output.txt");

    if (outputFile.is_open()) {
        outputFile << "Final duration: " << duration_final.count() << " microseconds" << std::endl;

        outputFile.close();
    } else {
        std::cerr << "Error: Unable to open the output file." << std::endl;
    }

    return 0;
}
