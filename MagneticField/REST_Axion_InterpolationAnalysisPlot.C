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
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"
#include <TLatex.h>

//*******************************************************************************************************
//*** Description: This script analyzes the MagneticField class to determine whether it is beneficial to enable or 
//*** disable trilinear interpolation when calculating the magnetic field at an arbitrary point. It plots the 
//*** probability transmission for each mass at a specified accuracy and also plots the runtime for each mass.
//*** 
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial axion mass in eV (default: 0.2).
//*** - mf: Final axion mass in eV (default: 0.5).
//*** - dL: Accuracy value for intrgration in Standard, depends on the axion mass (default 1.).
//*** - useLogSCale: set log scale in y-axis for plots (default: false)
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetInterpolation'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************


struct FieldTrack {
    bool interpolation;
    std::unique_ptr<TRestAxionMagneticField> magneticField;
    std::unique_ptr<TRestAxionField> axionField;

    std::vector<double> probability;
    double Time;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_InterpolationAnalysisPlot(Int_t nData = 250, Double_t Ea = 4.2, std::string gasName = "He", 
                    Double_t mi = 0.28, Double_t mf = 0.42, Int_t dL = 1, Bool_t useLogScale =  true){
    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff"};
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-5, 5, -11000);
    TVector3 fposition(5, -5 , 11000);
    TVector3 direction = (position - fposition).Unit();
    std::vector<Double_t> masses;


    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine mass values
    for(unsigned j = 0; j< nData; j++){
        masses.push_back(mi + j *(mf - mi)/ nData);
    }

    std::string folder = "InterpolationAnalysis/";
    for(const auto &fieldName : fieldNames){
        std::map<std::string, FieldTrack> fields;
        fields["Interpolacion"] = {true, std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName), std::make_unique<TRestAxionField>()};
        fields["No-Interpolacion"] = {false, std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName), std::make_unique<TRestAxionField>()};

        for(auto &field: fields){
            if (gas != nullptr) 
                field.second.axionField->AssignBufferGas(gas.get());

            field.second.axionField->AssignMagneticField(field.second.magneticField.get());
        }

        for(auto &field : fields){
            // Set interpolation on or off
            field.second.magneticField->SetInterpolation(field.second.interpolation);

            auto start_time = std::chrono::high_resolution_clock::now();
            std::vector<Double_t> magneticValues = field.second.magneticField->GetTransversalComponentAlongPath(position, fposition, dL);

            for(const auto &ma : masses){

                // GSL
                //std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
                // Standard
                Double_t probField = field.second.axionField->GammaTransmissionProbability(magneticValues, dL, Ea, ma);
                field.second.probability.push_back(probField);

                if(kDebug){
                    std::cout << "Mass: " << ma << std::endl;
                    std::cout << field.first << std::endl;
                    std::cout << "Probability: " << probField << std::endl;
                    std::cout << std::endl;
                }
            }    
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            field.second.Time = duration.count();
        }

        /// PLOT ///
        if(kPlot){
            // Create canvas to plot the probabilities against the mass
            TCanvas *canvasProb = new TCanvas((fieldName + "_MassProbability").c_str(), (fieldName + "_MassProb").c_str(), 800, 600);
            canvasProb->cd();

            // Pad for graphs
            TPad *padTop = new TPad("PadTop", "", 0.001 ,0.3 ,0.999, 0.999);
            padTop->SetTopMargin(0.10);
            padTop->SetLeftMargin(0.16);
            padTop->SetBottomMargin(0.0);
            padTop->SetRightMargin(0.05);
            padTop->SetBorderMode(-1);
            padTop->Draw();

            // Pad for residuals
            TPad *padBottom = new TPad("PadBottom", "", 0.001 ,0.001 ,0.999, 0.3);
            padBottom->SetTopMargin(0.0);
            padBottom->SetLeftMargin(0.16);
            padBottom->SetBottomMargin(0.45);
            padBottom->SetRightMargin(0.05);
            padBottom->SetBorderMode(-1);
            padBottom->Draw();

            // Calculate residuals between  setting it off and on
            std::vector<Double_t> Residuals;
            for (size_t i = 0; i < nData; ++i) {
                Double_t residual = std::abs(fields["Interpolacion"].probability[i] - fields["No-Interpolacion"].probability[i]) / fields["Interpolacion"].probability[i] * 100.0 ;
                Residuals.push_back(residual);
            }

            TGraph *graphInter = new TGraph(masses.size(), masses.data(), Residuals.data());

            // Cambiar al pad inferior
            padBottom->cd();
            padBottom->Update();

            // Configurar y dibujar el gráfico en el pad inferior
            graphInter->SetMarkerStyle(8);
            graphInter->SetMarkerSize(0.4);
            graphInter->SetMarkerColor(kBlack);
            graphInter->SetTitle("");
            graphInter->GetXaxis()->SetRange(mi, mf);
            graphInter->GetYaxis()->SetRangeUser(0, 100);
            graphInter->GetXaxis()->SetTitle("Masa Axion (eV)");
            graphInter->GetYaxis()->SetTitle("Residuos (%)");
            graphInter->GetXaxis()->SetTitleSize(0.16);
            graphInter->GetXaxis()->SetLabelSize(0.16);
            graphInter->GetYaxis()->SetTitleSize(0.12);
            graphInter->GetYaxis()->SetLabelSize(0.11);
            graphInter->GetYaxis()->SetTitleFont(40);
            graphInter->GetYaxis()->SetTitleOffset(0.6);
            graphInter->GetXaxis()->SetTitleFont(40);
            graphInter->GetYaxis()->SetLabelFont(40);
            graphInter->GetXaxis()->SetLabelFont(40);
            graphInter->GetYaxis()->SetNdivisions(505);
            graphInter->Draw("AP");
            
            if(useLogScale)
                padBottom->SetLogy();

            padTop->cd();
            padTop->Update();
            Int_t colorIndex = 1;
            TLegend *legendProb = new TLegend(0.67, 0.75, 0.95, 0.9);
            //std::vector<TGraphErrors*> graphsProb;
            std::vector<TGraph*> graphsProb;

            for (const auto &field : fields) {
                // TGraphErrors *graph = new TGraphErrors(masses.size(), masses.data(), field.second.probability.data(), nullptr, field.second.error.data());
                TGraph *graph = new TGraph(masses.size(), masses.data(), field.second.probability.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                graph->SetTitle("");
                graph->GetYaxis()->SetTitle("Probabilidad");
                //graph->GetXaxis()->SetTitle("Masa Axion (eV)");
                graph->GetXaxis()->SetRange(mi, mf);
                graph->GetYaxis()->SetRangeUser(5e-30, 15e-18);
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
                legendProb->AddEntry(graph, field.first.c_str(), "l");
                colorIndex++;

                graphsProb.push_back(graph);
            }

            legendProb->SetTextSize(0.045);
            legendProb->Draw();
            // Set logarithmic scale if required
            if (useLogScale)
                padTop->SetLogy();

            if constexpr (kSave) {
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }

                std::string fileNameProb = fieldName + "_SProbabilityInterpolation.pdf";
                canvasProb->SaveAs((folder + fileNameProb).c_str());
            }

            delete canvasProb;
            delete legendProb;
        }

        // Save times for both 
        std::string fileName = fieldName + "_SProbabilityInterpolationRun.txt";
        std::ofstream outputFile((folder + fileName).c_str());
        if (!outputFile.is_open()) {
            std::cerr << "Error: Unable to open the file for writing!" << std::endl;
            return 1;
        }  

        outputFile << "Time in seconds" << std::endl;
        for(const auto &field: fields)
            outputFile << field.first << ": " << field.second.Time << std::endl;
        outputFile.close();
    }
    return 0;
}
