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
//*** This function performs integral analysis for axion field simulations. It calculates the probability
//*** of axion-gamma transmission and the corresponding error, as well as the computation time for each
//*** integration method. The analysis is performed for a range of axion masses.
//*** 
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - mi: Initial axion mass in eV (default: 0.2).
//*** - mf: Final axion mass in eV (default: 0.5).
//*** - useLogScale: Bool to set the y-axis to log scale for plotting (default: false).
//*** - dL: Length of the integration step in mm (default: 10).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
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

Int_t REST_Axion_IntegralAnalysisPlot(Int_t nData = 150, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.28, 
            Double_t mf = 0.42, Bool_t useLogScale = true, Double_t dL = 10){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    const Double_t gasDensity = 2.9836e-10;
    const TVector3 position(-5, 5, -11000);
    const TVector3 fPosition(5, -5 , 11000);
    const TVector3 direction = (position - fPosition).Unit();
    std::vector<Double_t> mass;

    for(Int_t j = 0; j < nData; j++) {
        mass.push_back(mi + j * (mf - mi) / nData);
    }

    for(const auto &fieldName : fieldNames){
        std::map<std::string, TypeIntegration> integrations = {
            {"Integral-Estandar", {}},
            {"Integral-GSL", {}}
        };

        Double_t accuracy;
        if (fieldName == "babyIAXO_2024_cutoff")
            accuracy = 0.25;
        else
            accuracy = 0.3;
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

            integrations["Integral-GSL"].probability.push_back(probFieldGSL.first);
            integrations["Integral-GSL"].error.push_back(probFieldGSL.second);
            integrations["Integral-GSL"].timeComputation.push_back(durationGSL.count());

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

            integrations["Integral-Estandar"].probability.push_back(probFieldStandard);
            integrations["Integral-Estandar"].error.push_back(0);
            integrations["Integral-Estandar"].timeComputation.push_back(durationStandard.count() + durationM.count());

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
            TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbabilityBoth").c_str(), (fieldName + "_MassProb").c_str(), 600, 500);
            canvasProb->cd();

            // Pad for graphs
            TPad *padTop = new TPad("PadTop", "", 0.0 ,0.3 ,1.0, 1.0);
            padTop->SetTopMargin(0.10);
            padTop->SetLeftMargin(0.165);
            padTop->SetBottomMargin(0.0);
            padTop->SetRightMargin(0.05);
            padTop->SetBorderMode(-1);
            padTop->Draw();
            
            // Pad for residuals
            TPad *padBottom = new TPad("PadBottom", "", 0.0 ,0.0 ,1.0, 0.3);
            padBottom->SetTopMargin(0.0);
            padBottom->SetLeftMargin(0.165);
            padBottom->SetBottomMargin(0.40);
            padBottom->SetRightMargin(0.05);
            padBottom->SetBorderMode(-1);
            padBottom->Draw();

            padTop->cd();
            Int_t colorIndex = 1;
            TLegend *legendProb = new TLegend(0.65, 0.7, 0.95, 0.9);
            std::vector<TGraph*> graphsProb;
            for (const auto &integration : integrations) {
                TGraph *graph = new TGraphErrors(mass.size(), mass.data(), integration.second.probability.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                graph->SetTitle("");
                graph->GetYaxis()->SetTitle("Probabilidad");
                //graph->GetXaxis()->SetTitle("Masa Axion (eV)");
                graph->GetXaxis()->SetRange(mi, mf);
                if (fieldName == "babyIAXO_2024_cutoff")
                    graph->GetYaxis()->SetRangeUser(15e-30, 1e-18);
                else
                    graph->GetYaxis()->SetRangeUser(15e-31, 1e-18);
                graph->GetXaxis()->SetTitleSize(0.07); 
                graph->GetXaxis()->SetTitleFont(40);  
                graph->GetXaxis()->SetLabelSize(0.07); 
                graph->GetXaxis()->SetLabelFont(40);  
                graph->GetYaxis()->SetTitleSize(0.07); 
                graph->GetYaxis()->SetTitleFont(40);  
                graph->GetYaxis()->SetLabelSize(0.07); 
                graph->GetYaxis()->SetLabelFont(40);
                graph->GetYaxis()->SetNdivisions(505);
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendProb->AddEntry(graph, integration.first.c_str(), "l");
                colorIndex++;

                graphsProb.push_back(graph);
            }

            legendProb->SetTextSize(0.0535);
            legendProb->Draw();
            // Set logarithmic scale if required
            if (useLogScale)
                padTop->SetLogy();

            // Create the pad to plot the residuals of probability between Standard and GSL integrals
            colorIndex = 1;
            std::vector<Double_t> residuals;

            for (size_t j = 0; j < mass.size(); ++j) {
                Double_t resi = std::abs(integrations["Integral-Estandar"].probability[j] - integrations["Integral-GSL"].probability[j]) / integrations["Integral-Estandar"].probability[j] * 100.0;
                residuals.push_back(resi);
            }
            padBottom->cd();

            TGraph *graphInter = nullptr;
            graphInter = new TGraph(nData, mass.data(), residuals.data());
            graphInter->SetMarkerStyle(8);
            graphInter->SetMarkerSize(0.4);
            graphInter->SetMarkerColor(kBlack);
            graphInter->SetTitle("");
            graphInter->GetXaxis()->SetTitle("Masa Axion (eV)");
            graphInter->GetYaxis()->SetTitle("Residuos (%)");
            graphInter->GetXaxis()->SetTitleSize(0.16);
            graphInter->GetXaxis()->SetLabelSize(0.16);
            graphInter->GetYaxis()->SetTitleSize(0.12);
            graphInter->GetYaxis()->SetLabelSize(0.11);
            graphInter->GetYaxis()->SetTitleFont(40);
            graphInter->GetYaxis()->SetTitleOffset(0.65);
            graphInter->GetXaxis()->SetTitleFont(40);
            graphInter->GetYaxis()->SetLabelFont(40);
            graphInter->GetXaxis()->SetLabelFont(40);
            graphInter->GetYaxis()->SetNdivisions(505);
            graphInter->Draw("AP");

            if (useLogScale)
                padBottom->SetLogy();


            // Create the canvas to plot the runTime of each integral
            TCanvas *canvasRun = new TCanvas((fieldName + "_MassRunTime").c_str(), (fieldName + "_MassRun").c_str(), 600, 500);
            canvasRun->cd();

            gPad->SetLeftMargin(0.138);
            gPad->SetBottomMargin(0.15);
            colorIndex = 1;
            TLegend *legendRun = new TLegend(0.58, 0.75, 0.9, 0.9);
            std::vector<TGraph*> graphsRun;

            for (const auto &integration : integrations) {
                std::vector<Double_t> runtime_seconds;
                for (const auto &runtime_micros : integration.second.timeComputation) {
                    runtime_seconds.push_back(runtime_micros * 1.0e-6); // Convert microseconds to seconds
                }
                TGraph *graph = new TGraph(mass.size(), mass.data(), runtime_seconds.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                graph->SetTitle("");
                graph->GetYaxis()->SetTitle("Tiempo Computacional (s)");
                graph->GetXaxis()->SetTitle("Masa Axion (eV)");
                graph->GetXaxis()->SetLimits(mi, mf); // Set x-axis limits
                graph->GetXaxis()->SetTitleSize(0.055); 
                graph->GetXaxis()->SetTitleFont(40);  
                graph->GetXaxis()->SetLabelSize(0.055); 
                if (fieldName == "babyIAXO_2024_cutoff")
                    graph->GetYaxis()->SetRangeUser(0, 120);
                else
                    graph->GetYaxis()->SetRangeUser(0, 160);
                graph->GetXaxis()->SetLabelFont(40);  
                graph->GetYaxis()->SetTitleSize(0.055); 
                graph->GetYaxis()->SetTitleFont(40);  
                graph->GetYaxis()->SetLabelSize(0.055); 
                graph->GetYaxis()->SetLabelFont(40); 
                if (colorIndex == 1) {
                    graph->Draw("ACP");
                } else {
                    graph->Draw("Same");
                }
                legendRun->AddEntry(graph, integration.first.c_str(), "l");
                colorIndex++;

                graphsRun.push_back(graph);
            }

            // // Crear un gráfico para la línea roja en babyIAXO_2024_cutoff
            // TGraph *redLineGraph = new TGraph(2);
            // if (fieldName == "babyIAXO_2024_cutoff"){
            //     redLineGraph->SetPoint(0, mi, 33); // Establecer el punto inicial
            //     redLineGraph->SetPoint(1, mf, 33); // Establecer el punto final
            // }
            // else{
            //     redLineGraph->SetPoint(0, mi, 92); // Establecer el punto inicial
            //     redLineGraph->SetPoint(1, mf, 92); // Establecer el punto final    
            // }
            // redLineGraph->SetLineColor(kRed); // Establecer el color de la línea roja

            // // Dibujar la línea roja en el mismo gráfico que el gráfico "babyIAXO_2024_cutoff"
            // redLineGraph->Draw("L SAME");


            legendRun->SetTextSize(0.041);
            legendRun->Draw();

            // Save the plots if required
            if (kSave) {
                std::string folder = "IntegralAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameProb = fieldName + "_ProbabilityIntegral.pdf";
                std::string fileNameRun = fieldName + "_RunTimeIntegral.pdf";
                canvasProb->SaveAs((folder + fileNameProb).c_str());
                canvasRun->SaveAs((folder + fileNameRun).c_str());
            }

            delete canvasProb;
            delete canvasRun;
            delete legendProb;
            delete legendRun;
        }

        std::string filename;
        std::string folder = "IntegralAnalysis/";
        std::ostringstream ossAccuracy;
        ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
        filename = folder + "REST_AXION_" + fieldName + "_IntegralAnalysis_Accuracy_" + ossAccuracy.str() + ".txt";   

        std::ofstream outputFile(filename);
        if (!outputFile.is_open()) {
            std::cerr << "Error: Unable to open the file for writing!" << std::endl;
            return 1;
        }

        outputFile << "Mass" << "\t" << "ProbabilidadStandard" << "\t" << "TiempoStandard (μs)" << "\t" << "ProbabilidadGSL" << "\t" << "ErrorGSL" << "TiempoGSL (μs)" << std::endl;
        for(size_t i = 0; i<nData; i++){
            outputFile << mass[i] << "\t" << integrations["Integral-Estandar"].probability[i] << "\t" << integrations["Integral-Estandar"].timeComputation[i] << "\t" 
            << integrations["Integral-GSL"].probability[i] << "\t" << integrations["Integral-GSL"].error[i] << "\t" << integrations["Integral-GSL"].timeComputation[i] <<  std::endl;
        }
        outputFile.close();

    }
    return 0;
}
