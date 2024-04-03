#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>

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
//*** Mesh Map Definitions in mm:
//*** (20, 20, 100), (30, 30, 150), (50, 50, 250), (50, 50, 500)
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial axion mass in eV (default: -0.1).
//*** - mf: Final axion mass in eV (default: 0.1).
//*** - Accuracy: Accuracy value for intrgration in GSL, depends on the axion mass (default 0.2).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetInterpolation'. and 
//*** `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************


struct FieldTrack {
    bool interpolation;

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<double> timeComputation;
};


int REST_Axion_InterpolationAnalysisMassProb(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", 
                    Double_t mi = 0, Double_t mf = 0.1, Double_t accuracy = 0.2){

    Bool_t fDebug = true;
    Bool_t fPlot = true;
    Bool_t fSave = true;

    // Create Variables
    std::string fieldName = "babyIAXO_2024";
    Double_t gasDensity = 2.9836e-10;
    TVector3 position(-100, -100 ,-11000);
    TVector3 direction(0.01, 0.01 ,1);
    std::vector<Double_t> mass;

    std::map<std::string, FieldTrack> fields = {
        {"Interpolation", {true}},
        {"No-Interpolation", {false}}
    };

    // Create an instance of TRestAxionBufferGas if gasName is provided and TRestAxionMagneticField
    TRestAxionMagneticField *magneticField = new TRestAxionMagneticField("fields.rml", fieldName);
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Create an instance of TRestAxionField and assign magnetic field and gas (if provided)
    TRestAxionField *axionField = new TRestAxionField();
    if (gas != nullptr) {
        axionField->AssignBufferGas(gas);
    }
    magneticField->SetTrack(position,direction);
    axionField->AssignMagneticField(magneticField); 

    // Iterate over each field, enabling interpolation and disabling it
    Double_t step = (mf-mi)/nData;
    for(unsigned i = 0; i<nData; i++){
        Double_t ma = mi + i * step;

        for(auto& field : fields){
            magneticField->SetInterpolation(field.second.interpolation);
            auto start_time = std::chrono::high_resolution_clock::now();
            //Depends a lot on the accuracy, ERROR: cannot reach tolerance because of roundoff error
            std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 1000, 20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            field.second.probability.push_back(probField.first);
            field.second.error.push_back(probField.second);
            field.second.timeComputation.push_back(duration.count());

            if(fDebug){
                std::cout << "Mass: " << ma << std::endl;
                std::cout << field.first << std::endl;
                std::cout << "Probability: " << probField.first << std::endl;
                std::cout << "Error: " << probField.second << std::endl;
                std::cout << "Runtime: " << duration.count() << std::endl;
                std::cout << std::endl;
            }

        }

        mass.push_back(ma);
    }

    /// PLOT ///
    if(fPlot){
        /// Create TCanvas for plotting probability transmission against mass
        TCanvas* canvas1 = new TCanvas("c1", "c1", 800, 650);
        TMultiGraph* mg = new TMultiGraph();
        std::stringstream title;
        title << "Probability vs. Mass for accuracy: " << std::fixed << std::setprecision(1) << accuracy;
        mg->SetTitle(title.str().c_str());


        // Create a legend for the graphs
        TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);

        // Plot the probability vs. mass for each FieldTrack
        Int_t colorIndex = 2;
        for (auto& field : fields) {
            TGraphErrors* graph = new TGraphErrors(nData, &mass[0], &field.second.probability[0], 0, &field.second.error[0]);
            graph->SetLineColor(colorIndex);
            graph->SetLineWidth(1);
            //graph->SetMarkerStyle(20); // Set marker style
            //graph->SetMarkerSize(1);   // Set marker size

            // Add the graph to the TMultiGraph
            mg->Add(graph);

            // Add entry to the legend
            legend->AddEntry(graph, (field.first).c_str(), "l");
            colorIndex++;
        }

        // Draw the TMultiGraph and legend
        canvas1->cd();
        mg->Draw("AC");
        mg->GetXaxis()->SetTitle("Mass (eV)");
        mg->GetYaxis()->SetTitle("Probability");
        mg->GetXaxis()->SetTitleSize(0.035);
        mg->GetXaxis()->SetLabelSize(0.025);
        mg->GetYaxis()->SetTitleSize(0.035);
        mg->GetYaxis()->SetLabelSize(0.025);
        legend->Draw();

        /// Create Canvas for plotting runtime against mass
        TCanvas *canvas2 = new TCanvas("c2", "c2", 800, 600);
        TLegend* legend_run = new TLegend(0.7, 0.7, 0.9, 0.9);
        std::vector<TGraph*> graphs;

        // Plot the runtime vs. mass for each
        colorIndex = 2;
        for (auto& field : fields) {
            TGraph* graph = new TGraph(nData, &mass[0], &field.second.timeComputation[0]);
            graph->SetLineColor(colorIndex);
            graph->SetLineWidth(1);

            graphs.push_back(graph);

            // Add entry to the legend
            legend_run->AddEntry(graph, (field.first).c_str(), "l");
            colorIndex++;
        }

        canvas2->cd();
        graphs[0]->SetTitle("Runtime vs. Mass");
        graphs[0]->GetXaxis()->SetTitle("Mass (eV)");
        graphs[0]->GetYaxis()->SetTitle("Runtime (ms)");
        graphs[0]->Draw("APC");

        for(size_t k = 1; k < graphs.size(); k++){
            graphs[k]->Draw("SAME");
        }

        legend_run->Draw();

        if(fSave){
            canvas1->SaveAs("ProbabilityInterpolation.png");
            canvas2->SaveAs("RuntimeInterpolation.png");
        }
    }

    delete gas;
    delete magneticField;
    delete axionField;

    return 0;
}