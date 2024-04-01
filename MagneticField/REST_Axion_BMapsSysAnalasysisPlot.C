#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>

#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"
#include <TLatex.h>


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
//*** - nData: Number of data points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the gas used in the buffer gas (default: "He").
//*** - mi: Initial axion mass in eV (default: 0).
//*** - mf: Final axion mass in eV (default: 0.4).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::SetTrack`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************


struct FieldInfo {
    TRestAxionMagneticField* magneticField;
    TRestAxionField* axionField;

    std::vector<double> probability;
    std::vector<double> error;
    std::vector<double> timeComputation;
};

Int_t REST_Axion_BMapsSysAnalysisPlot(Int_t nData = 100, Double_t Ea = 4.2, std::string gasName = "He", 
                                            Double_t mi = 0, Double_t mf = 0.4) {

    Bool_t fDebug = true;
    Bool_t fPlot = true;
    Bool_t fSave = true;

    /// Definition of variables
    const char* cfgFileName = "fields.rml";
    Double_t gasDensity = 1e-11;
    TVector3 position(-100, -100 ,-11000);
    TVector3 direction(0.01, 0.01 ,1);
    std::vector<Double_t> mass;

    // Define all four fields
    std::map<std::string, FieldInfo> fields = {
        {"MentiskCut", {new TRestAxionMagneticField(cfgFileName, "babyIAXO_2024_cutoff"), new TRestAxionField()}},
        {"Mentisk", {new TRestAxionMagneticField(cfgFileName, "babyIAXO_2024"), new TRestAxionField()}},
        {"Bykovskiy2019", {new TRestAxionMagneticField(cfgFileName, "babyIAXO"), new TRestAxionField()}},
        {"Bykovskiy2020", {new TRestAxionMagneticField(cfgFileName, "babyIAXO_HD"), new TRestAxionField()}}
    };

    // Set up buffer gas
    TRestAxionBufferGas* gas = nullptr;
    if (!gasName.empty()) {
        gas = new TRestAxionBufferGas();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Set up Axion field
    for (auto& field : fields) {
        if (gas != nullptr) {
            field.second.axionField->AssignBufferGas(gas);
        }
        field.second.magneticField->SetTrack(position,direction);
        field.second.axionField->AssignMagneticField(field.second.magneticField);
    }

    // Calculate transmission probability and computation time for each field map for each mass
    Double_t step = (mf-mi) / nData;
    for(unsigned j = 0; j< nData; j++){
        Double_t ma = (mi + j*step);
        
        for(auto& field : fields){
            auto start_time = std::chrono::high_resolution_clock::now();
            std::pair<Double_t, Double_t> probField = field.second.axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 100, 20);
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
                std::cout << "Runtime: " <<duration.count() << std::endl;
                std::cout << std::endl;
            }       
        }

        mass.push_back(ma);
    }

    if(fPlot){
        /// PLOT ///
        TCanvas* canvas1 = new TCanvas("canvas1", "Bykovskiy2019 vs Bykovskiy2020", 800, 600);
        TCanvas* canvas2 = new TCanvas("canvas2", "Mentisk vs MentiskCut", 900, 800);

        // Canvas 1 - Bykovskiy2019 vs Bykovskiy2020
        canvas1->cd();
        std::vector<TGraphErrors*> graphs1;
        Int_t colorIndex = 1;
        for (auto& field : fields) {
            if (field.first == "Bykovskiy2019" || field.first == "Bykovskiy2020") {
                TGraphErrors* graph1 = new TGraphErrors(nData, &mass[0], &field.second.probability[0], nullptr, &field.second.error[0]);
                
                graph1->SetLineColor(colorIndex);
                graph1->SetLineWidth(1);
                
                graph1->Draw((field.first == "Bykovskiy2019") ? "ACP" : "L SAME");
                graphs1.push_back(graph1);
                colorIndex++;
            }
        }

        // Set axis labels for canvas 1
        graphs1[0]->GetXaxis()->SetTitle("Mass (eV)");
        graphs1[0]->GetYaxis()->SetTitle("Probability");
        graphs1[0]->SetTitle("Bykovskiy2019 vs Bykovskiy2020");
        graphs1[0]->GetXaxis()->SetTitleSize(0.04);
        graphs1[0]->GetXaxis()->SetLabelSize(0.03);
        graphs1[0]->GetYaxis()->SetTitleSize(0.04);
        graphs1[0]->GetYaxis()->SetLabelSize(0.03);

        // Canvas 2 - Mentisk vs MentiskCut
        canvas2->Divide(2, 1);

        // First pad - Mentisk
        canvas2->cd(1);
        std::vector<TGraphErrors*> graphs2_1;
        for (auto& field : fields) {
            if (field.first == "Mentisk") {
                TGraphErrors* graph2_1 = new TGraphErrors(nData, &mass[0], &field.second.probability[0], nullptr, &field.second.error[0]);
                graph2_1->SetLineColor(colorIndex);
                graph2_1->SetLineWidth(1);

                graph2_1->Draw("ACP");
                graphs2_1.push_back(graph2_1);
                colorIndex++;
            }
        }

        // Set axis labels for the first pad of canvas 2
        graphs2_1[0]->GetXaxis()->SetTitle("Mass (eV)");
        graphs2_1[0]->GetYaxis()->SetTitle("Probability");
        graphs2_1[0]->SetTitle("Mentisk");
        graphs2_1[0]->GetXaxis()->SetTitleSize(0.04);
        graphs2_1[0]->GetXaxis()->SetLabelSize(0.03);
        graphs2_1[0]->GetYaxis()->SetTitleSize(0.04);
        graphs2_1[0]->GetYaxis()->SetLabelSize(0.03);

        // Second pad - MentiskCut
        canvas2->cd(2);
        std::vector<TGraphErrors*> graphs2_2;
        for (auto& field : fields) {
            if (field.first == "MentiskCut") {
                TGraphErrors* graph2_2 = new TGraphErrors(nData, &mass[0], &field.second.probability[0], nullptr, &field.second.error[0]);
                graph2_2->SetLineColor(colorIndex);
                graph2_2->SetLineWidth(1);
                graph2_2->Draw("ACP");
                graphs2_2.push_back(graph2_2);
                colorIndex++;
            }
        }

        // Set axis labels for the second pad of canvas 2
        graphs2_2[0]->GetXaxis()->SetTitle("Mass (eV)");
        graphs2_2[0]->GetYaxis()->SetTitle("Probability");
        graphs2_2[0]->SetTitle("MentiskCut");
        graphs2_2[0]->GetXaxis()->SetTitleSize(0.04);
        graphs2_2[0]->GetXaxis()->SetLabelSize(0.03);
        graphs2_2[0]->GetYaxis()->SetTitleSize(0.04);
        graphs2_2[0]->GetYaxis()->SetLabelSize(0.03);

        TCanvas* canvas3 = new TCanvas("canvas3", "Mass vs Runtime", 800, 600);
        canvas3->cd();
        TMultiGraph* mg = new TMultiGraph();
        TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
        Int_t colorIndex1 = 1;

        for (auto& field : fields) {
            std::vector<double> runtime = field.second.timeComputation;
            TGraph* graph3 = new TGraph(nData, &mass[0], &runtime[0]);
            graph3->SetLineColor(colorIndex1);
            graph3->SetLineWidth(1);
            graph3->SetTitle(field.first.c_str());
            mg->Add(graph3);
            legend->AddEntry(graph3, field.first.c_str(), "l");
            colorIndex1++;
        }

        mg->Draw("ACP");
        mg->GetXaxis()->SetTitle("Mass (eV)");
        mg->GetYaxis()->SetTitle("Runtime (ms)");
        legend->Draw();
        mg->GetXaxis()->SetTitleSize(0.035);
        mg->GetXaxis()->SetLabelSize(0.025);
        mg->GetYaxis()->SetTitleSize(0.035);
        mg->GetYaxis()->SetLabelSize(0.025);

        if(fSave){
            canvas1->SaveAs("BykovskiyProbabilityMap.png");
            canvas2->SaveAs("MentiskProbabilityMap.png");
            canvas3->SaveAs("RunTimeFieldMaps.C");
        }
    }

    delete gas;
    for (auto& field : fields) {
        delete field.second.magneticField;
        delete field.second.axionField;
    }

    return 0;
}