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
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalCOmponentAlongPath`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability`, and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct TypeIntegration {
    std::vector<double> probability;        
    std::vector<double> error;
    std::vector<double> timeComputation;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_IntegralAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.3, 
            Double_t mf = 0.4){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-5, 5, -9000);
    TVector3 fPosition(5, -5 , 9000);
    TVector3 direction = (position - fPosition).Unit();
    std::vector<Double_t> mass;

    for(Int_t j = 0; j < nData; j++) {
        mass.push_back(mi + j * (mf - mi) / nData);
    }

    for(const auto &fieldName : fieldNames){
        // CHANGE ACCURACY OF 2ND MAP
        Double_t accuracy;
        if(fieldName == "babyIAXO_2024_cutoff")
            accuracy = 0.1;
        else
            accuracy = 0.25;

        std::map<std::string, TypeIntegration> integrations = {
            {"Standard-Integral", {}},
            {"GSL-Integral", {}}
        };
        // Create magnetic field
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);

        //Assign bufferGas and magneticField
        magneticField->SetTrack(position, direction);

        auto start_timeM = std::chrono::high_resolution_clock::now();
        std::vector<Double_t> magneticValues = magneticField->GetTransversalComponentAlongPath(position, fPosition, dL);
        auto end_timeM = std::chrono::high_resolution_clock::now();
        auto durationM = std::chrono::duration_cast<std::chrono::microseconds>(end_timeM - start_timeM);

        auto ax = std::make_unique<TRestAxionField>();
        std::unique_ptr<TRestAxionBufferGas> gas;
        if (!gasName.empty()) {
            gas = std::make_unique<TRestAxionBufferGas>();
            gas->SetGasDensity(gasName, gasDensity);
            ax->AssignBufferGas(gas.get());
        }
        ax->AssignMagneticField(magneticField.get());

        for(const auto &ma : mass){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            //GSL Integration
            auto start_timeGSL = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probFieldGSL = ax->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
            auto end_timeGSL = std::chrono::high_resolution_clock::now();
            auto durationGSL = std::chrono::duration_cast<std::chrono::microseconds>(end_timeGSL - start_timeGSL);

            integrations["GSL-Integral"].probability.push_back(probFieldGSL.first);
            integrations["GSL-Integral"].error.push_back(probFieldGSL.second);
            integrations["GSL-Integral"].timeComputation.push_back(durationGSL.count());

            if(kDebug){
                std::cout << "Integration using GSL" << std::endl;
                std::cout << "Probability: " << probFieldGSL.first << std::endl;
                std::cout << "Error: " << probFieldGSL.second << std::endl;
                std::cout << "Runtime (μs): " << durationGSL.count() << std::endl;
                std::cout << std::endl;
            }

            //Standard Integration
            auto start_timeStandard = std::chrono::high_resolution_clock::now();
            Double_t probFieldStandard = ax->GammaTransmissionProbability(magneticValues, dL, Ea, ma);
            auto end_timeStandard = std::chrono::high_resolution_clock::now();
            auto durationStandard = std::chrono::duration_cast<std::chrono::microseconds>(end_timeStandard - start_timeStandard);

            integrations["Standard-Integral"].probability.push_back(probFieldStandard);
            integrations["Standard-Integral"].error.push_back(0);
            integrations["Standard-Integral"].timeComputation.push_back(durationStandard.count() + durationM.count());

            if(kDebug){
                std::cout << "Integration using standard" << std::endl;
                std::cout << "Probability: " << probFieldStandard << std::endl;
                std::cout << "Error: " << 0 << std::endl; // No error in standard integration
                std::cout << "Runtime (μs): " << durationStandard.count() << std::endl;
                std::cout << std::endl;
            }
        }

        if(kPlot){
             // Create TCanvas for plotting probability vs. mass
            TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbabilityBoth").c_str(), (fieldName + "_MassProb").c_str(), 850, 673);
            canvasProb->cd();

            Int_t colorIndex = 1;
            TLegend *legendProb = new TLegend(0.1, 0.8, 0.3, 0.9);
            std::vector<TGraph*> graphsProb;
            for (const auto &integration : integrations) {
                TGraph *graph = new TGraphErrors(mass.size(), mass.data(), integration.second.probability.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendProb->AddEntry(graph, integration.first.c_str(), "l");
                colorIndex++;

                graphsProb.push_back(graph);
            }

            graphsProb[0]->SetTitle("Axion Mass vs Probability");
            graphsProb[0]->GetYaxis()->SetTitle("Probability");
            graphsProb[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphsProb[0]->GetXaxis()->SetTitleSize(0.03);
            graphsProb[0]->GetYaxis()->SetTitleSize(0.03);
            graphsProb[0]->GetXaxis()->SetLabelSize(0.03);
            graphsProb[0]->GetYaxis()->SetLabelSize(0.03);
            legendProb->Draw();

            // Set logarithmic scale if required
            if (useLogScale)
                canvasProb->SetLogy();

            // Create the canvas to plot the runTime of each gas track
            TCanvas *canvasRun = new TCanvas((fieldName + "_MassRunTime").c_str(), (fieldName + "_MassRun").c_str(), 850, 673);
            canvasRun->cd();

            colorIndex = 1;
            TLegend *legendRun = new TLegend(0.1, 0.8, 0.3, 0.9);
            std::vector<TGraph*> graphsRun;

            for (const auto &integration : integrations) {
                TGraph *graph = new TGraph(mass.size(), mass.data(), integration.second.timeComputation.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendRun->AddEntry(graph, integration.first.c_str(), "l");
                colorIndex++;

                graphsRun.push_back(graph);
            }

            graphsRun[0]->SetTitle("Axion Mass vs RunTime");
            graphsRun[0]->GetYaxis()->SetTitle("RunTime (μs)");
            graphsRun[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
            graphsRun[0]->GetXaxis()->SetTitleSize(0.03);
            graphsRun[0]->GetYaxis()->SetTitleSize(0.03);
            graphsRun[0]->GetXaxis()->SetLabelSize(0.03);
            graphsRun[0]->GetYaxis()->SetLabelSize(0.03);
            legendRun->Draw();

            // Save the plots if required
            if (kSave) {
                std::string folder = "IntegralAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameProb = fieldName + "_ProbabilityIntegral.png";
                std::string fileNameRun = fieldName + "_RunTimeIntegral.png";
                canvasProb->SaveAs((folder + fileNameProb).c_str());
                canvasRun->SaveAs((folder + fileNameRun).c_str());
            }

        delete canvasProb;
        delete canvasRun;
        delete legendProb;
        delete legendRun;

        }
    }
    return 0;
}
