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
//*** Description:
//*** This function performs analysis on tracks of axions under varying conditions and generates heatmaps
//*** to visualize the runtime of the analysis. It simulates the trajectory of axions under magnetic fields
//*** and calculates the runtime required for each trajectory and its probability.
//***
//*** Arguments by default are (in order):
//*** - nData: Number of data points (axionMass) to generate (default: 100).
//***          Specifies the number of data points to simulate along the trajectory.
//*** - Ea: Axion energy in keV (default: 4.2).
//*** - dMax: Maximum value for the x and y coordinates (default: 20).
//***         Specifies the maximum value for the x and y coordinates of the trajectory.
//*** - dMin: Minimum value for the x and y coordinates (default: -20).
//***         Specifies the minimum value for the x and y coordinates of the trajectory.
//*** - dL: The differential element in mm (default: 10).
//***        Specifies the size of the differential element used in the simulation.
//*** - gasName: Gas name (default: "He").
//*** - nTracks: number of tracks to plot (default: 5).
//***
//*** Dependencies:
//*** The generated data are the results from `TRestAxionMagneticField::GetTransversalComponentAlongPath`.
//*** `TRestAxionMagneticField::SetTrack', and `TRestAxionField::GammaTransmissionProbability' 
//*** and `TRestAxionField::GammaTransmissionFieldMapProbability'
//***
//*** Author: Raul Ena
//*******************************************************************************************************

// Constants
constexpr bool kDebug = true;
constexpr bool kSave = true;
constexpr bool kPlot = true;

// Function to select randomly nTracks of dx and dy
void selectDxy(const std::vector<Double_t>& dx, const std::vector<Double_t>& dy, Int_t nTracks, std::vector<Double_t>& selectedDx, std::vector<Double_t>& selectedDy);

// Function that sets Graph limits in x and y
void SetGraphLimits(TMultiGraph* mg, Double_t paddingPercentage);

Int_t REST_Axion_AnalysisTracksTime(Double_t nData = 20, Double_t Ea = 4.2, Double_t dMax = 10, Double_t dMin = -10,
                                    Double_t dL = 10, const std::string& gasName = "He", Double_t nTracks = 2) {

    auto start_time_code = std::chrono::high_resolution_clock::now();                                    
    const TVector3 startPoint(0, 0, -11000);
    const Double_t gasDensity = 9.345e-10;
    std::vector<Double_t> dx, dy, selectedDx, selectedDy;

    // Variables to Plot Tracks
    std::vector<TVector3> startPoints;
    std::vector<TVector3> endPoints;

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

    std::vector<std::string> fieldNames = {"babyIAXO_2024_cutoff" , "babyIAXO_2024"};

    // Fill in the values for later choose them randomly
    for(size_t k = 0; k < nData; k++){
        dx.push_back(dMin + k * (dMax - dMin) / nData);
        dy.push_back(dMin + k * (dMax - dMin) / nData);
    }

    if(nTracks <= 0 || nTracks > dx.size()) {
        std::cerr << "ERROR: Invalid number of selected tracks." << std::endl;
        return -1;
    }
    selectDxy(dx, dy, nTracks, selectedDx, selectedDy);

    // Fill in an use DrawTracks ti visualize the Tracks we choose
    for(size_t t = 0; t < selectedDx.size(); t++){
        startPoints.push_back(startPoint);
        endPoints.push_back(TVector3(selectedDx[t], selectedDy[t], 11000));
    }

    for (const auto& fieldName : fieldNames) {
        auto field = std::make_unique<TRestAxionMagneticField>("fields.rml", fieldName);
        axionField->AssignMagneticField(field.get());

        // Plot Tracks
        if(kPlot)
            field->DrawTracks(startPoints, endPoints, 100, kSave);

        auto canvasHeatMapProbGSL = std::make_unique<TCanvas>((fieldName + "_Probability_HeatmapsGSL").c_str(), (fieldName + " Probability HeatmapsGSL").c_str(), 850, 700);
        auto canvasHeatMapRunTimeGSL = std::make_unique<TCanvas>((fieldName + "_Runtime_HeatmapsGSL").c_str(), (fieldName + " Runtime Heatmaps").c_str(), 850, 700);
        auto canvasHeatMapProbStandard = std::make_unique<TCanvas>((fieldName + "_Probability_HeatmapsStandard").c_str(), (fieldName + " Probability HeatmapsStandard").c_str(), 850, 700);
        auto canvasHeatMapRunTimeStandard = std::make_unique<TCanvas>((fieldName + "_Runtime_HeatmapsStandard").c_str(), (fieldName + " Runtime HeatmapsStandard").c_str(), 850, 700);

        auto heatmapProbStandard = std::make_unique<TH2D>(("ProbabilityStandard_" + fieldName).c_str(), (fieldName + " Heatmap Probability Standard").c_str(), nData, dMin, dMax, nData, dMin, dMax);
        auto heatmapProbGSL = std::make_unique<TH2D>(("ProbabilityGSL_" + fieldName).c_str(), (fieldName + " Heatmap Probability GSL").c_str(), nData, dMin, dMax, nData, dMin, dMax);
        auto heatmapRuntimeStandard = std::make_unique<TH2D>(("RuntimeStandard_" + fieldName).c_str(), (fieldName + " Heatmap Runtime Standard").c_str(), nData, dMin, dMax, nData, dMin, dMax);
        auto heatmapRuntimeGSL = std::make_unique<TH2D>(("RuntimeGSL_" + fieldName).c_str(), (fieldName + " Heatmap Runtime GSL").c_str(), nData, dMin, dMax, nData, dMin, dMax);

        // Vector to hold probabilities for each selected dx
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDxProbabilitiesGSL(selectedDx.size());
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDxProbabilitiesStandard(selectedDx.size());
        // Vector to hold probabilities for each selected dy
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDyProbabilitiesGSL(selectedDy.size());
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDyProbabilitiesStandard(selectedDy.size());

        // Vector to hold runtimes for each selected dx
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDxRunTimeGSL(selectedDx.size());
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDxRunTimeStandard(selectedDx.size());
        // Vector to hold runtimes for each selected dy
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDyRunTimeGSL(selectedDy.size());
        std::vector<std::pair<std::vector<Double_t>, Double_t>> selectedDyRunTimeStandard(selectedDy.size());

        for (size_t i = 0; i < nData; ++i) {
            Double_t xEnd = dx[i];
            for (size_t j = 0; j < nData; ++j) {
                Double_t yEnd = dy[j];
                TVector3 endPoint(xEnd, yEnd, 11000);

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

                        selectedDxRunTimeGSL[k].first.push_back(duration_gsl.count());
                        selectedDxRunTimeStandard[k].first.push_back(duration_standard.count());
                        selectedDxRunTimeGSL[k].second = xEnd;
                        selectedDxRunTimeStandard[k].second = xEnd;

                        if (kDebug) {
                            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                            std::cout << "Data saved for selected dx: " << xEnd << std::endl;
                            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        }

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

                        selectedDyRunTimeGSL[k].first.push_back(duration_gsl.count());
                        selectedDyRunTimeStandard[k].first.push_back(duration_standard.count());
                        selectedDyRunTimeGSL[k].second = yEnd;
                        selectedDyRunTimeStandard[k].second = yEnd;

                        if (kDebug) {
                            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                            std::cout << "Data saved for selected dy: " << yEnd << std::endl;
                            std::cout << "+--------------------------------------------------------------------------+" << std::endl;
                        }

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
        heatmapProbStandard->SetStats(0);
        heatmapProbStandard->GetXaxis()->SetTitle("dx");
        heatmapProbStandard->GetYaxis()->SetTitle("dy");
        heatmapProbStandard->GetZaxis()->SetTitle("Probability");
        heatmapProbStandard->GetXaxis()->SetTitleSize(0.03); 
        heatmapProbStandard->GetXaxis()->SetTitleFont(40);  
        heatmapProbStandard->GetXaxis()->SetLabelSize(0.025); 
        heatmapProbStandard->GetXaxis()->SetLabelFont(40);  
        heatmapProbStandard->GetYaxis()->SetTitleSize(0.03); 
        heatmapProbStandard->GetYaxis()->SetTitleFont(40);  
        heatmapProbStandard->GetYaxis()->SetLabelSize(0.025); 
        heatmapProbStandard->GetYaxis()->SetLabelFont(40); 
        heatmapProbStandard->GetYaxis()->SetTitleOffset(1.2);
        heatmapProbStandard->GetYaxis()->SetLabelOffset(0.012); 
        heatmapProbStandard->GetXaxis()->SetTitleOffset(1.1);
        heatmapProbStandard->GetXaxis()->SetLabelOffset(0.012);

        heatmapProbStandard->GetZaxis()->SetTitleSize(0.03); 
        heatmapProbStandard->GetZaxis()->SetTitleFont(40);  
        heatmapProbStandard->GetZaxis()->SetLabelSize(0.025); 
        heatmapProbStandard->GetZaxis()->SetLabelFont(40); 
        heatmapProbStandard->GetZaxis()->SetTitleOffset(1.45);
        heatmapProbStandard->GetZaxis()->SetLabelOffset(0.012);

        heatmapProbStandard->SetContour(100);
        gStyle->SetPalette(kRainBow); 
        gPad->SetRightMargin(0.15);
        heatmapProbStandard->Draw("COLZ");
        canvasHeatMapProbStandard->Update();


        canvasHeatMapProbGSL->cd();
        heatmapProbGSL->SetStats(0);
        heatmapProbGSL->GetXaxis()->SetTitle("dx");
        heatmapProbGSL->GetYaxis()->SetTitle("dy");
        heatmapProbGSL->GetZaxis()->SetTitle("Probability");
        heatmapProbGSL->GetXaxis()->SetTitleSize(0.03); 
        heatmapProbGSL->GetXaxis()->SetTitleFont(40);  
        heatmapProbGSL->GetXaxis()->SetLabelSize(0.025); 
        heatmapProbGSL->GetXaxis()->SetLabelFont(40);  
        heatmapProbGSL->GetYaxis()->SetTitleSize(0.03); 
        heatmapProbGSL->GetYaxis()->SetTitleFont(40);  
        heatmapProbGSL->GetYaxis()->SetLabelSize(0.025); 
        heatmapProbGSL->GetYaxis()->SetLabelFont(40); 
        heatmapProbGSL->GetYaxis()->SetTitleOffset(1.2);
        heatmapProbGSL->GetYaxis()->SetLabelOffset(0.015);
        heatmapProbGSL->GetXaxis()->SetTitleOffset(1.1);
        heatmapProbGSL->GetXaxis()->SetLabelOffset(0.015);

        heatmapProbGSL->GetZaxis()->SetTitleSize(0.03); 
        heatmapProbGSL->GetZaxis()->SetTitleFont(40);  
        heatmapProbGSL->GetZaxis()->SetLabelSize(0.025); 
        heatmapProbGSL->GetZaxis()->SetLabelFont(40); 
        heatmapProbGSL->GetZaxis()->SetTitleOffset(1.45);
        heatmapProbGSL->GetZaxis()->SetLabelOffset(0.012);

        heatmapProbGSL->SetContour(100);
        gStyle->SetPalette(kRainBow); 
        gPad->SetRightMargin(0.15);
        heatmapProbGSL->Draw("COLZ");
        canvasHeatMapProbGSL->Update();

        canvasHeatMapRunTimeStandard->cd();
        heatmapRuntimeStandard->SetStats(0);
        heatmapRuntimeStandard->GetXaxis()->SetTitle("dx");
        heatmapRuntimeStandard->GetYaxis()->SetTitle("dy");
        heatmapRuntimeStandard->GetZaxis()->SetTitle("Time (#mu s)");
        heatmapRuntimeStandard->GetXaxis()->SetTitleSize(0.03); 
        heatmapRuntimeStandard->GetXaxis()->SetTitleFont(40);  
        heatmapRuntimeStandard->GetXaxis()->SetLabelSize(0.025); 
        heatmapRuntimeStandard->GetXaxis()->SetLabelFont(40);  
        heatmapRuntimeStandard->GetYaxis()->SetTitleSize(0.03); 
        heatmapRuntimeStandard->GetYaxis()->SetTitleFont(40);  
        heatmapRuntimeStandard->GetYaxis()->SetLabelSize(0.025); 
        heatmapRuntimeStandard->GetYaxis()->SetLabelFont(40); 
        heatmapRuntimeStandard->GetYaxis()->SetTitleOffset(1.2);
        heatmapRuntimeStandard->GetYaxis()->SetLabelOffset(0.015); 
        heatmapRuntimeStandard->GetXaxis()->SetTitleOffset(1.1);
        heatmapRuntimeStandard->GetXaxis()->SetLabelOffset(0.015);
            
        heatmapRuntimeStandard->GetZaxis()->SetTitleSize(0.03); 
        heatmapRuntimeStandard->GetZaxis()->SetTitleFont(40);  
        heatmapRuntimeStandard->GetZaxis()->SetLabelSize(0.025); 
        heatmapRuntimeStandard->GetZaxis()->SetLabelFont(40); 
        heatmapRuntimeStandard->GetZaxis()->SetTitleOffset(1.45);
        heatmapRuntimeStandard->GetZaxis()->SetLabelOffset(0.012);
        heatmapRuntimeStandard->SetContour(100);
        gStyle->SetPalette(kRainBow); 
        gPad->SetRightMargin(0.15);
        heatmapRuntimeStandard->Draw("COLZ");
        canvasHeatMapRunTimeStandard->Update();

        canvasHeatMapRunTimeGSL->cd();
        heatmapRuntimeGSL->SetStats(0);
        heatmapRuntimeGSL->GetXaxis()->SetTitle("dx");
        heatmapRuntimeGSL->GetYaxis()->SetTitle("dy");
        heatmapRuntimeGSL->GetZaxis()->SetTitle("Error");
        heatmapRuntimeGSL->GetXaxis()->SetTitleSize(0.03); 
        heatmapRuntimeGSL->GetXaxis()->SetTitleFont(40);  
        heatmapRuntimeGSL->GetXaxis()->SetLabelSize(0.025); 
        heatmapRuntimeGSL->GetXaxis()->SetLabelFont(40);  
        heatmapRuntimeGSL->GetYaxis()->SetTitleSize(0.03); 
        heatmapRuntimeGSL->GetYaxis()->SetTitleFont(40);  
        heatmapRuntimeGSL->GetYaxis()->SetLabelSize(0.025); 
        heatmapRuntimeGSL->GetYaxis()->SetLabelFont(40); 
        heatmapRuntimeGSL->GetYaxis()->SetTitleOffset(1.2);
        heatmapRuntimeGSL->GetYaxis()->SetLabelOffset(0.015); 
        heatmapRuntimeGSL->GetXaxis()->SetTitleOffset(1.1);
        heatmapRuntimeGSL->GetXaxis()->SetLabelOffset(0.015);
            
        heatmapRuntimeGSL->GetZaxis()->SetTitleSize(0.03); 
        heatmapRuntimeGSL->GetZaxis()->SetTitleFont(40);  
        heatmapRuntimeGSL->GetZaxis()->SetLabelSize(0.025); 
        heatmapRuntimeGSL->GetZaxis()->SetLabelFont(40); 
        heatmapRuntimeGSL->GetZaxis()->SetTitleOffset(1.45);
        heatmapRuntimeGSL->GetZaxis()->SetLabelOffset(0.012);
        heatmapRuntimeGSL->SetContour(100);
        gStyle->SetPalette(kRainBow); 
        gPad->SetRightMargin(0.15);
        heatmapRuntimeGSL->Draw("COLZ");
        canvasHeatMapRunTimeGSL->Update();

        if (kSave) {
            std::string folder = "HeatMapsTracks/";
            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directory(folder);
            }
            std::string imageNameProbS = folder + fieldName + "_Probability_HeatmapsStandard.png";
            std::string imageNameProbG = folder + fieldName + "_Probability_HeatmapsGSL.png";
            std::string imageNameRunTimeS = folder + fieldName + "_Runtime_HeatmapsStandard.png";
            std::string imageNameRunTimeG = folder + fieldName + "_Runtime_HeatmapsGSL.png";
            canvasHeatMapProbStandard->SaveAs(imageNameProbS.c_str());
            canvasHeatMapProbGSL->SaveAs(imageNameProbG.c_str());
            canvasHeatMapRunTimeStandard->SaveAs(imageNameRunTimeS.c_str());
            canvasHeatMapRunTimeGSL->SaveAs(imageNameRunTimeG.c_str());
        }

        if (kPlot) {
            auto canvasProb = std::make_unique<TCanvas>((fieldName + "_ProbabilityPlot").c_str(), (fieldName + " ProbabilityPlot").c_str(), 800, 600);
            canvasProb->Divide(2,2);
            
            canvasProb->cd(1);
            auto mg_dx_prob_gsl = std::make_unique<TMultiGraph>();
            mg_dx_prob_gsl->SetTitle("Probability vs. dy for selected dx (GSL)");
            mg_dx_prob_gsl->GetXaxis()->SetTitle("dy (mm)");
            mg_dx_prob_gsl->GetYaxis()->SetTitle("Probability");
            mg_dx_prob_gsl->GetXaxis()->SetLabelSize(0.03); 
            mg_dx_prob_gsl->GetYaxis()->SetLabelSize(0.03); 
            mg_dx_prob_gsl->GetXaxis()->SetTitleSize(0.03); 
            mg_dx_prob_gsl->GetYaxis()->SetTitleSize(0.03); 
            mg_dx_prob_gsl->GetXaxis()->SetTitleOffset(1.2); 
            mg_dx_prob_gsl->GetYaxis()->SetTitleOffset(1.2);

            Int_t colorIndex = 1;
            for (size_t i = 0; i < selectedDxProbabilitiesGSL.size(); ++i) {
                auto& probabilities = selectedDxProbabilitiesGSL[i].first;
                auto EndX = selectedDxProbabilitiesGSL[i].second;

                auto graph = std::make_unique<TGraph>(nData, &dy[0], &probabilities[0]);
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dx_prob_gsl->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dx_prob_gsl.get(), 0.05);
            mg_dx_prob_gsl->Draw("ACP");

            // Pad 2 for selectedDxProbabilitiesStandard
            canvasProb->cd(2);
            auto mg_dx_prob_standard = std::make_unique<TMultiGraph>();
            mg_dx_prob_standard->SetTitle("Probability vs. dy for selected dx (Standard)");
            mg_dx_prob_standard->GetXaxis()->SetTitle("dy (mm)");
            mg_dx_prob_standard->GetYaxis()->SetTitle("Probability");
            mg_dx_prob_standard->GetXaxis()->SetLabelSize(0.03); 
            mg_dx_prob_standard->GetYaxis()->SetLabelSize(0.03); 
            mg_dx_prob_standard->GetXaxis()->SetTitleSize(0.03); 
            mg_dx_prob_standard->GetYaxis()->SetTitleSize(0.03); 
            mg_dx_prob_standard->GetXaxis()->SetTitleOffset(1.2); 
            mg_dx_prob_standard->GetYaxis()->SetTitleOffset(1.2);

            colorIndex = 1;
            for (size_t i = 0; i < selectedDxProbabilitiesStandard.size(); ++i) {
                auto& probabilities = selectedDxProbabilitiesStandard[i].first;
                auto EndX = selectedDxProbabilitiesStandard[i].second;

                auto graph = std::make_unique<TGraph>(nData, &dy[0], &probabilities[0]);
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dx_prob_standard->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dx_prob_standard.get(), 0.05);
            mg_dx_prob_standard->Draw("ACP");

            // Pad 3 for selectedDyProbabilitiesGSL
            canvasProb->cd(3);
            auto mg_dy_prob_gsl = std::make_unique<TMultiGraph>();
            mg_dy_prob_gsl->SetTitle("Probability vs. dx for selected dy (GSL)");
            mg_dy_prob_gsl->GetXaxis()->SetTitle("dx (mm)");
            mg_dy_prob_gsl->GetYaxis()->SetTitle("Probability");
            mg_dy_prob_gsl->GetXaxis()->SetLabelSize(0.03); 
            mg_dy_prob_gsl->GetYaxis()->SetLabelSize(0.03); 
            mg_dy_prob_gsl->GetXaxis()->SetTitleSize(0.03); 
            mg_dy_prob_gsl->GetYaxis()->SetTitleSize(0.03); 
            mg_dy_prob_gsl->GetXaxis()->SetTitleOffset(1.2); 
            mg_dy_prob_gsl->GetYaxis()->SetTitleOffset(1.2);

            colorIndex = 1;
            for (size_t i = 0; i < selectedDyProbabilitiesGSL.size(); ++i) {
                auto& probabilities = selectedDyProbabilitiesGSL[i].first;
                auto EndY = selectedDyProbabilitiesGSL[i].second;

                auto graph = std::make_unique<TGraph>(probabilities.size(), dx.data(), probabilities.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dy_prob_gsl->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dy_prob_gsl.get(), 0.05);
            mg_dy_prob_gsl->Draw("ACP");

            // Pad 4 for selectedDyProbabilitiesStandard
            canvasProb->cd(4);
            auto mg_dy_prob_standard = std::make_unique<TMultiGraph>();
            mg_dy_prob_standard->SetTitle("Probability vs. dx for selected dy (Standard)");
            mg_dy_prob_standard->GetXaxis()->SetTitle("dx (mm)");
            mg_dy_prob_standard->GetYaxis()->SetTitle("Probability");
            mg_dy_prob_standard->GetXaxis()->SetLabelSize(0.03); 
            mg_dy_prob_standard->GetYaxis()->SetLabelSize(0.03); 
            mg_dy_prob_standard->GetXaxis()->SetTitleSize(0.03); 
            mg_dy_prob_standard->GetYaxis()->SetTitleSize(0.03); 
            mg_dy_prob_standard->GetXaxis()->SetTitleOffset(1.2); 
            mg_dy_prob_standard->GetYaxis()->SetTitleOffset(1.2);

            colorIndex = 1;
            for (size_t i = 0; i < selectedDyProbabilitiesStandard.size(); ++i) {
                auto& probabilities = selectedDyProbabilitiesStandard[i].first;
                auto EndY = selectedDyProbabilitiesStandard[i].second;

                auto graph = std::make_unique<TGraph>(probabilities.size(), dx.data(), probabilities.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dy_prob_standard->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dy_prob_standard.get(), 0.05);
            mg_dy_prob_standard->Draw("ACP");

            canvasProb->Draw();

            if (kSave) {
                std::string folder = "HeatMapsTracks/";
                std::string imageNameProb = folder + fieldName + "_Probability_Plots.png";
                canvasProb->SaveAs(imageNameProb.c_str());
            }

            auto canvasRuntime = std::make_unique<TCanvas>((fieldName + "_RunTimePlot").c_str(), (fieldName + " RunTimePlot").c_str(), 800, 600);
            canvasRuntime->Divide(2, 2);

            // Pad 1 for selectedDxProbabilitiesGSL
            canvasRuntime->cd(1);
            auto mg_dx_run_gsl = std::make_unique<TMultiGraph>();
            mg_dx_run_gsl->SetTitle("RunTime vs. dy for selected dx (GSL)");
            mg_dx_run_gsl->GetXaxis()->SetTitle("dy (mm)");
            mg_dx_run_gsl->GetYaxis()->SetTitle("Runtime (μs)");
            mg_dx_run_gsl->GetXaxis()->SetLabelSize(0.03); 
            mg_dx_run_gsl->GetYaxis()->SetLabelSize(0.03); 
            mg_dx_run_gsl->GetXaxis()->SetTitleSize(0.03); 
            mg_dx_run_gsl->GetYaxis()->SetTitleSize(0.03); 
            mg_dx_run_gsl->GetXaxis()->SetTitleOffset(1.2); 
            mg_dx_run_gsl->GetYaxis()->SetTitleOffset(1.2);

            colorIndex = 1;
            for (size_t i = 0; i < selectedDxRunTimeGSL.size(); ++i) {
                auto& runTime = selectedDxRunTimeGSL[i].first;
                auto EndX = selectedDxRunTimeGSL[i].second;
                
                auto graph = std::make_unique<TGraph>(runTime.size(), dy.data(), runTime.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dx_run_gsl->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dx_run_gsl.get(), 0.05);
            mg_dx_run_gsl->Draw("ACP");

            // Pad 2 for selectedDxProbabilitiesStandard
            canvasRuntime->cd(2);
            auto mg_dx_run_standard = std::make_unique<TMultiGraph>();   
            mg_dx_run_standard->SetTitle("RunTime vs. dy for selected dx (Standard)");
            mg_dx_run_standard->GetXaxis()->SetTitle("dy (mm)");
            mg_dx_run_standard->GetYaxis()->SetTitle("Runtime (ms)");
            mg_dx_run_standard->GetXaxis()->SetLabelSize(0.03); 
            mg_dx_run_standard->GetYaxis()->SetLabelSize(0.03); 
            mg_dx_run_standard->GetXaxis()->SetTitleSize(0.03); 
            mg_dx_run_standard->GetYaxis()->SetTitleSize(0.03); 
            mg_dx_run_standard->GetXaxis()->SetTitleOffset(1.2); 
            mg_dx_run_standard->GetYaxis()->SetTitleOffset(1.2);

            colorIndex = 1;
            for (size_t i = 0; i < selectedDxRunTimeStandard.size(); ++i) {
                auto& runTime = selectedDxRunTimeStandard[i].first;
                auto EndX = selectedDxRunTimeStandard[i].second;
                
                auto graph = std::make_unique<TGraph>(runTime.size(), dy.data(), runTime.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dx_run_standard->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dx_run_standard.get(), 0.05);
            mg_dx_run_standard->Draw("ACP");

            // Pad 3 for selectedDyProbabilitiesGSL
            canvasRuntime->cd(3);
            auto mg_dy_run_gsl = std::make_unique<TMultiGraph>();  
            mg_dy_run_gsl->SetTitle("RunTime vs. dx for selected dy (GSL)");
            mg_dy_run_gsl->GetXaxis()->SetTitle("dx (mm)");
            mg_dy_run_gsl->GetYaxis()->SetTitle("Runtime (μs)");
            mg_dy_run_gsl->GetXaxis()->SetLabelSize(0.03); 
            mg_dy_run_gsl->GetYaxis()->SetLabelSize(0.03); 
            mg_dy_run_gsl->GetXaxis()->SetTitleSize(0.03); 
            mg_dy_run_gsl->GetYaxis()->SetTitleSize(0.03); 
            mg_dy_run_gsl->GetXaxis()->SetTitleOffset(1.2); 
            mg_dy_run_gsl->GetYaxis()->SetTitleOffset(1.2);
            std::unique_ptr<TLegend> legend_dy_run_gsl(new TLegend(0.1, 0.7, 0.48, 0.9));
            for (size_t i = 0; i < selectedDyRunTimeGSL.size(); ++i) {
                auto& runTime = selectedDyRunTimeGSL[i].first;
                auto EndY = selectedDyRunTimeGSL[i].second;

                auto graph = std::make_unique<TGraph>(runTime.size(), dx.data(), runTime.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dy_run_gsl->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dy_run_gsl.get(), 0.05);
            mg_dy_run_gsl->Draw("ACP");

            // Pad 4 for selectedDyProbabilitiesStandard
            canvasRuntime->cd(4);
            auto mg_dy_run_standard = std::make_unique<TMultiGraph>();  
            mg_dy_run_standard->SetTitle("RunTime vs. dx for selected dy (Standard)");
            mg_dy_run_standard->GetXaxis()->SetTitle("dx (mm)");
            mg_dy_run_standard->GetYaxis()->SetTitle("Runtime (ms)");
            mg_dy_run_standard->GetXaxis()->SetLabelSize(0.03); 
            mg_dy_run_standard->GetYaxis()->SetLabelSize(0.03); 
            mg_dy_run_standard->GetXaxis()->SetTitleSize(0.03); 
            mg_dy_run_standard->GetYaxis()->SetTitleSize(0.03); 
            mg_dy_run_standard->GetXaxis()->SetTitleOffset(1.2); 
            mg_dy_run_standard->GetYaxis()->SetTitleOffset(1.2);

            colorIndex = 1;
            for (size_t i = 0; i < selectedDyRunTimeStandard.size(); ++i) {
                auto& runTime = selectedDyRunTimeStandard[i].first;
                auto EndY = selectedDyRunTimeStandard[i].second;

                auto graph = std::make_unique<TGraph>(runTime.size(), dx.data(), runTime.data());
                graph->SetLineColor(colorIndex);
                graph->SetLineWidth(1);
                mg_dy_run_standard->Add(graph.get()); 
                graph.release();

                colorIndex ++;
            }
            SetGraphLimits(mg_dy_run_standard.get(), 0.05);
            mg_dy_run_standard->Draw("ACP");
            canvasRuntime->Draw();

            if (kSave) {
                std::string folder = "HeatMapsTracks/";
                std::string imageNameRuntime = folder + fieldName + "_Runtime_Plots.png";
                canvasRuntime->SaveAs(imageNameRuntime.c_str());
            }
        }

    }
    auto end_time_code = std::chrono::high_resolution_clock::now();
    auto duration_code = std::chrono::duration_cast<std::chrono::seconds>(end_time_code - start_time_code);

    std::ofstream outFile("HeatMapsTracks/DurationCode.txt");
    if (outFile.is_open()) {
        outFile << duration_code.count() << " seconds";
        outFile.close();
    } else {
        std::cerr << "Error: Unable to open DurationCode.txt for writing." << std::endl;
    }

    return 0;
}

void selectDxy(const std::vector<Double_t>& dx, const std::vector<Double_t>& dy, Int_t nTracks, std::vector<Double_t>& selectedDx, std::vector<Double_t>& selectedDy) {
    // Number of data points
    Int_t nData = dx.size();
    // Create a vector to hold indices from 0 to nData-1
    std::vector<Int_t> indices(nData);
    std::iota(indices.begin(), indices.end(), 0);

    // Shuffle the indices
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    // Clear selectedDx and selectedDy
    selectedDx.clear();
    selectedDy.clear();

    // Select nTracks random indices from shuffled indices
    for (Int_t i = 0; i < nTracks; ++i) {
        // Check if the index has already been selected
        // If yes, choose another one
        Int_t randomIndex;
        do {
            randomIndex = indices.back(); // Take the last index in shuffled order
            indices.pop_back(); // Remove it from the list
        } while (std::find(selectedDx.begin(), selectedDx.end(), dx[randomIndex]) != selectedDx.end() &&
                 std::find(selectedDy.begin(), selectedDy.end(), dy[randomIndex]) != selectedDy.end());
        
        // Add the selected values to selectedDx and selectedDy
        selectedDx.push_back(dx[randomIndex]);
        selectedDy.push_back(dy[randomIndex]);
    }
}

void SetGraphLimits(TMultiGraph* mg, Double_t paddingPercentage) {
    if (!mg) return;

    Double_t xMin = std::numeric_limits<Double_t>::max();
    Double_t xMax = std::numeric_limits<Double_t>::lowest();
    Double_t yMin = std::numeric_limits<Double_t>::max();
    Double_t yMax = std::numeric_limits<Double_t>::lowest();

    // Iterate through all graphs in the TMultiGraph
    TIter next(mg->GetListOfGraphs());
    TObject* obj;
    while ((obj = next())) {
        if (obj->InheritsFrom(TGraph::Class())) {
            TGraph* graph = dynamic_cast<TGraph*>(obj);
            if (graph) {
                Int_t nPoints = graph->GetN();
                if (nPoints == 0) continue;

                // Compute minimum and maximum x and y values
                Double_t* xVals = graph->GetX();
                Double_t* yVals = graph->GetY();
                for (Int_t i = 0; i < nPoints; ++i) {
                    if (xVals[i] < xMin) xMin = xVals[i];
                    if (xVals[i] > xMax) xMax = xVals[i];
                    if (yVals[i] < yMin) yMin = yVals[i];
                    if (yVals[i] > yMax) yMax = yVals[i];
                }
            }
        }
    }

    // Set axis limits with padding
    Double_t xPadding = paddingPercentage * (xMax - xMin);
    Double_t yPadding = paddingPercentage * (yMax - yMin);
    mg->GetXaxis()->SetLimits(xMin - xPadding, xMax + xPadding);
    mg->GetYaxis()->SetRangeUser(yMin - yPadding, yMax + yPadding);
}
