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
//*** This function performs GSL integral analysis for axion experiments. It calculates the runtime, probability, and error values
//*** for different axion masses and integration accuracies. The results are plotted on graphs for visualization.
//*** 
//*** Arguments by default are (in order):
//*** - nData: Number of accuracy points to generate (default: 100).
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - gasName: Name of the buffer gas (default: "He").
//*** - m1: Initial axion mass in eV far from resonance (default: 0.01).
//*** - m3: Middle axion mass in eV far/close to resonance (default: 0.1).
//*** - m2: Final axion mass in eV close to resonance (default: 0.3).
//*** - accuracyInitial: Initial integration accuracy (default: 0.01).
//*** - accuracyFinal: Final integration accuracy (default: 1.).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`,
//*** `TRestAxionField::GammaTransmissionFieldMapProbability`, and `TRestAxionBufferGas::SetGasDensity`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

constexpr bool kDebug = true;
constexpr bool kPlot = true;
constexpr bool kSave = true;

void SetYRange(TGraph* graph, Double_t percentage = 0.1);

Int_t REST_Axion_GSLIntegralAnalysis(Int_t nData = 50, Double_t Ea = 4.2, std::string gasName = "He", Double_t m1 = 0.01, 
                                    Double_t m3 = 0.1, Double_t m2 = 0.3, Double_t accuracyInitial = 0.1, Double_t accuracyFinal = 1.) {

    // Create Variables
    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", //"babyIAXO_2024"
    };
    const Double_t gasDensity = 2.9836e-10;
    const TVector3 initialPosition(-5, 5, -11000);
    const TVector3 direction = (initialPosition - TVector3(5, -5, 11000)).Unit();
    std::vector<Double_t> masses;
    std::vector<Double_t> accuracyValues;

    for (Int_t j = 0; j < nData; j++) {
        Double_t accuracy = accuracyInitial + j * (accuracyFinal - accuracyInitial) / nData;
        accuracyValues.push_back(accuracy);
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

    for (const auto &fieldName : fieldNames) {
        
        // Create instance of magnetic field, axion field and buffer gas and then assign it.
        auto magneticField = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        auto axionField = std::make_unique<TRestAxionField>();

        if (gas != nullptr)
            axionField->AssignBufferGas(gas.get());
        axionField->AssignMagneticField(magneticField.get());
        magneticField->SetTrack(initialPosition, direction);

        for (const auto &ma : masses) {
            if (kDebug) {
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << "Axion Mass: " << ma << std::endl;
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                std::cout << std::endl;
            }
            
            std::vector<Double_t> probValues, errorValues, runValues;

            for (const auto &accuracy : accuracyValues) {
                if (kDebug) {
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << "Accuracy value " << accuracy << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                    std::cout << std::endl;
                }

                // GSL Integration
                auto start_time = std::chrono::high_resolution_clock::now();
                std::pair<Double_t, Double_t> probField = axionField->GammaTransmissionFieldMapProbability(Ea, ma, accuracy, 100, 20);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

                probValues.push_back(probField.first);
                errorValues.push_back(probField.second);
                runValues.push_back(duration.count());

                if (kDebug) {
                    std::cout << "Probability: " << probField.first << std::endl;
                    std::cout << "Error: " << probField.second << std::endl;
                    std::cout << "Runtime (ms): " << duration.count() << std::endl;
                    std::cout << std::endl;
                }
            }

           if (kPlot) {
                TCanvas *canvas = new TCanvas((fieldName + "_Analysis" + std::to_string(ma)).c_str(), (fieldName + "_Analysis" + std::to_string(ma)).c_str(), 1200, 400);
                canvas->Divide(3, 1);

                // First pad for the probability integration plot
                canvas->cd(1);
                TGraph *graphProb = new TGraph(nData, &accuracyValues[0], &probValues[0]);
                graphProb->SetTitle("");
                //graphProb->SetMarkerStyle(8); 
                //graphProb->SetMarkerColor(kBlack); 
                //graphProb->SetMarkerSize(0.4);  
                graphProb->SetLineColor(kBlack); 
                graphProb->SetLineWidth(2);
                graphProb->GetXaxis()->SetTitle("Precision");
                graphProb->GetYaxis()->SetTitle("Probabilidad");
                graphProb->GetXaxis()->SetRangeUser(accuracyInitial, accuracyFinal);
                SetYRange(graphProb, 0.05);
                graphProb->GetXaxis()->SetTitleSize(0.03); 
                graphProb->GetXaxis()->SetTitleFont(40);  
                graphProb->GetXaxis()->SetLabelSize(0.025); 
                graphProb->GetXaxis()->SetLabelFont(40);  
                graphProb->GetYaxis()->SetTitleSize(0.03); 
                graphProb->GetYaxis()->SetTitleFont(40);  
                graphProb->GetYaxis()->SetLabelSize(0.025); 
                graphProb->GetYaxis()->SetLabelFont(40);
                graphProb->Draw("ACP");

                // Second pad for the error integration plot
                canvas->cd(2);
                TGraph *graphError = new TGraph(nData, &accuracyValues[0], &errorValues[0]);
                graphError->SetTitle("");
                //graphError->SetMarkerStyle(8);
                //graphError->SetMarkerColor(kBlack);  
                //graphError->SetMarkerSize(0.4);  
                graphError->SetLineColor(kBlack); 
                graphError->SetLineWidth(2);
                graphError->GetXaxis()->SetTitle("Precision");
                graphError->GetYaxis()->SetTitle("Error");
                graphError->GetXaxis()->SetRangeUser(accuracyInitial, accuracyFinal);
                SetYRange(graphError, 0.05);
                graphError->GetXaxis()->SetTitleSize(0.03); 
                graphError->GetXaxis()->SetTitleFont(40);  
                graphError->GetXaxis()->SetLabelSize(0.025); 
                graphError->GetXaxis()->SetLabelFont(40);  
                graphError->GetYaxis()->SetTitleSize(0.03); 
                graphError->GetYaxis()->SetTitleFont(40);  
                graphError->GetYaxis()->SetLabelSize(0.025); 
                graphError->GetYaxis()->SetLabelFont(40); 
                graphError->Draw("ACP");

                // Third pad for the runtime plot
                canvas->cd(3);
                TGraph *graphRuntime = new TGraph(nData, &accuracyValues[0], &runValues[0]);
                graphRuntime->SetTitle("");
                //graphRuntime->SetMarkerStyle(8); 
                //graphRuntime->SetMarkerColor(kBlack); 
                //graphRuntime->SetMarkerSize(0.4);  
                graphRuntime->SetLineColor(kBlack); 
                graphRuntime->SetLineWidth(2);
                graphRuntime->GetXaxis()->SetTitle("Precision");
                graphRuntime->GetYaxis()->SetTitle("Tiempo computacional (ms)");
                graphRuntime->GetXaxis()->SetRangeUser(accuracyInitial, accuracyFinal);
                SetYRange(graphRuntime, 0.05);
                graphRuntime->GetXaxis()->SetTitleSize(0.03); 
                graphRuntime->GetXaxis()->SetTitleFont(40);  
                graphRuntime->GetXaxis()->SetLabelSize(0.025); 
                graphRuntime->GetXaxis()->SetLabelFont(40);  
                graphRuntime->GetYaxis()->SetTitleSize(0.03); 
                graphRuntime->GetYaxis()->SetTitleFont(40);  
                graphRuntime->GetYaxis()->SetLabelSize(0.025); 
                graphRuntime->GetYaxis()->SetLabelFont(40); 
                graphRuntime->Draw("ACP");

                canvas->Update();

                // Save the canvas if required
                if (kSave) {
                    std::string folder = "GSL_Integral_Analysis1/";
                    std::ostringstream ossMass;
                    ossMass << std::fixed << std::setprecision(2) << ma;
                    if (!std::filesystem::exists(folder)) {
                        std::filesystem::create_directory(folder);
                    }
                    canvas->SaveAs((folder + fieldName + "_Analysis_GSL_Mass_" + ossMass.str() + ".pdf").c_str());
                }

                delete graphProb;
                delete graphError;
                delete graphRuntime;
                delete canvas;
            }

            std::string filename;
            std::string folder = "GSL_Integral_Analysis1/";
            std::ostringstream ossMass;
            ossMass << std::fixed << std::setprecision(2) << ma;
            filename = folder + "REST_AXION_" + fieldName + "_GSLIntegralAnalysis_Mass_" + ossMass.str() + ".txt";   

            std::ofstream outputFile(filename);
            if (!outputFile.is_open()) {
                std::cerr << "Error: Unable to open the file for writing!" << std::endl;
                return 1;
            }

            outputFile << "Precisión" << "\t" << "Probabilidad" << "\t" << "Error" << "\t" << "Tiempo (ms)" << std::endl;
            for(size_t i = 0; i<nData; i++){
                outputFile << accuracyValues[i] << "\t" << probValues[i] << "\t" << errorValues[i] << "\t" << runValues[i] << std::endl;
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