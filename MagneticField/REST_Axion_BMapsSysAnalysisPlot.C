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
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description: This script performs analysis on multiple magnetic field maps, calculating the 
//*** probability and error for each map along different mass values. It then plots these values to 
//*** facilitate comparison between the magnetic field maps.
//***
//*** Field Map Definitions:
//*** - MentiskCut: Magnetic field map with 1cm precision in X and Y axes and 5cm in Z axis for the babyIAXO magnet. Cut-off in Z-range (-6,6)m.
//*** - Mentisk: Magnetic field map with 1cm precision in X and Y axes and 5cm in Z axis for the babyIAXO magnet. Full length.
//*** - Bykovskiy2019: Magnetic field map with 5cm precision in X, Y, and Z axes for the babyIAXO magnet.
//*** - Bykovskiy2020: Magnetic field map with 1cm precision in XY axes and 5cm in Z axis for the babyIAXO magnet.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate (default: 150).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the gas used in the buffer gas (default: "He").
//*** - mi: Initial axion mass in eV (default: 0).
//*** - mf: Final axion mass in eV (default: 0.5).
//*** - useLogScale: whether to use y-axis in log scale (default: true)
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

struct FieldInfo {
    std::unique_ptr<TRestAxionMagneticField> magneticField;
    std::unique_ptr<TRestAxionField> axionField;

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<double> timeComputation;
};

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

Int_t REST_Axion_BMapsSysAnalysisPlot(Int_t nData = 150, Double_t Ea = 4.2, std::string gasName = "He", 
                                            Double_t mi = 0.28, Double_t mf = 0.42, Bool_t useLogScale = true) {

    /// Definition of variables
    const char* cfgFileName = "fields.rml";
    const TVector3 position(-5, 5, -11000);
    const TVector3 direction = (position - TVector3(5, -5, 11000)).Unit();
    const Double_t gasDensity = 2.9868e-10;
    std::vector<Double_t> mass;

    Double_t accuracy = 0.45;

    // Define all four fields
    std::map<std::string, FieldInfo> fields;
    fields["MentinkCut"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_2024_cutoff"), std::make_unique<TRestAxionField>()};
    fields["Mentink"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_2024"), std::make_unique<TRestAxionField>()};
    fields["Bykovskiy2019"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO"), std::make_unique<TRestAxionField>()};
    fields["Bykovskiy2020"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_HD"), std::make_unique<TRestAxionField>()};

    // Set up buffer gas
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }
    
    // Set up Axion field and assign magentic field
    for (auto& field : fields) {
        if (gas != nullptr) {
            field.second.axionField->AssignBufferGas(gas.get());
        }
        field.second.magneticField->SetTrack(position, direction);
        field.second.axionField->AssignMagneticField(field.second.magneticField.get());
    } 

    // Calculate transmission probability and computation time for each field map for each mass
    Double_t step = (mf - mi) / nData;
    for (unsigned j = 0; j < nData; j++) {
        Double_t ma = (mi + j * step);

        if(kDebug){
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << "Mass: " << ma << std::endl;
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << std::endl;
        }

        for (auto& field : fields) {
            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            field.second.probability.push_back(probField.first);
            field.second.error.push_back(probField.second);
            field.second.timeComputation.push_back(duration.count());

            if (kDebug) {
                
                std::cout << field.first << std::endl;
                std::cout << "Probability: " << probField.first << std::endl;
                std::cout << "Error: " << probField.second << std::endl;
                std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
                std::cout << std::endl;
            }       
        }

        mass.push_back(ma);
    }

    if (kPlot) { /// PLOT ///
        std::vector<Color_t> colors = {kBlue+2, kMagenta-6, kYellow+3, kGreen+2};

        // Canvas 1 - Bykovskiy2019 vs Bykovskiy2020
        TCanvas* canvas1 = new TCanvas("canvas1", "", 800, 600);
        canvas1->cd();

        // Pad for graphs
        TPad *padTop = new TPad("PadTop", "", 0.0 ,0.3 ,1.0, 1.0);
        padTop->SetTopMargin(0.10);
        padTop->SetLeftMargin(0.165);
        padTop->SetBottomMargin(0);
        padTop->SetRightMargin(0.05);
        padTop->SetBorderMode(0);
        padTop->Draw();
        
        // Pad for residuals
        TPad *padBottom = new TPad("PadBottom", "", 0.0 ,0.0 ,1.0, 0.3);
        padBottom->SetTopMargin(0.);
        padBottom->SetLeftMargin(0.165);
        padBottom->SetBottomMargin(0.4);
        padBottom->SetRightMargin(0.05);
        padBottom->SetBorderMode(-1);
        padBottom->Draw();

        padTop->cd();
        TLegend* legendB = new TLegend(0.67, 0.7, 0.95, 0.9);
        Int_t colorIndex = 0;

        TGraphErrors* graphByb2019 = new TGraphErrors(nData, &mass[0], &fields["Bykovskiy2019"].probability[0], 0, &fields["Bykovskiy2019"].error[0]);      
        graphByb2019->SetTitle("");
        graphByb2019->GetXaxis()->SetTitle("Masa Axion (eV)");
        graphByb2019->GetYaxis()->SetTitle("Probabilidad");
        graphByb2019->GetXaxis()->SetRangeUser(mi, mf);
        graphByb2019->GetYaxis()->SetRangeUser(5e-27, 1e-18);
        graphByb2019->GetXaxis()->SetTitleSize(0.07); 
        graphByb2019->GetXaxis()->SetTitleFont(40);  
        graphByb2019->GetXaxis()->SetLabelSize(0.07); 
        graphByb2019->GetXaxis()->SetLabelFont(40);  
        graphByb2019->GetYaxis()->SetTitleSize(0.07); 
        graphByb2019->GetYaxis()->SetTitleFont(40);  
        graphByb2019->GetYaxis()->SetLabelSize(0.07); 
        graphByb2019->GetYaxis()->SetLabelFont(40);
        graphByb2019->GetYaxis()->SetLabelSize(0.07); 
        graphByb2019->GetYaxis()->SetNdivisions(305);  // Set Y axis divisions
        graphByb2019->GetXaxis()->SetNdivisions(305);  // Set X axis divisions
        graphByb2019->SetLineColor(colors[colorIndex]);
        graphByb2019->SetLineWidth(1);
        legendB->AddEntry(graphByb2019, "Bykovskiy2019", "l");
        colorIndex++;
                                
        TGraphErrors* graphByb2020 = new TGraphErrors(nData, &mass[0], &fields["Bykovskiy2020"].probability[0], 0, &fields["Bykovskiy2020"].error[0]);
        graphByb2020->SetLineColor(colors[colorIndex]);
        graphByb2020->SetLineWidth(1);  
        graphByb2020->SetTitle("");
        graphByb2020->GetXaxis()->SetTitle("Masa Axion (eV)");
        graphByb2020->GetYaxis()->SetTitle("Probabilidad");
        graphByb2020->GetXaxis()->SetRangeUser(mi, mf);
        graphByb2020->GetYaxis()->SetRangeUser(5e-27, 1e-18);
        graphByb2020->GetXaxis()->SetTitleSize(0.07); 
        graphByb2020->GetXaxis()->SetTitleFont(40);  
        graphByb2020->GetXaxis()->SetLabelSize(0.07); 
        graphByb2020->GetXaxis()->SetLabelFont(40);  
        graphByb2020->GetYaxis()->SetTitleSize(0.07); 
        graphByb2020->GetYaxis()->SetTitleFont(40);  
        graphByb2020->GetYaxis()->SetLabelSize(0.07); 
        graphByb2020->GetYaxis()->SetLabelFont(40);
        graphByb2020->GetYaxis()->SetLabelSize(0.07); 
        graphByb2020->GetYaxis()->SetNdivisions(305);  // Set Y axis divisions
        graphByb2020->GetXaxis()->SetNdivisions(305);  // Set X axis divisions
        colorIndex++;

        legendB->AddEntry(graphByb2020, "Bykovskiy2020", "l");                      
        graphByb2019->Draw("ACP");
        graphByb2020->Draw("ACP Same");

        legendB->SetTextSize(0.055);
        if (useLogScale) 
            padTop->SetLogy();

        legendB->Draw();

        std::vector<Double_t> residualsBykovskiy;
        for (unsigned j = 0; j < nData; j++) {
            Double_t residual = std::abs(fields["Bykovskiy2019"].probability[j] - fields["Bykovskiy2020"].probability[j]) / fields["Bykovskiy2019"].probability[j] * 100.0;
            residualsBykovskiy.push_back(residual);
        }

        padBottom->cd();
        TGraph* residualGraphBykovskiy = new TGraph(nData, &mass[0], &residualsBykovskiy[0]);
        residualGraphBykovskiy->SetMarkerStyle(8); 
        residualGraphBykovskiy->SetMarkerColor(kBlue+3);
        residualGraphBykovskiy->SetMarkerSize(0.65);  
        residualGraphBykovskiy->SetTitle("");
        residualGraphBykovskiy->GetXaxis()->SetTitle("Masa Axion (eV)");
        residualGraphBykovskiy->GetYaxis()->SetTitle("Residuos (%)");
        residualGraphBykovskiy->GetXaxis()->SetTitleSize(0.16);
        residualGraphBykovskiy->GetXaxis()->SetLabelSize(0.16);
        residualGraphBykovskiy->GetYaxis()->SetTitleSize(0.12);
        residualGraphBykovskiy->GetYaxis()->SetLabelSize(0.11);
        residualGraphBykovskiy->GetYaxis()->SetTitleFont(40);
        residualGraphBykovskiy->GetYaxis()->SetTitleOffset(0.6);
        residualGraphBykovskiy->GetXaxis()->SetTitleFont(40); 
        residualGraphBykovskiy->GetYaxis()->SetLabelFont(40);
        residualGraphBykovskiy->GetXaxis()->SetLabelFont(40);
        residualGraphBykovskiy->GetYaxis()->SetNdivisions(305);  // Set Y axis divisions
        residualGraphBykovskiy->GetXaxis()->SetNdivisions(305);  // Set X axis divisions 


        residualGraphBykovskiy->GetXaxis()->SetRange(mi, mf); 
        residualGraphBykovskiy->Draw("AP");

        canvas1->Update();

        // Canvas 2 - Mentisk vs MentiskCut
        TCanvas* canvas2 = new TCanvas("canvas2", "", 800, 600);
        canvas2->cd();

        // Pad for graphs
        TPad *padTop1 = new TPad("PadTop1", "", 0.0 ,0.3 ,1.0, 1.0);
        padTop1->SetTopMargin(0.1);
        padTop1->SetLeftMargin(0.165);
        padTop1->SetBottomMargin(0.0);
        padTop1->SetRightMargin(0.05);
        padTop1->SetBorderMode(-1);
        padTop1->Draw();
        
        // Pad for residuals
        TPad *padBottom1 = new TPad("PadBottom1", "", 0.0 ,0.0 ,1.0, 0.3);
        padBottom1->SetTopMargin(0.);
        padBottom1->SetLeftMargin(0.165);
        padBottom1->SetBottomMargin(0.4);
        padBottom1->SetRightMargin(0.05);
        padBottom1->SetBorderMode(-1);
        padBottom1->Draw();

        padTop1->cd();
        std::vector<TGraph*> graphs2;
        TLegend* legendM = new TLegend(0.67, 0.7, 0.95, 0.9);
        for (auto& field : fields) {
            if (field.first == "Mentink" || field.first == "MentinkCut") {
                TGraphErrors* graph = new TGraphErrors(nData, &mass[0], &field.second.probability[0], 0, &field.second.error[0]);

                graph->SetLineColor(colors[colorIndex]);
                graph->SetLineWidth(1);

                if (graphs2.empty()) {
                    graph->SetTitle("");
                    graph->GetXaxis()->SetTitle("Masa Axion (eV)");
                    graph->GetYaxis()->SetTitle("Probabilidad");
                    graph->GetXaxis()->SetRange(mi, mf);
                    graph->GetYaxis()->SetRangeUser(5e-32, 1e-18);
                    graph->GetXaxis()->SetTitleSize(0.07); 
                    graph->GetXaxis()->SetTitleFont(40);  
                    graph->GetXaxis()->SetLabelSize(0.07); 
                    graph->GetXaxis()->SetLabelFont(40);  
                    graph->GetYaxis()->SetTitleSize(0.07); 
                    graph->GetYaxis()->SetTitleFont(40);  
                    graph->GetYaxis()->SetLabelSize(0.07); 
                    graph->GetYaxis()->SetLabelFont(40);
                    graph->GetYaxis()->SetLabelSize(0.07); 
                    graph->GetYaxis()->SetNdivisions(305);  // Set Y axis divisions
                    graph->GetXaxis()->SetNdivisions(305);  // Set X axis divisions
                    graph->Draw("ACP");
                } else {
                    graph->GetYaxis()->SetNdivisions(305);  // Set Y axis divisions
                    graph->GetXaxis()->SetNdivisions(305);  // Set X axis divisions
                    graph->Draw("CP SAME"); 
                }

                //graph->Draw((field.first == "Mentisk") ? "ACP" : "L SAME");
                graphs2.push_back(graph);
                legendM->AddEntry(graph, field.first.c_str(), "l");
                colorIndex++;
            }
        }

        legendM->SetTextSize(0.055);
        if (useLogScale)
            padTop1->SetLogy();
        legendM->Draw();

        std::vector<Double_t> residualsMentisk;
        for (unsigned j = 0; j < nData; j++) {
            Double_t residual = std::abs(fields["Mentink"].probability[j] - fields["MentinkCut"].probability[j]) / fields["Mentink"].probability[j] * 100.0;
            residualsMentisk.push_back(residual);
        }

        padBottom1->cd();
        TGraph* residualGraphMentisk = new TGraph(nData, &mass[0], &residualsMentisk[0]);
        residualGraphMentisk->SetMarkerStyle(8); 
        residualGraphMentisk->SetMarkerColor(kGreen-3);
        residualGraphMentisk->SetMarkerSize(0.6); 
        residualGraphMentisk->SetTitle(""); 
        residualGraphMentisk->GetXaxis()->SetTitle("Masa Axion (eV)");
        residualGraphMentisk->GetYaxis()->SetTitle("Residuos (%)");
        residualGraphMentisk->GetXaxis()->SetTitleSize(0.16);
        residualGraphMentisk->GetXaxis()->SetLabelSize(0.16);
        residualGraphMentisk->GetYaxis()->SetTitleSize(0.12);
        residualGraphMentisk->GetYaxis()->SetLabelSize(0.11);
        residualGraphMentisk->GetYaxis()->SetTitleFont(40);     
        residualGraphMentisk->GetYaxis()->SetTitleOffset(0.6);
        residualGraphMentisk->GetXaxis()->SetTitleFont(40); 
        residualGraphMentisk->GetYaxis()->SetLabelFont(40);     
        residualGraphMentisk->GetXaxis()->SetLabelFont(40);
        residualGraphMentisk->GetYaxis()->SetNdivisions(305);  // Set Y axis divisions
        residualGraphMentisk->GetXaxis()->SetNdivisions(305);  // Set X axis divisions 

        residualGraphMentisk->GetXaxis()->SetRange(mi, mf);  
        residualGraphMentisk->Draw("AP");
        padBottom1->SetLogy();

        canvas2->Update();

        // Canvas for runTime
        TCanvas* canvas3 = new TCanvas("canvas3", "", 800, 600);
        canvas3->cd();
        TMultiGraph* mg = new TMultiGraph();
        TLegend* legend = new TLegend(0.65, 0.74, 0.9, 0.9);
        colorIndex = 0;

        for (auto& field : fields) {
            std::vector<double> runtime = field.second.timeComputation;
            TGraph* graph = new TGraph(nData, &mass[0], &runtime[0]);
            graph->SetLineColor(colors[colorIndex]);
            graph->SetLineWidth(1);
            graph->SetTitle(field.first.c_str());
            mg->Add(graph);
            legend->AddEntry(graph, field.first.c_str(), "l");
            colorIndex++;
        }
        legend->SetTextSize(0.05);
        legend->Draw();
        
        gPad->SetLeftMargin(0.145);
        gPad->SetBottomMargin(0.12);
        mg->GetXaxis()->SetTitle("Masa Axion (eV)");
        mg->GetYaxis()->SetTitle("Tiempo computacional (ms)");
        mg->GetXaxis()->SetRange(mi, mf);
        mg->GetXaxis()->SetTitleSize(0.04); 
        mg->GetXaxis()->SetTitleFont(40);  
        mg->GetXaxis()->SetLabelSize(0.04); 
        mg->GetXaxis()->SetLabelFont(40);  
        mg->GetYaxis()->SetTitleSize(0.04); 
        mg->GetYaxis()->SetTitleFont(40);  
        mg->GetYaxis()->SetLabelSize(0.04); 
        mg->GetYaxis()->SetLabelFont(40); 
        mg->Draw("ACP");

        if (kSave) {
            std::string folder = "BMapsAnalysis/";
            std::ostringstream ossAccuracy;
            ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }
            std::string fileNameB, fileNameM, fileNameR;
            fileNameB = folder + "BykovskiyProbabilityMap_Acc_" + ossAccuracy.str();
            fileNameM = folder + "MentiskProbabilityMap_Acc_" + ossAccuracy.str();
            fileNameR = folder + "RunTimeFieldMaps_Acc_" + ossAccuracy.str();
            if (useLogScale) {
                fileNameB += "_log.pdf";
                fileNameM += "_log.pdf";
                fileNameR += "_log.pdf";
            } else {
                fileNameB += ".pdf";
                fileNameM += ".pdf";
                fileNameR += ".pdf";
            }
            canvas1->SaveAs(fileNameB.c_str());
            canvas2->SaveAs(fileNameM.c_str());
            canvas3->SaveAs(fileNameR.c_str());
        }
    }
    return 0;
}