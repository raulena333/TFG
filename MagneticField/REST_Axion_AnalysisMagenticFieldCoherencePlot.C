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
//*** - nData: Number of data points (axionMass) to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial Axion mass in eV (default: 0.).
//*** - mf: Final Axion mass in eV (default: 0.4).
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

Int_t REST_Axion_AnalysisMagenticFieldCoherencePlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.2, 
                                            Double_t mf = 0.5, Double_t B = 2.0, Bool_t useLogScale = true) {
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
    std::vector<int> coherenceLengths = {7000, 8000, 9000, 10000};
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
        TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);

        // Define color options for the graphs
        std::vector<Color_t> colors = {kBlack, kBlue, kRed, kGreen};

        // Loop through each coherence length
        for (size_t j = 0; j < graphs.size(); j++) {
            TGraph *graph = graphs[j];
            graph->SetLineColor(colors[j]);
            graph->SetLineWidth(1);
            legend->AddEntry(graph, Form("Lcoh = %d", coherenceLengths[j]), "l");
            if (j == 0)
                graph->Draw("ACP");
            else
                graph->Draw("SAME");
        }

        graphs[0]->SetTitle("");
        graphs[0]->GetYaxis()->SetTitle("Probability");
        graphs[0]->GetXaxis()->SetTitle("Axion Mass (eV)");
        graphs[0]->GetXaxis()->SetTitleSize(0.03);
        graphs[0]->GetYaxis()->SetTitleSize(0.03);
        graphs[0]->GetXaxis()->SetLabelSize(0.03);
        graphs[0]->GetYaxis()->SetLabelSize(0.03);

        canvas->SetLogy(useLogScale);
        legend->Draw();   
        canvas->Update();

        // Save the canvas if required
        if (kSave) {                   
            std::string folder = "CoherenceAnalysis/";
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }
            std::string name = folder + "ProbabilityVsMass_ConstantField_" + std::to_string(B) + ".png"; 
            canvas->SaveAs(name.c_str());
        }
    }

    return 0;
}
