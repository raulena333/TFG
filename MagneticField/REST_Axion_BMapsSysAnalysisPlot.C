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

void FormatYAxisAsPercentage(TGraph* graph);

Int_t REST_Axion_BMapsSysAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", 
                                            Double_t mi = 0.2, Double_t mf = 0.5, Bool_t useLogScale = true) {

    /// Definition of variables
    const char* cfgFileName = "fields.rml";
    const TVector3 position(-5, 5, -11000);
    const TVector3 direction = (position - TVector3(5, -5, 11000)).Unit();
    const Double_t gasDensity = 2.9868e-10;
    std::vector<Double_t> mass;

    Double_t accuracy = 0.3;

    // Define all four fields
    std::map<std::string, FieldInfo> fields;
    fields["MentiskCut"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_2024_cutoff"), std::make_unique<TRestAxionField>()};
    fields["Mentisk"] = {std::make_unique<TRestAxionMagneticField>(cfgFileName, "babyIAXO_2024"), std::make_unique<TRestAxionField>()};
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

    if (kPlot) {
        std::vector<Color_t> colors = {kBlue-2, kMagenta-6, kYellow+3, kGreen+2};
        /// PLOT ///
        TCanvas* canvas1 = new TCanvas("canvas1", "", 800, 600);
        TCanvas* canvas2 = new TCanvas("canvas2", "", 800, 600);

        // Canvas 1 - Bykovskiy2019 vs Bykovskiy2020
        canvas1->cd();
        std::vector<TGraphErrors*> graphs1;
        TLegend* legendB = new TLegend(0.7, 0.8, 0.9, 0.9);
        Int_t colorIndex = 0;
        for (auto& field : fields) {
            if (field.first == "Bykovskiy2019" || field.first == "Bykovskiy2020") {
                TGraphErrors* graph = new TGraphErrors(nData, &mass[0], &field.second.probability[0], 0, &field.second.error[0]);
                
                graph->SetLineColor(colors[colorIndex]);
                graph->SetLineWidth(1);

                if (useLogScale) {
                    canvas1->SetLogy();
                }
                
                graph->Draw((field.first == "Bykovskiy2019") ? "ACP" : "L SAME");
                graphs1.push_back(graph);
                legendB->AddEntry(graph, field.first.c_str(), "l");
                colorIndex++;
            }
        }

        // Set axis labels for canvas 1
        graphs1[0]->SetTitle("");
        graphs1[0]->GetXaxis()->SetTitle("Masa Axion (eV)");
        graphs1[0]->GetYaxis()->SetTitle("Probabilidad");
        graphs1[0]->GetXaxis()->SetRange(mi, mf);
        graphs1[0]->GetYaxis()->SetRangeUser(1e-27, 1e-19);
        graphs1[0]->GetXaxis()->SetTitleSize(0.03); 
        graphs1[0]->GetXaxis()->SetTitleFont(40);  
        graphs1[0]->GetXaxis()->SetLabelSize(0.025); 
        graphs1[0]->GetXaxis()->SetLabelFont(40);  
        graphs1[0]->GetYaxis()->SetTitleSize(0.03); 
        graphs1[0]->GetYaxis()->SetTitleFont(40);  
        graphs1[0]->GetYaxis()->SetLabelSize(0.025); 
        graphs1[0]->GetYaxis()->SetLabelFont(40); 

        legendB->Draw();

        // Canvas 2 - Mentisk vs MentiskCut
        canvas2->cd();

        std::vector<TGraphErrors*> graphs2;
        TLegend* legendM = new TLegend(0.7, 0.8, 0.9, 0.9);
        for (auto& field : fields) {
            if (field.first == "Mentisk" || field.first == "MentiskCut") {
                TGraphErrors* graph = new TGraphErrors(nData, &mass[0], &field.second.probability[0], 0, &field.second.error[0]);

                graph->SetLineColor(colors[colorIndex]);
                graph->SetLineWidth(1);

                if (useLogScale) {
                    canvas2->SetLogy();
                }

                graph->Draw((field.first == "Mentisk") ? "ACP" : "L SAME");
                graphs2.push_back(graph);
                legendM->AddEntry(graph, field.first.c_str(), "l");
                colorIndex++;
            }
        }

        // Set axis labels for canvas 2
        graphs2[0]->SetTitle("");
        graphs2[0]->GetXaxis()->SetTitle("Masa Axion (eV)");
        graphs2[0]->GetYaxis()->SetTitle("Probabilidad");
        graphs2[0]->GetXaxis()->SetRange(mi, mf);
        graphs2[0]->GetYaxis()->SetRangeUser(1e-32, 1e-19);
        graphs2[0]->GetXaxis()->SetTitleSize(0.03); 
        graphs2[0]->GetXaxis()->SetTitleFont(40);  
        graphs2[0]->GetXaxis()->SetLabelSize(0.025); 
        graphs2[0]->GetXaxis()->SetLabelFont(40);  
        graphs2[0]->GetYaxis()->SetTitleSize(0.03); 
        graphs2[0]->GetYaxis()->SetTitleFont(40);  
        graphs2[0]->GetYaxis()->SetLabelSize(0.025); 
        graphs2[0]->GetYaxis()->SetLabelFont(40); 

        legendM->Draw();

        TCanvas* canvas3 = new TCanvas("canvas3", "", 800, 600);
        canvas3->cd();
        TMultiGraph* mg = new TMultiGraph();
        TLegend* legend = new TLegend(0.1, 0.75, 0.3, 0.9);
        colorIndex = 1;

        for (auto& field : fields) {
            std::vector<double> runtime = field.second.timeComputation;
            TGraph* graph = new TGraph(nData, &mass[0], &runtime[0]);
            graph->SetLineColor(colorIndex);
            graph->SetLineWidth(1);
            graph->SetTitle(field.first.c_str());
            mg->Add(graph);
            legend->AddEntry(graph, field.first.c_str(), "l");
            colorIndex++;
        }

        mg->Draw("ACP");
        mg->GetXaxis()->SetTitle("Masa Axion (eV)");
        mg->GetYaxis()->SetTitle("Tiempo computacional (ms)");
        mg->GetXaxis()->SetRange(mi, mf);
        legend->Draw();
        mg->GetXaxis()->SetTitleSize(0.03); 
        mg->GetXaxis()->SetTitleFont(40);  
        mg->GetXaxis()->SetLabelSize(0.025); 
        mg->GetXaxis()->SetLabelFont(40);  
        mg->GetYaxis()->SetTitleSize(0.03); 
        mg->GetYaxis()->SetTitleFont(40);  
        mg->GetYaxis()->SetLabelSize(0.025); 
        mg->GetYaxis()->SetLabelFont(40); 

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
                fileNameB += "_log.png";
                fileNameM += "_log.png";
                fileNameR += "_log.png";
            } else {
                fileNameB += ".png";
                fileNameM += ".png";
                fileNameR += ".png";
            }
            canvas1->SaveAs(fileNameB.c_str());
            canvas2->SaveAs(fileNameM.c_str());
            canvas3->SaveAs(fileNameR.c_str());
        }

        // Create a TCanvas for the Bykovskiy residuals
        TCanvas* canvas4 = new TCanvas("canvas4", "", 500, 300);
        canvas4->cd();

        std::vector<Double_t> residualsBykovskiy;
        for (unsigned j = 0; j < nData; j++) {
            Double_t residual = std::abs(fields["Bykovskiy2019"].probability[j] - fields["Bykovskiy2020"].probability[j]) / fields["Bykovskiy2019"].probability[j] * 100.0;
            residualsBykovskiy.push_back(residual);
        }

        TGraph* residualGraphBykovskiy = new TGraph(nData, &mass[0], &residualsBykovskiy[0]);
        residualGraphBykovskiy->SetMarkerStyle(8); 
        residualGraphBykovskiy->SetMarkerColor(kBlue+3);
        residualGraphBykovskiy->SetMarkerSize(0.4);  
        residualGraphBykovskiy->SetTitle("");
        residualGraphBykovskiy->GetXaxis()->SetTitle("Masa Axion (eV)");
        residualGraphBykovskiy->GetYaxis()->SetTitle("Residuos (%)");
        residualGraphBykovskiy->GetXaxis()->SetTitleSize(0.04);
        residualGraphBykovskiy->GetXaxis()->SetLabelSize(0.03);
        residualGraphBykovskiy->GetYaxis()->SetTitleSize(0.04);
        residualGraphBykovskiy->GetYaxis()->SetLabelSize(0.03);
        residualGraphBykovskiy->GetYaxis()->SetTitleFont(62);
        residualGraphBykovskiy->GetYaxis()->SetTitleOffset(0.8);
        residualGraphBykovskiy->GetXaxis()->SetTitleFont(62); 
        residualGraphBykovskiy->GetYaxis()->SetLabelFont(62);
        residualGraphBykovskiy->GetXaxis()->SetLabelFont(62);

        residualGraphBykovskiy->GetXaxis()->SetRange(mi, mf); 
        residualGraphBykovskiy->Draw("AP");

        // Create a TCanvas for the Mentisk residuals
        TCanvas* canvas5 = new TCanvas("canvas5", "", 500, 300);
        canvas5->cd();

        std::vector<Double_t> residualsMentisk;
        for (unsigned j = 0; j < nData; j++) {
            Double_t residual = std::abs(fields["Mentisk"].probability[j] - fields["MentiskCut"].probability[j]) / fields["Mentisk"].probability[j] * 100.0;
            residualsMentisk.push_back(residual);
        }

        TGraph* residualGraphMentisk = new TGraph(nData, &mass[0], &residualsMentisk[0]);
        residualGraphMentisk->SetMarkerStyle(8); 
        residualGraphMentisk->SetMarkerColor(kGreen-3);
        residualGraphMentisk->SetMarkerSize(0.4); 
        residualGraphMentisk->SetTitle(""); 
        residualGraphMentisk->GetXaxis()->SetTitle("Masa Axion (eV)");
        residualGraphMentisk->GetYaxis()->SetTitle("Residuos (%)");
        residualGraphMentisk->GetXaxis()->SetTitleSize(0.04);
        residualGraphMentisk->GetXaxis()->SetLabelSize(0.03);
        residualGraphMentisk->GetYaxis()->SetTitleSize(0.04);
        residualGraphMentisk->GetYaxis()->SetLabelSize(0.03);
        residualGraphMentisk->GetYaxis()->SetTitleFont(62);     
        residualGraphMentisk->GetYaxis()->SetTitleOffset(0.8);
        residualGraphMentisk->GetXaxis()->SetTitleFont(62); 
        residualGraphMentisk->GetYaxis()->SetLabelFont(62);     
        residualGraphMentisk->GetXaxis()->SetLabelFont(62); 

        residualGraphMentisk->GetXaxis()->SetRange(mi, mf);  
        residualGraphMentisk->Draw("AP");

        gPad->SetLogy();


        // Save the plots if kSave is true
        if (kSave) {
            std::string folder = "BMapsAnalysis/";
            std::ostringstream ossAccuracy;
            ossAccuracy << std::fixed << std::setprecision(2) << accuracy;
            std::string fileNameBykovskiyResidual = folder + "BykovskiyResidual_Acc_" + ossAccuracy.str() + ".png";
            canvas4->SaveAs(fileNameBykovskiyResidual.c_str());  
            std::string fileNameMentiskResidual = folder + "MentiskResidual_Acc_" + ossAccuracy.str() + ".png";
            canvas5->SaveAs(fileNameMentiskResidual.c_str());   
        }

    }
    return 0;
}

