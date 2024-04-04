#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>
#include <TCanvas.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** This script performs an analysis of axion detection probability and runtime in a magnetic field 
//*** environment, comparing the effects of using different gases or a vacuum. It calculates the 
//*** probability of axion transmission through the field at various axion masses and measures the 
//*** computation time required for each scenario.
//*** 
//*** Gas Definitions:
//*** - 'He': Helium gas defined by its density.
//*** - '': Vacuum.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 50).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - mi: Initial axion mass in eV (default: -0.1).
//*** - mf: Final axion mass in eV (default: 0.1).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability` and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct gasTrack
{
    TRestAxionField *axionField;
    TRestAxionBufferGas *gas;
    std::string gasName;

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<double> timeComputation;

};

Int_t REST_Axion_GasAnalysisMassProb(Int_t nData = 50, Double_t Ea = 4.2, Double_t mi = -0.1, Double_t mf = 0.1)
{

    Bool_t fDebug = true;
    Bool_t fPlot = true;
    Bool_t fSave = true;

    // Create Variables
    std::string fieldName = "babyIAXO_2024_cutoff";
    Double_t gasDensity = 2.6e-9;
    TVector3 position(-100, -100, -11000);
    TVector3 direction(0.01, 0.01, 1);
    std::vector<Double_t> mass;

    // Create magnetic field
    TRestAxionMagneticField *magneticField = new TRestAxionMagneticField("fields.rml", fieldName);

    // Create gas tracks
    std::map<std::string, gasTrack> gasTracks = {
        {"He-Gas", {new TRestAxionField(), new TRestAxionBufferGas(), "He"}},
        {"Vacuum", {new TRestAxionField(), nullptr, ""}}
    };

    // Assign gas and magnetic field to each gas track
    for (auto &track : gasTracks)
    {
        if (track.second.gas != nullptr)
        {
            track.second.gas->SetGasDensity(track.second.gasName, gasDensity);
            track.second.axionField->AssignBufferGas(track.second.gas);
        }
        track.second.axionField->AssignMagneticField(magneticField);
        magneticField->SetTrack(position, direction);
    }

    // Simulation loop
    Double_t step = (mf - mi) / nData;
    for (unsigned i = 0; i < nData; i++)
    {
        Double_t ma = (mi + i * step);
        for (auto &track : gasTracks)
        {
            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = track.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 100, 20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            track.second.probability.push_back(probField.first);
            track.second.error.push_back(probField.second);
            track.second.timeComputation.push_back(duration.count());

            if (fDebug)
            {
                std::cout << "Mass: " << ma << std::endl;
                std::cout << track.first << std::endl;
                std::cout << "Probability: " << probField.first << std::endl;
                std::cout << "Error: " << probField.second << std::endl;
                std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
                std::cout << std::endl;
            }
        }

        mass.push_back(ma);
    }

    /// PLOT ///
    if (fPlot) {
        // Create TCanvas for plotting probability vs. mass
        TCanvas* canvas1 = new TCanvas("c1", "c1", 900, 450);
        canvas1->Divide(2, 1);

        // Plot the probability vs. mass for each FieldTrack
        Int_t i = 1;
        for (auto& track : gasTracks) {
            canvas1->cd(i);

            TGraphErrors* graph = new TGraphErrors(nData, &mass[0], &track.second.probability[0], nullptr, &track.second.error[0]);
            graph->SetLineColor(i);
            graph->SetLineWidth(1);

            graph->SetTitle((track.first + " Probability vs. Mass").c_str());
            graph->GetXaxis()->SetTitle("Mass (eV)");
            graph->GetYaxis()->SetTitle("Probability");

            graph->Draw("ACP");
            i++;
        }

        // Create TCanvas for plotting runtime vs. mass
        TCanvas* canvas2 = new TCanvas("c2", "c2", 800, 650);
        TMultiGraph* mg0 = new TMultiGraph();

        // Create a legend for the graphs
        TLegend* legend0 = new TLegend(0.7, 0.7, 0.9, 0.9);

        // Plot the runtime vs. mass for each FieldTrack
        Int_t colorIndex = 1;
        for (auto& track : gasTracks) {
            TGraph* graph = new TGraph(nData, &mass[0], &track.second.timeComputation[0]);
            graph->SetLineColor(colorIndex);
            graph->SetLineWidth(1);

            // Add the graph to the TMultiGraph
            mg0->Add(graph);

            // Add entry to the legend
            legend0->AddEntry(graph, (track.first).c_str(), "l");

            colorIndex++;
        }

        // Set titles and axes labels
        mg0->SetTitle("Runtime vs. Mass");
        mg0->GetXaxis()->SetTitle("Mass (eV)");
        mg0->GetYaxis()->SetTitle("Runtime (ms)");
        mg0->GetXaxis()->SetTitleSize(0.04);
        mg0->GetXaxis()->SetLabelSize(0.03);
        mg0->GetYaxis()->SetTitleSize(0.04);
        mg0->GetYaxis()->SetLabelSize(0.03);

        // Draw the TMultiGraph
        canvas2->cd();
        mg0->Draw("ACP");

        // Add the legend
        legend0->Draw();

        if (fSave) {
            canvas1->SaveAs("ProbabilityGas.png");
            canvas2->SaveAs("RuntimeGas.png");
        }
    }


    // Clean up memory
    delete magneticField;
    for (auto &track : gasTracks)
    {
        delete track.second.axionField;
        if (track.second.gas != nullptr)
            delete track.second.gas;
    }

    return 0;
}