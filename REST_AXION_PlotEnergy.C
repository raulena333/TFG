#include <iostream>
#include <vector>
#include "TCanvas.h"
#include "TGraphErrors.h"

void REST_AXION_PlotEnergy(TRestRun *run, TRestAnalysisTree *ana){

    // Reset ROOT environment
    gROOT->Reset();

    // Variables
    std::string obName1 = "optics_efficiency";
    std::string obName2 = "final_energy";
    std::vector<Double_t> efficiency_values, energy_values;

    Int_t n_bins = 100;
    Double_t energy_maximum = 10.0;
    Double_t minimum = 0.0;
    Double_t efficiency_maximum = 1.0;

    // Create vectors for energy and efficiency values
    std::vector<Double_t> energy_vector(n_bins, 0.0);
    std::vector<std::vector<Double_t>> efficiency_vector(n_bins);

    Double_t bin_width = (energy_maximum - minimum) / n_bins;
    for (Int_t i = 0; i < n_bins; ++i) {
        energy_vector[i] = minimum + (bin_width * i);
    }

    // Get the observable ID
    Int_t id1 = ana->GetObservableID(obName1);
    Int_t id2 = ana->GetObservableID(obName2);
    Int_t nOfRuns = run->GetEntries(); // Get the number of entries

    for (auto i = 0; i < nOfRuns; i++){
        run->GetEntry(i);
        Double_t efficiency = ana->GetDblObservableValue(id1);
        Double_t energy = ana->GetDblObservableValue(id2);

        efficiency_values.push_back(efficiency);
        energy_values.push_back(energy);
        
        // Find the bin for the current energy value
        Int_t bin = static_cast<Int_t>((energy - minimum) / bin_width);
        
        // Add efficiency value to the corresponding bin
        if (bin >= 0 && bin < n_bins) {
            efficiency_vector[bin].push_back(efficiency);
        }
    }

    // Calculate means and errors for each bin
    std::vector<Double_t> efficiency_means(n_bins, 0.0);
    std::vector<Double_t> efficiency_errors(n_bins, 0.0);
    for (Int_t j = 0; j < n_bins; ++j) {
        if (!efficiency_vector[j].empty()) {
            Double_t sum = 0.0;
            for (Double_t value : efficiency_vector[j]) {
                sum += value;
            }
            Double_t mean = sum / efficiency_vector[j].size();
            
            // Calculate standard deviation
            Double_t sum_squared_diff = 0.0;
            for (Double_t value : efficiency_vector[j]) {
                sum_squared_diff += (value - mean) * (value - mean);
            }
            Double_t variance = sum_squared_diff / efficiency_vector[j].size();
            Double_t error = (efficiency_vector[j].size() > 1) ? sqrt(variance) / sqrt(efficiency_vector[j].size()) : 0.0;
            
            efficiency_means[j] = mean;
            efficiency_errors[j] = error;
        } else {
            efficiency_means[j] = 0.0;
            efficiency_errors[j] = 0.0;
        }
    }

    // Calculate error in energy (ex)
    std::vector<Double_t> energy_errors(n_bins, bin_width / 2.0);

    for(Int_t j = 0; j<n_bins; j++){
       std::cout << energy_vector[j] << "+" << energy_errors[j] << "\t" << efficiency_means[j] << "+" << efficiency_errors[j] << std::endl;
    }


    /// PLOT ///
    TCanvas *canvas = new TCanvas("canvas", "Efficiency against energy", 600, 430);
    canvas->cd();

    // Create TGraphErrors without errors for x-axis
    TGraphErrors *graph = new TGraphErrors(n_bins, &energy_vector[0], &efficiency_means[0], 0, &efficiency_errors[0]);

    graph->Draw("AP");
    graph->SetTitle("");
    graph->GetYaxis()->SetTitle("Eficiencia");
    graph->GetXaxis()->SetTitle("Energia (keV)");
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(0.4);
    graph->SetMarkerColor(kRed-3);
    graph->GetXaxis()->SetRange(0, 90);
    graph->GetXaxis()->SetTitleSize(0.04);
    graph->GetXaxis()->SetLabelSize(0.03);
    graph->GetYaxis()->SetTitleSize(0.04);
    graph->GetYaxis()->SetLabelSize(0.03);
    graph->GetYaxis()->SetTitleFont(62);
    graph->GetYaxis()->SetTitleOffset(0.8);
    graph->GetXaxis()->SetTitleFont(62); 
    graph->GetYaxis()->SetLabelFont(62);
    graph->GetXaxis()->SetLabelFont(62);

    canvas->Update();

    // Save canvas as an image file
    canvas->SaveAs("Efficiency_Plot.pdf");
}


