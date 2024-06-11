#include <iostream>
#include <vector>
#include <filesystem>

#include <TCanvas.h>
#include <TGraph.h>
#include <TLegend.h>
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description: function designed to analyze and plot the probability of gamma transmission versus 
//*** the axion mass under varying coherence lengths of the magnetic field.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points (axionMass) to generate (default: 200).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial Axion mass in eV (default: 0.2).
//*** - mf: Final Axion mass in eV (default: 0.5).
//*** - B: The magnetic field strength in Tesla (default: 2.0)
//*** - useLogScale: Whether to use log scale for y-axis (default: true)
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionField::GammaTransmissionProbability`. 
//***
//*** Author: Raul Ena
//*******************************************************************************************************

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_AnalysisMagenticFieldCoherencePlot(Int_t nData = 200, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.2, 
                                            Double_t mf = 0.5, Double_t B = 2., Bool_t useLogScale = true) {
    // Create Variables
    const Double_t gasDensity = 2.9836e-10;
    std::vector<Double_t> axionMass(nData);
    for(int j = 0; j < nData; j++)
        axionMass[j] = mi + j * (mf - mi) / nData;

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Create an instance of TRestAxionField and gas (if provided)
    TRestAxionField axionField;
    if (gas != nullptr) {
        axionField.AssignBufferGas(gas.get());
    }

    std::vector<TGraph*> graphs;
    std::vector<Int_t> coherenceLengths = {100, 500, 1000, 5000, 10000};
    //std::vector<Int_t> coherenceLengths = {1000, 5000, 10000, 50000};
    for(const auto& Lcoh : coherenceLengths){
        if (kDebug) {
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << "Coherence length " << Lcoh << std::endl;
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << std::endl;
        }
        std::vector<Double_t> probabilities;
        for(const auto& ma : axionMass){
            Double_t probability = axionField.GammaTransmissionProbability(B, Lcoh, Ea, ma);
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Axion Mass: " << ma << std::endl;
                std::cout << "Probability: " << probability << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            probabilities.push_back(probability);
        }
        TGraph *graph = new TGraph(nData, axionMass.data(), probabilities.data());
        graphs.push_back(graph);
    }
      
    /// PLOT ///
    if (kPlot) {
        // Create a canvas to display the plot
        TCanvas *canvas = new TCanvas("canvas", "Probability vs. Axion Mass", 800, 600);
        TLegend *legend = new TLegend(0.62, 0.67, 0.9, 0.9);
        gPad->SetLeftMargin(0.14);

        // Define color options for the graphs
        std::vector<Color_t> colors = {kBlack, kBlue, kRed, kGreen, kCyan};

        // Loop through each coherence length
        for (size_t j = 0; j < graphs.size(); j++) {
            TGraph *graph = graphs[j];
            graph->SetLineColor(colors[j]);
            graph->SetLineWidth(1);
            legend->AddEntry(graph, Form("Lcoh = %d", coherenceLengths[j]), "l");

            graph->SetTitle("");
            graph->GetYaxis()->SetTitle("Probabilidad");
            graph->GetXaxis()->SetTitle("Masa Axion (eV)");
            graph->GetXaxis()->SetRange(mi, mf);
            graph->GetYaxis()->SetRangeUser(1e-27, 1e-17);
            graph->GetXaxis()->SetTitleSize(0.04); 
            graph->GetXaxis()->SetTitleFont(40);  
            graph->GetXaxis()->SetLabelSize(0.04); 
            graph->GetXaxis()->SetLabelFont(40);  
            graph->GetYaxis()->SetTitleSize(0.04); 
            graph->GetYaxis()->SetTitleFont(40);  
            graph->GetYaxis()->SetLabelSize(0.04); 

            if (j == 0)
                graph->Draw("ACP");
            else
                graph->Draw("CP SAME");
        }

        gPad->SetLeftMargin(0.145);
        gPad->SetBottomMargin(0.13);
        gPad->SetLogy(useLogScale);
        legend->SetTextSize(0.0425);
        legend->Draw();   
        canvas->Update();

        // Save the canvas if required
        if (kSave) {                   
            std::string folder = "CoherenceAnalysis/";
            std::ostringstream ossB;
            ossB << std::fixed << std::setprecision(1) << B;
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }
            std::string name = folder + "ProbabilityVsMass_ConstantField_B:" + ossB.str() + ".pdf"; 
            canvas->SaveAs(name.c_str());
        }

        delete canvas;
        delete legend;
    }

    return 0;
}
