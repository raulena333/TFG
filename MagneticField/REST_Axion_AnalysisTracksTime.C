#include <iostream>
#include <chrono>
#include <vector>
#include <TCanvas.h>
#include <TH2D.h>
#include <TMultiGraph.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TRandom3.h>

#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

//*******************************************************************************************************
//*** Description:
//*** This function performs analysis on tracks of axions under varying conditions and generates heatmaps
//*** to visualize the runtime of the analysis. It simulates the trajectory of axions under magnetic fields
//*** and calculates the runtime required for each trajectory and its probability.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points (axionMass) to generate (default: 100).
//***          Specifies the number of data points to simulate along the trajectory.
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - dMax: Maximum value for the x and y coordinates (default: 50).
//***         Specifies the maximum value for the x and y coordinates of the trajectory.
//*** - dMin: Minimum value for the x and y coordinates (default: -50).
//***         Specifies the minimum value for the x and y coordinates of the trajectory.
//*** - dL: The differential element in mm (default: 10).
//***        Specifies the size of the differential element used in the simulation.
//*** - gasName: Gas name (default: "He").
//*** - nTracks: number of tracks to plot (default: 5).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`.
//***
//*** Author: Raul Ena
//*******************************************************************************************************

// Constants
constexpr bool kDebug = true;
constexpr bool kSave = true;
constexpr bool kPlot = true;
constexpr int kNumBins = 100; // Number of bins for histograms

// Function to select randomly nTracks of dx and dy
void selectDxy(const std::vector<Double_t>& dx, const std::vector<Double_t>& dy, Int_t nTracks, std::vector<Double_t>& selectedDx, std::vector<Double_t>& selectedDy);

Int_t REST_Axion_AnalysisTracksTime(Double_t nData = 3, Double_t Ea = 4.2, Double_t dMax = 50, Double_t dMin = -50,
                       Double_t dL = 10, const std::string& gasName = "He", Double_t nTracks = 2) {

    const TVector3 startPoint(0, 0, -7000);
    const Double_t gasDensity = 9.345e-10;
    std::vector<Double_t> dx, dy, selectedDx, selectedDy;

    // Create TRestAxionField and TRestAxionBufferGas instances
    auto axionField = std::make_unique<TRestAxionField>();
    std::unique_ptr<TRestAxionBufferGas> gas = nullptr;

    if (!gasName.empty()) {
        gas = std::make_unique<TRestAxionBufferGas>();
        gas->SetGasDensity(gasName, gasDensity);
        axionField->AssignBufferGas(gas.get());
    }

    // Mass On Resonance
    Double_t axionMass = (gas != nullptr) ? gas->GetPhotonMass(Ea) : 0;

    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff", "babyIAXO_2024"};

    // Fill in the values for later choose them randmoly
    for(size_t k = 0; k < nData; k++){
        dx.push_back(dMin + k * (dMax - dMin) / nData);
        dy.push_back(dMin + k * (dMax - dMin) / nData);
    }

    if(nTracks <= 0 || nTracks > dx.size()) {
        std::cerr << "ERROR: Invalid number of selected tracks." << std::endl;
        return -1;
    }
    
    selectDxy(dx ,dy , nTracks, selectedDx, selectedDy);

    for (const auto& fieldName : fieldNames) {
        auto field = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        axionField->AssignMagneticField(field.get());

        auto canvasHeatMapProbGSL = std::make_unique<TCanvas>((fieldName + "_Probability_HeatmapsGSL").c_str(), (fieldName + " Probability HeatmapsGSL").c_str(), 800, 600);
        auto canvasHeatMapRunTimeGSL = std::make_unique<TCanvas>((fieldName + "_Runtime_HeatmapsGSL").c_str(), (fieldName + " Runtime Heatmaps").c_str(), 800, 600);
        auto canvasHeatMapProbStandard = std::make_unique<TCanvas>((fieldName + "_Probability_HeatmapsStandard").c_str(), (fieldName + " Probability HeatmapsStandard").c_str(), 800, 600);
        auto canvasHeatMapRunTimeStandard = std::make_unique<TCanvas>((fieldName + "_Runtime_HeatmapsStandard").c_str(), (fieldName + " Runtime HeatmapsStandard").c_str(), 800, 600);
    
        auto heatmapProbStandard = std::make_unique<TH2D>(("ProbabilityStandard_" + fieldName).c_str(), (fieldName + " Heatmap Probability Standard").c_str(), kNumBins, dMin, dMax, kNumBins, dMin, dMax);
        auto heatmapProbGSL = std::make_unique<TH2D>(("ProbabilityGSL_" + fieldName).c_str(), (fieldName + " Heatmap Probability GSL").c_str(), kNumBins, dMin, dMax, kNumBins, dMin, dMax);
        auto heatmapRuntimeStandard = std::make_unique<TH2D>(("RuntimeStandard_" + fieldName).c_str(), (fieldName + " Heatmap Runtime Standard").c_str(), kNumBins, dMin, dMax, kNumBins, dMin, dMax);
        auto heatmapRuntimeGSL = std::make_unique<TH2D>(("RuntimeGSL_" + fieldName).c_str(), (fieldName + " Heatmap Runtime GSL").c_str(), kNumBins, dMin, dMax, kNumBins, dMin, dMax);

        // Vector to hold probabilities for each selected dx
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDxProbabilitiesGSL(selectedDx.size());
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDxProbabilitiesStandard(selectedDx.size());
        // Vector to hold probabilities for each selected dy
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDyProbabilitiesGSL(selectedDy.size());
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDyProbabilitiesStandard(selectedDy.size());

        for (size_t i = 0; i < nData; ++i) {
            Double_t xEnd = dx[i];
            for (size_t j = 0; j < nData; ++j) {
                Double_t yEnd = dy[j];
                TVector3 endPoint(xEnd, yEnd, 7000);

                auto start_time_standard = std::chrono::high_resolution_clock::now();
                std::vector<Double_t> magneticValues_standard = field->GetTransversalComponentAlongPath(startPoint, endPoint, dL);
                auto end_time_standard = std::chrono::high_resolution_clock::now();
                auto duration_standard = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_standard - start_time_standard);

                Double_t probStandard = axionField->GammaTransmissionProbability(magneticValues_standard, dL, Ea, axionMass);

                if (kDebug) {
                    std::cout << "Standard Integration" << std::endl;
                    std::cout << "Time: " << duration_standard.count() << " ms" << std::endl;
                    std::cout << "endPoint: (" << xEnd << "," << yEnd << ",7000)" << std::endl;
                    std::cout << "Probability: " << probStandard << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                }

                TVector3 direction = (startPoint - endPoint).Unit();
                auto start_time_gsl = std::chrono::high_resolution_clock::now();
                field->SetTrack(startPoint, direction);
                auto end_time_gsl = std::chrono::high_resolution_clock::now();
                auto duration_gsl = std::chrono::duration_cast<std::chrono::microseconds>(end_time_gsl - start_time_gsl);

                std::pair<Double_t, Double_t> probGSL = axionField->GammaTransmissionFieldMapProbability(Ea, axionMass, 0.1, 100, 20);

                if (kDebug) {
                    std::cout << "GSL Integration" << std::endl;
                    std::cout << "Time: " << duration_gsl.count() << " μs" << std::endl;
                    std::cout << "Direction (" << direction.X() << "," << direction.Y() << "," << direction.Z() << ")" << std::endl;
                    std::cout << "Probability: " << probGSL.first << "+-" << probGSL.second << std::endl;
                    std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                }

                // Check if this dx matches any selected dx
                for (size_t k = 0; k < selectedDx.size(); ++k) {
                    if (xEnd == selectedDx[k]) {
                        selectedDxProbabilitiesGSL[k].first.push_back(probGSL.first);
                        selectedDxProbabilitiesStandard[k].first.push_back(probStandard);
                        selectedDxProbabilitiesGSL[k].second = xEnd;
                        selectedDxProbabilitiesStandard[k].second = xEnd; 
                        break; 
                    }
                }

                // Check if this dy matches any selected dy
                for (size_t k = 0; k < selectedDy.size(); ++k) {
                    if (yEnd == selectedDy[k]) {
                        selectedDyProbabilitiesGSL[k].first.push_back(probGSL.first);
                        selectedDyProbabilitiesStandard[k].first.push_back(probStandard);
                        selectedDyProbabilitiesGSL[k].second = yEnd;
                        selectedDyProbabilitiesStandard[k].second = yEnd;
                        break;
                    }
                }

                heatmapRuntimeStandard->Fill(xEnd, yEnd, duration_standard.count());
                heatmapRuntimeGSL->Fill(xEnd, yEnd, duration_gsl.count());
                heatmapProbStandard->Fill(xEnd, yEnd, probStandard);
                heatmapProbGSL->Fill(xEnd, yEnd, probGSL.first);
            }
        }  

        canvasHeatMapProbStandard->cd();
        heatmapProbStandard->Draw("colz");
        canvasHeatMapProbGSL->cd();
        heatmapProbGSL->Draw("colz");

        canvasHeatMapRunTimeStandard->cd();
        heatmapRuntimeStandard->Draw("colz");
        canvasHeatMapRunTimeGSL->cd();
        heatmapRuntimeGSL->Draw("colz");

        if (kSave) {
            std::string imageNameProbS = fieldName + "_Probability_HeatmapsStandard.png";
            std::string imageNameProbG = fieldName + "_Probability_HeatmapsGSL.png";
            std::string imageNameRunTimeS = fieldName + "_Runtime_HeatmapsStandard.png";
            std::string imageNameRunTimeG = fieldName + "_Runtime_HeatmapsGSL.png";
            canvasHeatMapProbStandard->SaveAs(imageNameProbS.c_str());
            canvasHeatMapProbGSL->SaveAs(imageNameProbG.c_str());
            canvasHeatMapRunTimeStandard->SaveAs(imageNameRunTimeS.c_str());
            canvasHeatMapRunTimeGSL->SaveAs(imageNameRunTimeG.c_str());
        }

        if (kPlot) {
            std::unique_ptr<TCanvas> canvas(new TCanvas((fieldName + "_canvas").c_str(), (fieldName + " Probability and Time Plots").c_str(), 800, 600));
            canvas->Divide(2, 2);

            // Pad 1 for selectedDxProbabilitiesGSL
            canvas->cd(1);
            std::unique_ptr<TMultiGraph> mg_dx_prob_gsl(new TMultiGraph());
            mg_dx_prob_gsl->SetTitle("Probability vs. dy for selected dx (GSL)");
            mg_dx_prob_gsl->GetXaxis()->SetTitle("dy (mm)");
            mg_dx_prob_gsl->GetYaxis()->SetTitle("Probability");
            std::unique_ptr<TLegend> legend_dx_prob_gsl(new TLegend(0.1, 0.7, 0.48, 0.9));
            for (size_t i = 0; i < selectedDxProbabilitiesGSL.size(); ++i) {
                auto& probabilities = selectedDxProbabilitiesGSL[i].first;
                auto xEnd = selectedDxProbabilitiesGSL[i].second;
                std::unique_ptr<TGraph> graph(new TGraph(probabilities.size(), dy.data(), probabilities.data()));
                graph->SetMarkerStyle(20 + i);
                graph->SetMarkerColor(i + 1);
                mg_dx_prob_gsl->Add(graph.get());
                legend_dx_prob_gsl->AddEntry(graph.get(), ("Dx = " + std::to_string(xEnd)).c_str(), "lp");
            }
            mg_dx_prob_gsl->Draw("AP");
            legend_dx_prob_gsl->Draw();

            // Pad 2 for selectedDxProbabilitiesStandard
            canvas->cd(2);
            std::unique_ptr<TMultiGraph> mg_dx_prob_standard(new TMultiGraph());
            mg_dx_prob_standard->SetTitle("Probability vs. dy for selected dx (Standard)");
            mg_dx_prob_standard->GetXaxis()->SetTitle("dy (mm)");
            mg_dx_prob_standard->GetYaxis()->SetTitle("Probability");
            std::unique_ptr<TLegend> legend_dx_prob_standard(new TLegend(0.1, 0.7, 0.48, 0.9));
            for (size_t i = 0; i < selectedDxProbabilitiesStandard.size(); ++i) {
                auto& probabilities = selectedDxProbabilitiesStandard[i].first;
                auto xEnd = selectedDxProbabilitiesStandard[i].second;
                std::unique_ptr<TGraph> graph(new TGraph(probabilities.size(), dy.data(), probabilities.data()));
                graph->SetMarkerStyle(20 + i);
                graph->SetMarkerColor(i + 1);
                mg_dx_prob_standard->Add(graph.get());
                legend_dx_prob_standard->AddEntry(graph.get(), ("Dx = " + std::to_string(xEnd)).c_str(), "lp");
            }
            mg_dx_prob_standard->Draw("AP");
            legend_dx_prob_standard->Draw();

            // Pad 3 for selectedDyProbabilitiesGSL
            canvas->cd(3);
            std::unique_ptr<TMultiGraph> mg_dy_prob_gsl(new TMultiGraph());
            mg_dy_prob_gsl->SetTitle("Probability vs. dx for selected dy (GSL)");
            mg_dy_prob_gsl->GetXaxis()->SetTitle("dx (mm)");
            mg_dy_prob_gsl->GetYaxis()->SetTitle("Probability");
            std::unique_ptr<TLegend> legend_dy_prob_gsl(new TLegend(0.1, 0.7, 0.48, 0.9));
            for (size_t i = 0; i < selectedDyProbabilitiesGSL.size(); ++i) {
                auto& probabilities = selectedDyProbabilitiesGSL[i].first;
                auto yEnd = selectedDyProbabilitiesGSL[i].second;
                std::unique_ptr<TGraph> graph(new TGraph(probabilities.size(), dx.data(), probabilities.data()));
                graph->SetMarkerStyle(20 + i);
                graph->SetMarkerColor(i + 1);
                mg_dy_prob_gsl->Add(graph.get());
                legend_dy_prob_gsl->AddEntry(graph.get(), ("Dy = " + std::to_string(yEnd)).c_str(), "lp");
            }
            mg_dy_prob_gsl->Draw("AP");
            legend_dy_prob_gsl->Draw();

            // Pad 4 for selectedDyProbabilitiesStandard
            canvas->cd(4);
            std::unique_ptr<TMultiGraph> mg_dy_prob_standard(new TMultiGraph());
            mg_dy_prob_standard->SetTitle("Probability vs. dx for selected dy (Standard)");
            mg_dy_prob_standard->GetXaxis()->SetTitle("dx (mm)");
            mg_dy_prob_standard->GetYaxis()->SetTitle("Probability");
            std::unique_ptr<TLegend> legend_dy_prob_standard(new TLegend(0.1, 0.7, 0.48, 0.9));
            for (size_t i = 0; i < selectedDyProbabilitiesStandard.size(); ++i) {
                auto& probabilities = selectedDyProbabilitiesStandard[i].first;
                auto yEnd = selectedDyProbabilitiesStandard[i].second;
                std::unique_ptr<TGraph> graph(new TGraph(probabilities.size(), dx.data(), probabilities.data()));
                graph->SetMarkerStyle(20 + i);
                graph->SetMarkerColor(i + 1);
                mg_dy_prob_standard->Add(graph.get());
                legend_dy_prob_standard->AddEntry(graph.get(), ("Dy = " + std::to_string(yEnd)).c_str(), "lp");
            }
            mg_dy_prob_standard->Draw("AP");
            legend_dy_prob_standard->Draw();
            canvas->Draw();

            if (kSave) {
                canvas->SaveAs("Probability_and_Time_Plots.png");
            }
        }


    }
    return 0;
}

void selectDxy(const std::vector<Double_t>& dx, const std::vector<Double_t>& dy, Int_t numSelectedTracks, std::vector<Double_t>& selectedDx, std::vector<Double_t>& selectedDy) {
    // Check if numSelectedTracks is valid
    if (numSelectedTracks <= 0 || numSelectedTracks > dx.size()) {
        std::cerr << "ERROR: Invalid number of selected tracks." << std::endl;
        return;
    }

    // Initialize indices vector
    std::vector<Int_t> indices(dx.size());
    for (int i = 0; i < dx.size(); ++i) {
        indices[i] = i;
    }

    // Shuffle indices vector randomly
    TRandom3 rng;
    std::shuffle(indices.begin(), indices.end(), std::default_random_engine(std::random_device()()));

    // Select first numSelectedTracks indices
    selectedDx.resize(numSelectedTracks);
    selectedDy.resize(numSelectedTracks);
    for (Int_t i = 0; i < numSelectedTracks; ++i) {
        Int_t index = indices[i];
        selectedDx[i] = dx[index];
        selectedDy[i] = dy[index];
    }
}
