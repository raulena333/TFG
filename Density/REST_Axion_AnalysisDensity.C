#include <iostream>
#include <vector>
#include <chrono>
#include <TLatex.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"
#include "TCanvas.h"
#include "TGraph.h"

//*******************************************************************************************************
//*** Description:
//*** This macro focuses on analyzing the significance of creating and defining a buffer gas composed of helium (He). 
//*** The main function, REST_Axion_AnalysisDensityCopy, conducts an in-depth analysis of axion field densities. 
//*** It calculates transmission probabilities and computation times for different scenarios, shedding light on the 
//*** behavior of axion fields in the presence of helium buffer gas. The analysis involves functions such as 
//*** GenerateDensityValues, ComputeTransmissionAndComputationTime, and CreateGraphsAndPushToVectors.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points to generate for density analysis. (default is 100).
//*** - Ea: Energy of the axion (eV). (default is 4.2).
//*** - gasName: Name of the buffer gas. (default is "He") (helium).
//*** - maxD: Maximum density value to consider in the analysis in kg/mm^3. (default is 1e-9).
//*** - minD: Minimum density value to consider in the analysis in kg/mm^3. (default is 1e-11).
//*** - dL: Length of the path in mm. (efault is 10.0).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath` and `
//*** TRestAxionMagneticField::SetTrack`, `TRestAxionField::GammaTransmissionProbability` and
//*** 'TRestAxionField::GammaTransmissionFieldMapProbability' and `TRestAxionBufferGas::GetPhotonMass` for resonances.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

// Function Declarations

// Generates density values within a specified range.
void GenerateDensityValues(Double_t minD, Double_t maxD, Int_t nData, std::vector<Double_t>& density);

// Computes transmission probabilities and computation times for axion fields.
void ComputeTransmissionAndComputationTime(TRestAxionField* axionField, const std::vector<Double_t>& magneticValuesStandard, Double_t Ea, Double_t ma, Double_t dL,
                                           std::vector<Double_t>& computationTimeStandard, std::vector<Double_t>& transmissionProbabilityStandard,
                                           std::vector<Double_t>& computationTimeGSL, std::vector<Double_t>& transmissionProbabilityGSL,
                                           std::vector<Double_t>& errorProbabilityGS, Bool_t fDebug);

// Creates TGraph objects and pushes them to vectors for further analysis.
void CreateGraphsAndPushToVectors(const std::vector<Double_t>& density, const std::vector<Double_t>& transmissionProbabilityStandard,
                                  std::vector<TGraph*>& TransmissionProbabilityvsDensityStandard, const std::vector<Double_t>& transmissionProbabilityGSL,
                                  std::vector<TGraph*>& TransmissionProbabilityvsDensityGSL, const std::vector<Double_t>& computationTimeStandard,
                                  std::vector<TGraph*>& ComputationTimevsDensityStandard, const std::vector<Double_t>& computationTimeGSL,
                                  std::vector<TGraph*>& ComputationTimevsDensityGSL);

// Analyzes axion field densities with default arguments.
Int_t REST_Axion_AnalysisDensity(Int_t nData = 150, Double_t Ea = 4.2, std::string gasName = "He", Double_t maxD = 1e-9,
                                     Double_t minD = 1e-11, Double_t dL = 10.0) {
    const bool fDebug = true;
    const bool fPlot = true;
    const bool fSave = true;

    TVector3 startPoint(21, 18, -7000);
    TVector3 endPoint(22, 0, 7000);
    TVector3 direction = (startPoint - endPoint).Unit();

    const std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};
    std::vector<Double_t> density;
    std::vector<Double_t> axionMass;
    std::vector<TGraph*> TransmissionProbabilityvsDensityStandard;
    std::vector<TGraph*> TransmissionProbabilityvsDensityGSL;
    std::vector<TGraph*> ComputationTimevsDensityStandard;
    std::vector<TGraph*> ComputationTimevsDensityGSL;

    density.reserve(nData);
    axionMass.reserve(nData);

    GenerateDensityValues(minD, maxD, nData, density);

    bool firstFieldName = true;

    for (const auto& fieldName : fieldNames) {
        auto field = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        std::vector<Double_t> magneticValuesStandard = field->GetTransversalComponentAlongPath(startPoint, endPoint, dL);
        field->SetTrack(startPoint, direction);

        std::vector<Double_t> computationTimeStandard, transmissionProbabilityStandard;
        std::vector<Double_t> computationTimeGSL, transmissionProbabilityGSL, errorProbabilityGSL;

        auto axionField = std::make_unique<TRestAxionField>();
        axionField->AssignMagneticField(field.get());

        for (const auto& value : density) {
            auto gas = std::make_unique<TRestAxionBufferGas>();
            gas->SetGasDensity(gasName, value);
            axionField->AssignBufferGas(gas.get());

            Double_t ma = gas->GetPhotonMass(Ea);
            if (firstFieldName) axionMass.push_back(ma);

            if(fDebug)
                std::cout << "Density Value: " << value << ", Axion Mass: " << ma << std::endl;
            ComputeTransmissionAndComputationTime(axionField.get(), magneticValuesStandard, Ea, ma, dL, computationTimeStandard,
                                                   transmissionProbabilityStandard, computationTimeGSL, transmissionProbabilityGSL, errorProbabilityGSL, fDebug);
            if(fDebug)
                std::cout << "+--------------------------------------------------------------------------+" << std::endl;

        }
        CreateGraphsAndPushToVectors(density, transmissionProbabilityStandard, TransmissionProbabilityvsDensityStandard,
                                        transmissionProbabilityGSL, TransmissionProbabilityvsDensityGSL,
                                        computationTimeStandard, ComputationTimevsDensityStandard, computationTimeGSL,
                                        ComputationTimevsDensityGSL);
        std::cout << ComputationTimevsDensityGSL.size() << std::endl;
        if(fDebug)
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;                            

        firstFieldName = false;
    }

    if (fPlot) {
        // Plot Axion Mass vs. Density
        TCanvas* canvasAxionMass = new TCanvas("canvas", "Axion Mass vs. Density", 800, 600);
        auto graph = std::make_unique<TGraph>(axionMass.size(), &density[0], &axionMass[0]);
        graph->SetTitle("Axion Mass vs. Density on Resonance");
        graph->GetXaxis()->SetTitle("Density (kg/mm3)");
        graph->GetYaxis()->SetTitle("Axion Mass (eV)");
        graph->GetYaxis()->SetTitleOffset(1.2); 
        graph->GetXaxis()->SetTitleSize(0.03); 
        graph->GetYaxis()->SetTitleSize(0.03); 
        graph->GetYaxis()->SetLabelSize(0.03); 
        graph->GetXaxis()->SetLabelSize(0.03);
        graph->SetLineWidth(2);
        graph->SetMarkerColor(kBlack);
        graph->Draw("ACP");
        if (fSave) canvasAxionMass->SaveAs("AxionMass_vs_Density.png");

       // Plot Transmission Probability vs. Density
        TCanvas* canvasTransmission = new TCanvas("canvasTransmission", "Transmission Probability vs. Density", 800, 600);
        canvasTransmission->Divide(2, 1);

        canvasTransmission->cd(1);
        TMultiGraph* mgTransmissionStandard = new TMultiGraph();
        TransmissionProbabilityvsDensityStandard[0]->SetLineColor(kOrange - 7);
        TransmissionProbabilityvsDensityStandard[0]->SetLineWidth(2); 
        mgTransmissionStandard->Add(TransmissionProbabilityvsDensityStandard[0]);
        TransmissionProbabilityvsDensityStandard[1]->SetLineColor(kGreen + 3);
        TransmissionProbabilityvsDensityStandard[1]->SetLineWidth(2); 
        mgTransmissionStandard->Add(TransmissionProbabilityvsDensityStandard[1]);
        mgTransmissionStandard->Draw("ACP");
        mgTransmissionStandard->SetTitle("Standard Integration");
        mgTransmissionStandard->GetXaxis()->SetTitle("Density (kg/mm3)");
        mgTransmissionStandard->GetYaxis()->SetTitle("Transmission Probability");
        mgTransmissionStandard->GetXaxis()->SetTitleSize(0.03);
        mgTransmissionStandard->GetYaxis()->SetTitleSize(0.03); 
        mgTransmissionStandard->GetYaxis()->SetLabelSize(0.02); 
        mgTransmissionStandard->GetXaxis()->SetLabelSize(0.02);

        // // Add legend
        // TLegend* legendTransmission = new TLegend(0.9, 0.9, 0.99, 0.99);
        // legendTransmission->AddEntry(TransmissionProbabilityvsDensityStandard[0], "babyIAXO_cutoff_2024", "l");
        // legendTransmission->AddEntry(TransmissionProbabilityvsDensityStandard[1], "babyIAXO_2024", "l");
        // legendTransmission->Draw();

        canvasTransmission->cd(2);
        TMultiGraph* mgTransmissionGSL = new TMultiGraph();
        TransmissionProbabilityvsDensityGSL[0]->SetLineColor(kOrange - 7);
        TransmissionProbabilityvsDensityGSL[0]->SetLineWidth(2);
        mgTransmissionGSL->Add(TransmissionProbabilityvsDensityGSL[0]);
        TransmissionProbabilityvsDensityGSL[1]->SetLineColor(kGreen + 3);
        TransmissionProbabilityvsDensityGSL[1]->SetLineWidth(2);
        mgTransmissionGSL->Add(TransmissionProbabilityvsDensityGSL[1]);
        mgTransmissionGSL->Draw("ACP");
        mgTransmissionGSL->SetTitle("GSL Integration");
        mgTransmissionGSL->GetXaxis()->SetTitle("Density (kg/mm3)");
        mgTransmissionGSL->GetYaxis()->SetTitle("Transmission Probability");
        mgTransmissionGSL->GetXaxis()->SetTitleSize(0.03);
        mgTransmissionGSL->GetYaxis()->SetTitleSize(0.03);
        mgTransmissionGSL->GetYaxis()->SetLabelSize(0.02); 
        mgTransmissionGSL->GetXaxis()->SetLabelSize(0.02);

        // // Add legend
        // TLegend* legendTransmission2 = new TLegend(0.8, 0.8, 0.9, 0.9);
        // legendTransmission2->AddEntry(TransmissionProbabilityvsDensityGSL[0], "babyIAXO_cutoff_2024", "l");
        // legendTransmission2->AddEntry(TransmissionProbabilityvsDensityGSL[1], "babyIAXO_2024", "l");
        // legendTransmission2->Draw();

        if (fSave) canvasTransmission->SaveAs("TransmissionProbability_vs_Density.png");

        // Plot Computation Time vs. Density
        TCanvas* canvasComputationTime = new TCanvas("canvasComputationTime", "Computation Time vs. Density", 800, 600);
        canvasComputationTime->Divide(2, 1);

        canvasComputationTime->cd(1);
        TMultiGraph* mgComputationStandard = new TMultiGraph();
        ComputationTimevsDensityStandard[0]->SetLineColor(kOrange - 7);
        ComputationTimevsDensityStandard[0]->SetLineWidth(2);
        mgComputationStandard->Add(ComputationTimevsDensityStandard[0]);
        ComputationTimevsDensityStandard[1]->SetLineColor(kGreen + 3);
        ComputationTimevsDensityStandard[1]->SetLineWidth(2);
        mgComputationStandard->Add(ComputationTimevsDensityStandard[1]);
        mgComputationStandard->Draw("ACP");
        mgComputationStandard->SetTitle("Standard Integration");
        mgComputationStandard->GetXaxis()->SetTitle("Density (kg/mm3)");
        mgComputationStandard->GetYaxis()->SetTitle("Computation Time (μs)");
        mgComputationStandard->GetXaxis()->SetTitleSize(0.03); 
        mgComputationStandard->GetYaxis()->SetTitleSize(0.03);
        mgComputationStandard->GetYaxis()->SetLabelSize(0.02);
        mgComputationStandard->GetXaxis()->SetLabelSize(0.02);

        // // Add legend
        // TLegend* legendComputation = new TLegend(0.8, 0.8, 0.9, 0.9);
        // legendComputation->AddEntry(ComputationTimevsDensityStandard[0], "babyIAXO_cutoff_2024", "l");
        // legendComputation->AddEntry(ComputationTimevsDensityStandard[1], "babyIAXO_2024", "l");
        // legendComputation->Draw();

        canvasComputationTime->cd(2);
        TMultiGraph* mgComputationGSL = new TMultiGraph();
        ComputationTimevsDensityGSL[0]->SetLineColor(kOrange - 7);
        ComputationTimevsDensityGSL[0]->SetLineWidth(2);
        mgComputationGSL->Add(ComputationTimevsDensityGSL[0]);
        ComputationTimevsDensityGSL[1]->SetLineColor(kGreen + 3);
        ComputationTimevsDensityGSL[1]->SetLineWidth(2);
        mgComputationGSL->Add(ComputationTimevsDensityGSL[1]);
        mgComputationGSL->Draw("ACP");
        mgComputationGSL->SetTitle("GSL Integration");
        mgComputationGSL->GetXaxis()->SetTitle("Density (kg/mm3)");
        mgComputationGSL->GetYaxis()->SetTitle("Computation Time (ms)");
        mgComputationGSL->GetXaxis()->SetTitleSize(0.03); 
        mgComputationGSL->GetYaxis()->SetTitleSize(0.03); 
        mgComputationGSL->GetYaxis()->SetLabelSize(0.02);
        mgComputationGSL->GetXaxis()->SetLabelSize(0.02);

        // // Add legend
        // TLegend* legendComputation2 = new TLegend(0.8, 0.8, 0.9, 0.9);
        // legendComputation2->AddEntry(ComputationTimevsDensityGSL[0], "babyIAXO_cutoff_2024", "l");
        // legendComputation2->AddEntry(ComputationTimevsDensityGSL[1], "babyIAXO_2024", "l");
        // legendComputation2->Draw();

        if (fSave) canvasComputationTime->SaveAs("ComputationTime_vs_Density.png");

        if(fDebug && fSave){
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << "Canvas created and saved successfully. " << std::endl;
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
        }
        else if(fDebug && !fSave){
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
            std::cout << "Canvas created successfully. " << std::endl;
            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
        }
    }

    return 0;
}

void GenerateDensityValues(Double_t minD, Double_t maxD, Int_t nData, std::vector<Double_t>& density) {
    Double_t stepD = (maxD - minD) / nData;
    for (Int_t j = 0; j < nData; j++) {
        density.push_back(minD + stepD * j);
    }
}

void ComputeTransmissionAndComputationTime(TRestAxionField* axionField, const std::vector<Double_t>& magneticValuesStandard, Double_t Ea, Double_t ma, Double_t dL,
                                           std::vector<Double_t>& computationTimeStandard, std::vector<Double_t>& transmissionProbabilityStandard,
                                           std::vector<Double_t>& computationTimeGSL, std::vector<Double_t>& transmissionProbabilityGSL,
                                           std::vector<Double_t>& errorProbabilityGSL, Bool_t fDebug) {
    auto start_time_standard = std::chrono::high_resolution_clock::now();
    Double_t probFieldStandard = axionField->GammaTransmissionProbability(magneticValuesStandard, dL, Ea, ma);
    auto end_time_standard = std::chrono::high_resolution_clock::now();
    auto duration_standard = std::chrono::duration_cast<std::chrono::microseconds>(end_time_standard - start_time_standard);

    transmissionProbabilityStandard.push_back(probFieldStandard);
    computationTimeStandard.push_back(duration_standard.count());

    auto start_time_GSL = std::chrono::high_resolution_clock::now();
    std::pair<Double_t, Double_t> probFieldGSL = axionField->GammaTransmissionFieldMapProbability(Ea, ma, 0.1, 100, 20);
    auto end_time_GSL = std::chrono::high_resolution_clock::now();
    auto duration_GSL = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_GSL - start_time_GSL);

    transmissionProbabilityGSL.push_back(probFieldGSL.first);
    computationTimeGSL.push_back(duration_GSL.count());
    errorProbabilityGSL.push_back(probFieldGSL.second);

    if (fDebug) {
        std::cout << "Standard Integral - Probability: " << probFieldStandard << ", Runtime: " << duration_standard.count() << " μs" << std::endl;
        std::cout << "GSL Integral - Probability: " << probFieldGSL.first << "+-" << probFieldGSL.second << ", Runtime: " << duration_GSL.count() << " ms" << std::endl;
        std::cout << std::endl;
    }
}

void CreateGraphsAndPushToVectors(const std::vector<Double_t>& density, const std::vector<Double_t>& transmissionProbabilityStandard,
                                  std::vector<TGraph*>& TransmissionProbabilityvsDensityStandard, const std::vector<Double_t>& transmissionProbabilityGSL,
                                  std::vector<TGraph*>& TransmissionProbabilityvsDensityGSL, const std::vector<Double_t>& computationTimeStandard,
                                  std::vector<TGraph*>& ComputationTimevsDensityStandard, const std::vector<Double_t>& computationTimeGSL,
                                  std::vector<TGraph*>& ComputationTimevsDensityGSL) {
    Int_t nData = density.size();                                
    auto transmissionGraphStandard = std::make_unique<TGraph>(nData, &density[0], &transmissionProbabilityStandard[0]);
    auto transmissionGraphGSL = std::make_unique<TGraph>(nData, &density[0], &transmissionProbabilityGSL[0]);
    auto computationTimeGraphStandard = std::make_unique<TGraph>(nData, &density[0], &computationTimeStandard[0]);
    auto computationTimeGraphGSL = std::make_unique<TGraph>(nData, &density[0], &computationTimeGSL[0]);

    TransmissionProbabilityvsDensityStandard.push_back(transmissionGraphStandard.release());
    TransmissionProbabilityvsDensityGSL.push_back(transmissionGraphGSL.release());
    ComputationTimevsDensityStandard.push_back(computationTimeGraphStandard.release());
    ComputationTimevsDensityGSL.push_back(computationTimeGraphGSL.release());
}

