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
#include <TGraph.h> 
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** This function performs standard integral analysis for axion experiments. It calculates the runtime and probability values
//*** for different axion masses and integration lengths (dL). The results are plotted on histograms and graphs for visualization.
//*** 
//*** Arguments by default are (in order):
//*** - nData: Number of dL points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the buffer gas (default: "He").
//*** - m1: Initial axion mass in eV far from resonance (default: 0.01).
//*** - m3: Middle axion mass in eV far/close to resonance (default: 0.1).
//*** - m2: Final axion mass in eV close to resonance (default: 0.3).
//*** - dLinitial: Initial integration length (default: 1).
//*** - dLfinal: Final integration length (default: 200).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`,
//*** `TRestAxionField::GammaTransmissionProbability`, and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

void SetYRange(TGraph* graph, Double_t percentage = 0.1);

Int_t REST_Axion_StandardIntegralAnalysis(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", 
            Double_t m1 = 0.01, Double_t m2 = 0.1, Double_t m3 = 0.3, Int_t dLinitial = 1, Int_t dLfinal = 200){

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024", //"babyIAXO_2024_cutoff"
    };
    const Double_t gasDensity = 2.9836e-10;
    const TVector3 initialPosition(-5, 5, -11000);
    const TVector3 finalPosition(5,-5, 11000);
    std::vector<Double_t> masses;
    std::vector<Double_t> dLvec;

    for (Int_t j = 0; j < nData; j++) {
        Double_t dL = dLinitial + j * (dLfinal - dLinitial) / nData;
        dLvec.push_back(dL);
    }

    // Create an instance of TRestAxionBufferGas if gasName is provided
    std::unique_ptr<TRestAxionBufferGas> gas;
    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
    }

    // Determine masses based on resonance condition
    for (unsigned i = 0; i < 4; ++i) {
        if (i < 3)
            masses.push_back(i == 0 ? m1 : (i == 1 ? m2 : m3));
        else
            masses.push_back(gas != nullptr ? gas->GetPhotonMass(Ea) : 0);
    }
    
    for(const auto &fieldName : fieldNames){
        
        // Create instance of magnetic field, axion field and buffer gas and then assign it.
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        if (gas!= nullptr)
            axionField->AssignBufferGas(gas.get());
        axionField->AssignMagneticField(magneticField.get());

        Int_t j = 0;
        for (const auto &ma : masses){
            if(kDebug){
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Axion Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            std::vector<Double_t> probValues, runValues;

            for(const auto &dL : dLvec){
                if(kDebug){
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "dL: " << dL << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }

                // Runtime for filling the magnetic values
                auto start_timeStandard = std::chrono::high_resolution_clock::now();
                std::vector<Double_t> magneticValues = magneticField->GetTransversalComponentAlongPath(initialPosition, finalPosition, Int_t(dL));
                auto end_timeStandard = std::chrono::high_resolution_clock::now();
                auto durationStandard = std::chrono::duration_cast<std::chrono::microseconds>(end_timeStandard - start_timeStandard);

                // Standard Integration
                auto start_time = std::chrono::high_resolution_clock::now();
                Double_t probField = axionField->GammaTransmissionProbability(magneticValues, dL, Ea, ma);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

                probValues.push_back(probField);
                runValues.push_back((durationStandard.count() + duration.count()) / 1e6);

                if(kDebug){
                    std::cout << "Probability: " << probField << std::endl;
                    std::cout << "Runtime (μs): " << duration.count() << std::endl;
                    std::cout << std::endl;
                }
            }

            if(kPlot){
                if(j == 0){
                    // Create canvas
                    TCanvas *canvas = new TCanvas((fieldName + std::to_string(ma) + "_Analysis").c_str(), (fieldName + std::to_string(ma) + "_Analysis").c_str(), 750, 450);
                    canvas->Divide(2, 1);

                    // Plot probability graph
                    canvas->cd(1);
                    TGraph *graphProb = new TGraph(nData, &dLvec[0], &probValues[0]);
                    graphProb->SetLineColor(kBlack);
                    graphProb->SetLineWidth(2);
                    graphProb->SetTitle("");
                    graphProb->GetXaxis()->SetTitle("dL (mm)");
                    graphProb->GetYaxis()->SetTitle("Probabilidad");
                    graphProb->GetXaxis()->SetRange(dLinitial, dLfinal);
                    //SetYRange(graphProb, 0.05);
                    if(fieldName == "babyIAXO_2024_cutoff")
                        graphProb->GetYaxis()->SetRangeUser(1e-30, 1e-18);
                    else
                        graphProb->GetYaxis()->SetRangeUser(1e-32, 1e-28);
                    gPad->SetLogy();
                    graphProb->GetXaxis()->SetTitleSize(0.03);
                    graphProb->GetXaxis()->SetTitleFont(40);
                    graphProb->GetXaxis()->SetLabelSize(0.025);
                    graphProb->GetXaxis()->SetLabelFont(40);
                    graphProb->GetYaxis()->SetTitleSize(0.03);
                    graphProb->GetYaxis()->SetTitleFont(40);
                    graphProb->GetYaxis()->SetLabelSize(0.025);
                    graphProb->GetYaxis()->SetLabelFont(40);
                    graphProb->Draw("ACP");

                    // Plot runtime graph
                    canvas->cd(2);
                    TGraph *graphRun = new TGraph(nData, &dLvec[0], &runValues[0]);
                    graphRun->SetLineColor(kBlack);
                    graphRun->SetLineWidth(2);
                    graphRun->SetTitle("");
                    graphRun->GetXaxis()->SetTitle("dL (mm)");
                    graphRun->GetYaxis()->SetTitle("Tiempo Computacional (s)");
                    graphRun->GetXaxis()->SetRange(dLinitial, dLfinal);
                    SetYRange(graphRun,0.05);
                    graphRun->GetXaxis()->SetTitleSize(0.03);
                    graphRun->GetXaxis()->SetTitleFont(40);
                    graphRun->GetXaxis()->SetLabelSize(0.025);
                    graphRun->GetXaxis()->SetLabelFont(40);
                    graphRun->GetYaxis()->SetTitleSize(0.03);
                    graphRun->GetYaxis()->SetTitleFont(40);
                    graphRun->GetYaxis()->SetLabelSize(0.025);
                    graphRun->GetYaxis()->SetLabelFont(40);
                    graphRun->Draw("ACP");

                    // Update canvas
                    canvas->Update();

                    // Save canvas if required
                    if(kSave){
                        std::string folder = "Standard_Integral_Analysis2/";
                        std::ostringstream ossMass;
                        ossMass << std::fixed << std::setprecision(2) << ma;
                        if (!std::filesystem::exists(folder)) {
                            std::filesystem::create_directory(folder);
                        }
                        canvas->SaveAs((folder + fieldName + "_Analysis_Standard_Mass_" + ossMass.str() + ".pdf").c_str());
                    }

                    // Delete objects
                    delete canvas;
                    delete graphProb;
                    delete graphRun;

                    j++;
                }
                else{   
                    // Create canvas
                    TCanvas *canvas = new TCanvas((fieldName + std::to_string(ma) + "_Analysis").c_str(), (fieldName + std::to_string(ma) + "_Analysis").c_str(), 650, 400);

                    // Plot probability graph
                    canvas->cd(1);
                    TGraph *graphProb = new TGraph(nData, &dLvec[0], &probValues[0]);
                    graphProb->SetLineColor(kBlack);
                    graphProb->SetLineWidth(2);
                    graphProb->SetTitle("");
                    graphProb->GetXaxis()->SetTitle("dL (mm)");
                    graphProb->GetYaxis()->SetTitle("Probabilidad");
                    graphProb->GetXaxis()->SetRange(dLinitial, dLfinal);
                    SetYRange(graphProb, 0.05);
                    if (ma != gas->GetPhotonMass(Ea)){
                        if(ma == 0.1)
                            graphProb->GetYaxis()->SetRangeUser(1e-30, 1e-18);
                        else if(ma == 0.3)
                            graphProb->GetYaxis()->SetRangeUser(1e-32, 1e-25);
                        gPad->SetLogy();
                    }
                    graphProb->GetXaxis()->SetTitleSize(0.03);
                    graphProb->GetXaxis()->SetTitleFont(40);
                    graphProb->GetXaxis()->SetLabelSize(0.025);
                    graphProb->GetXaxis()->SetLabelFont(40);
                    graphProb->GetYaxis()->SetTitleSize(0.03);
                    graphProb->GetYaxis()->SetTitleFont(40);
                    graphProb->GetYaxis()->SetLabelSize(0.025);
                    graphProb->GetYaxis()->SetLabelFont(40);
                    graphProb->Draw("ACP");

                    // Update canvas
                    canvas->Update();

                    // Save canvas if required
                    if(kSave){
                        std::string folder = "Standard_Integral_Analysis2/";
                        std::ostringstream ossMass;
                        ossMass << std::fixed << std::setprecision(2) << ma;
                        if (!std::filesystem::exists(folder)) {
                            std::filesystem::create_directory(folder);
                        }
                        canvas->SaveAs((folder + fieldName + "_Analysis_Standard_Mass_" + ossMass.str() + ".pdf").c_str());
                    }

                    // Delete objects
                    delete canvas;
                    delete graphProb;

                    j++;
                }
            }

            std::string filename;
            std::string folder = "Standard_Integral_Analysis2/";
            std::ostringstream ossMass;
            ossMass << std::fixed << std::setprecision(2) << ma;
            filename = folder + "REST_AXION_" + fieldName + "_StandardIntegralAnalysis_Mass_" + ossMass.str() + ".txt";   

            std::ofstream outputFile(filename);
            if (!outputFile.is_open()) {
                std::cerr << "Error: Unable to open the file for writing!" << std::endl;
                return 1;
            }

            outputFile << "dL" << "\t" << "Probabilidad" << "\t" << "Tiempo (s)" << std::endl;
            for(size_t i = 0; i<nData; i++){
                outputFile << dLvec[i] << "\t" << probValues[i] << "\t" << runValues[i] << std::endl;
            }
            outputFile.close();

        }     
    }
    return 0;
}

void SetYRange(TGraph* graph, Double_t percentage = 0.1) {
    if (graph) {
        TAxis* yAxis = graph->GetYaxis();
        if (yAxis) {
            Double_t minY = DBL_MAX;
            Double_t maxY = -DBL_MAX;
            Double_t *yValues = graph->GetY();
            if (yValues) {
                for (Int_t i = 0; i < graph->GetN(); ++i) {
                    if (yValues[i] < minY) minY = yValues[i];
                    if (yValues[i] > maxY) maxY = yValues[i];
                }

                // Calculate the range increase based on the percentage
                Double_t rangeIncrease = (maxY - minY) * percentage;
                minY -= rangeIncrease;
                maxY += rangeIncrease;

                yAxis->SetRangeUser(minY, maxY);
            }
        }
    }
}