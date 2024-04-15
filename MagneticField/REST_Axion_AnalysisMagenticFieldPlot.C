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
//*** Description: This function, performs an analysis of the magnetic field along multiple predefined tracks. 
//*** It calculates the transmission probability of axions along each track for varying axion masses. Additionally, 
//*** it generates a plot illustrating the relationship between axion mass and transmission  probability for different map fields.
//***
//*** Track Definitions:
//*** - Center Points: Points near the center of the volume along each axis.
//*** - Extreme Points: Points at the extreme ends of the volume along each axis.
//*** - Random Points: Randomly selected points within the volume.
//*** - Symmetric Points: Symmetrically positioned points with respect to the center along each axis.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points (axionMass) to generate (default: 150).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Gas name (default: "He").
//*** - mi: Initial Axion mass in eV (default: 0.2).
//*** - mf: Final Axion mass in eV (default: 0.5).
//*** - dL: The differential element in mm (default: 10)
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`,
//*** `TRestAxionField::GammaTransmissionProbability` and `TRestAxionBufferGas::GetPhotonMass`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************


struct FieldTrack {
    TVector3 startPoint;
    TVector3 endPoint;
    std::vector<double> magneticValues;
    std::vector<double> probability;
};

Int_t REST_Axion_AnalysisMagenticFieldPlot(Int_t nData = 150, Double_t Ea = 4.2, std::string gasName = "He", Double_t mi = 0.2, 
                                            Double_t mf = 0.5, Double_t dL = 10) {
    const bool fDebug = false;
    const bool fPlot = true;
    const bool fSave = true;

    // Create Variables
    const Double_t gasDensity = 2.9836e-10;
    std::vector<Double_t> mass;

    // Define all tracks
    std::map<std::string, FieldTrack> fieldTracks;

    std::vector<TVector3> startPoints = {
        TVector3(0, 0, -6100),
        TVector3(350, 350, -6100),
        TVector3(-350, -350, -6100),
        TVector3(-70, 20, -6100),
        TVector3(-20, 120, -6100),
        TVector3(-50, -90, -6100),
        TVector3(-250, 420, -6100)
    };

    std::vector<TVector3> endPoints = {
        TVector3(0, 0, 6100),
        TVector3(-350, -350, 6100),
        TVector3(350, 350, 6100),
        TVector3(-60, 70, 6100),
        TVector3(100, -40, 6100),
        TVector3(70, -120, 6100),
        TVector3(-270, -500, 6100)
    };

    std::vector<std::string> trackNames = {
        "Center", "Extreme1", "Extreme2", "Random", "Random1", "Random2", "Outside"
    };

    // Populate fieldTracks
    for (size_t i = 0; i < startPoints.size(); ++i) {
        fieldTracks.emplace(trackNames[i], FieldTrack{startPoints[i], endPoints[i]});
    }

    // Create an instance of TRestAxionBufferGas if gasName is provided
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    std::vector<std::string> fieldNames ={"babyIAXO_2024_cutoff",  "babyIAXO_2024"};

    for(const auto& fieldName : fieldNames){

        // Create an instance of TRestAxionMagneticField
        TRestAxionMagneticField *field = new TRestAxionMagneticField("fields.rml", fieldName);

        // Create an instance of TRestAxionField and assign magnetic field and gas (if provided)
        TRestAxionField* axionField = new TRestAxionField();
        axionField->AssignMagneticField(field);
        if (gas != nullptr) {
            axionField->AssignBufferGas(gas);
        }

        // Calculate magnetic field values for each track
        for (auto& fieldTrack : fieldTracks) {
            fieldTrack.second.magneticValues.clear();
            fieldTrack.second.probability.clear();
            mass.clear();

            auto start_time = std::chrono::high_resolution_clock::now();
            fieldTrack.second.magneticValues = field->GetTransversalComponentAlongPath(fieldTrack.second.startPoint, fieldTrack.second.endPoint, dL);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            if(fDebug){
                std::cout << "Time: " << duration.count() << " ms" << std::endl;    
                std::cout << fieldTrack.first << " magneticValues:" << std::endl;
            }
        }

        // Iterate over each mass
        Double_t step = (mf - mi) / nData; 
        for(unsigned j = 0; j<nData; j++){
            Double_t axionMass = (mi + j * step);
            for (auto& fieldTrack : fieldTracks) {
                const auto& magneticValues = fieldTrack.second.magneticValues;
                Double_t probability = axionField->GammaTransmissionProbability(magneticValues, dL, Ea, axionMass);
                fieldTrack.second.probability.push_back(probability);

                if (fDebug) {
                    std::cout << fieldTrack.first << std::endl;
                    std::cout << "Probability: " << probability << std::endl;
                    std::cout << std::endl;
                }
            }
            mass.push_back(axionMass);
        }  
        
        for (auto& fieldTrack : fieldTracks){
            std::cout << "MagneticValues" << std::endl;
            for (const auto& value : fieldTrack.second.magneticValues) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
            std::cout << "Probability" << std::endl;
            for (const auto& value : fieldTrack.second.probability) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
            std::cout << "Mass" << std::endl;
            for (const auto& value : mass) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
 
        if (fPlot) {
            // Create a canvas to display the main plot
            TCanvas *canvas = new TCanvas("canvas", "Probability vs. Magnetic Field", 800, 500);
            TLegend *legend = new TLegend(0.7, 0.7, 0.9, 0.9);

            // Define color options for the graphs
            std::vector<Color_t> colors = {kYellow+1, kYellow-5, kGreen+1, kGreen-1, kCyan+1, kBlue+1, kMagenta+1, kRed+1};

            // Loop through each field track to create individual graphs
            size_t colorIndex = 0;
            std::vector<TGraph*> graphs;
            for (const auto& fieldTrack : fieldTracks) {
                // Extract the probabilities for the current field track
                const auto& probabilities = fieldTrack.second.probability;

                TGraph *graph = new TGraph(probabilities.size(), &mass[0], probabilities.data());
                graph->SetLineColor(colors[colorIndex]); 
                graph->SetLineWidth(2);
                graph->SetTitle("");

                // Draw the graph on the canvas with "Same" option after the first graph
                if (graphs.empty()) {
                    graph->Draw("ACP");
                    graph->GetXaxis()->SetTitle("Masa Axion (eV)");
                    graph->GetYaxis()->SetTitle("Probabilidad");
                    graph->GetXaxis()->SetRange(mi, mf);
                    graph->GetYaxis()->SetRangeUser(1e-30, 1e-18);
                    graph->GetXaxis()->SetTitleSize(0.03); 
                    graph->GetXaxis()->SetTitleFont(40);  
                    graph->GetXaxis()->SetLabelSize(0.025); 
                    graph->GetXaxis()->SetLabelFont(40);  
                    graph->GetYaxis()->SetTitleSize(0.03); 
                    graph->GetYaxis()->SetTitleFont(40);  
                    graph->GetYaxis()->SetLabelSize(0.025); 
                } else {
                    graph->Draw("CP SAME"); 
                    graph->GetXaxis()->SetRange(mi, mf);
                }

                graphs.push_back(graph);
                legend->AddEntry(graph, fieldTrack.first.c_str(), "l");
                
                // Increment color index, or reset to 0 if exceeding the color options
                colorIndex = (colorIndex + 1) % colors.size();
            }

            canvas->SetLogy();
            legend->Draw();   
            canvas->Update();

            if (fSave) {                   
                std::string folder = "TrackAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }
                std::string name = folder + "ProbabilityVsMass_" + fieldName + ".pdf"; 
                canvas->SaveAs(name.c_str());
            }

            // Clean up
            delete legend;
            delete canvas;

            // Create a canvas to display the residuals
            TCanvas *canvas_residuals = new TCanvas("canvas_residuals", "Residuals", 800, 600);
            canvas_residuals->Divide(2, 1);

            // Calculate residuals between "random1" and "extreme1" against the "center" track
            std::vector<double> residualValues_random1_center;
            std::vector<double> residualValues_extreme1_center;
            for (size_t j = 0; j < mass.size(); ++j) {
                double residual_random1_center = std::abs((fieldTracks["Center"].probability[j] - fieldTracks["Random1"].probability[j])) / fieldTracks["Center"].probability[j] * 100.0;
                double residual_extreme1_center = std::abs((fieldTracks["Center"].probability[j] - fieldTracks["Extreme1"].probability[j])) / fieldTracks["Center"].probability[j] * 100.0;
                residualValues_random1_center.push_back(residual_random1_center);
                residualValues_extreme1_center.push_back(residual_extreme1_center);
            }

            // Create TGraphs for the residuals
            TGraph *residuals_random1_center = new TGraph(mass.size(), &mass[0], residualValues_random1_center.data());
            TGraph *residuals_extreme1_center = new TGraph(mass.size(), &mass[0], residualValues_extreme1_center.data());
            residuals_random1_center->SetMarkerColor(kMagenta);
            residuals_extreme1_center->SetMarkerColor(kBlue);
            residuals_random1_center->SetMarkerStyle(8);
            residuals_extreme1_center->SetMarkerStyle(8);

            // Draw residuals on the canvas
            canvas_residuals->cd(1);
            gPad->SetLogy();
            residuals_random1_center->SetMarkerSize(0.3);
            residuals_random1_center->Draw("AP");
            residuals_random1_center->SetTitle("");
            residuals_random1_center->GetYaxis()->SetTitle("Residuos (%)");
            residuals_random1_center->GetXaxis()->SetTitle("Masa Axion (eV)");
            residuals_random1_center->GetXaxis()->SetRange(mi, mf);
            residuals_random1_center->GetXaxis()->SetTitleSize(0.04);
            residuals_random1_center->GetXaxis()->SetLabelSize(0.03);
            residuals_random1_center->GetYaxis()->SetTitleSize(0.04);
            residuals_random1_center->GetYaxis()->SetLabelSize(0.03);
            residuals_random1_center->GetYaxis()->SetTitleFont(62);
            residuals_random1_center->GetYaxis()->SetTitleOffset(1.40);
            residuals_random1_center->GetXaxis()->SetTitleFont(62);
            residuals_random1_center->GetYaxis()->SetLabelFont(62);
            residuals_random1_center->GetXaxis()->SetLabelFont(62);
            canvas_residuals->cd(1)->SetLeftMargin(0.15);

            canvas_residuals->cd(2);
            gPad->SetLogy();
            residuals_extreme1_center->SetMarkerSize(0.3);
            residuals_extreme1_center->Draw("AP");
            residuals_extreme1_center->SetTitle("");
            residuals_extreme1_center->GetYaxis()->SetTitle("Residuos (%)");
            residuals_extreme1_center->GetXaxis()->SetTitle("Masa Axion (eV)");
            residuals_extreme1_center->GetXaxis()->SetRange(mi, mf);
            residuals_extreme1_center->GetXaxis()->SetTitleSize(0.04);
            residuals_extreme1_center->GetXaxis()->SetLabelSize(0.03);
            residuals_extreme1_center->GetYaxis()->SetTitleSize(0.04);
            residuals_extreme1_center->GetYaxis()->SetLabelSize(0.03);
            residuals_extreme1_center->GetYaxis()->SetTitleFont(62);
            residuals_extreme1_center->GetYaxis()->SetTitleOffset(1.05);
            residuals_extreme1_center->GetXaxis()->SetTitleFont(62);
            residuals_extreme1_center->GetYaxis()->SetLabelFont(62);
            residuals_extreme1_center->GetXaxis()->SetLabelFont(62);

            // Save canvas if required
            if (fSave) {                   
                std::string folder = "TrackAnalysis/";
                if (!std::filesystem::exists(folder)) {
                    std::filesystem::create_directory(folder);
                }
                std::string name = folder + "Residuals_" + fieldName + ".pdf"; 
                canvas_residuals->SaveAs(name.c_str());
            }

            // Clean up residuals canvas
            delete canvas_residuals;
            delete residuals_random1_center;
            delete residuals_extreme1_center;
        }
        // Clean up
        delete axionField;
        delete field;
    }
    // Clean up
    delete gas;

    return 0;
}