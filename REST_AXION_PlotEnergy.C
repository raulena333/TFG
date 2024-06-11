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
    std::string obName3 = "window_transmission";
    std::vector<Double_t> efficiency_values, energy_values, window_values;

    Int_t n_bins = 100;
    Double_t energy_maximum = 10.0;
    Double_t minimum = 0.0;
    Double_t efficiency_maximum = 1.0;

    // Create vectors for energy and efficiency values
    std::vector<Double_t> energy_vector(n_bins, 0.0);
    std::vector<std::vector<Double_t>> efficiency_vector(n_bins);
    std::vector<std::vector<Double_t>> window_vector(n_bins);

    Double_t bin_width = (energy_maximum - minimum) / n_bins;
    for (Int_t i = 0; i < n_bins; ++i) {
        energy_vector[i] = minimum + (bin_width * i);
    }

    // Get the observable ID
    Int_t id1 = ana->GetObservableID(obName1);
    Int_t id2 = ana->GetObservableID(obName2);
    Int_t id3 = ana->GetObservableID(obName3);
    Int_t nOfRuns = run->GetEntries(); // Get the number of entries

    for (auto i = 0; i < nOfRuns; i++){
        run->GetEntry(i);
        Double_t efficiency = ana->GetDblObservableValue(id1);
        Double_t energy = ana->GetDblObservableValue(id2);
        Double_t window = ana->GetDblObservableValue(id3);

        efficiency_values.push_back(efficiency);
        energy_values.push_back(energy);
        window_values.push_back(window);
        
        // Find the bin for the current energy value
        Int_t bin = static_cast<Int_t>((energy - minimum) / bin_width);
        
        // Add efficiency value to the corresponding bin
        if (bin >= 0 && bin < n_bins) {
            efficiency_vector[bin].push_back(efficiency);
            window_vector[bin].push_back(window);
        }
    }

    // Calculate means and errors for each bin
    std::vector<Double_t> efficiency_means(n_bins, 0.0);
    std::vector<Double_t> efficiency_errors(n_bins, 0.0);
    std::vector<Double_t> window_means(n_bins, 0.0);
    std::vector<Double_t> window_errors(n_bins, 0.0);

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

        if (!window_vector[j].empty()) {
            Double_t sum = 0.0;
            for (Double_t value : window_vector[j]) {
                sum += value;
            }
            Double_t mean = sum / window_vector[j].size();
            
            // Calculate standard deviation
            Double_t sum_squared_diff = 0.0;
            for (Double_t value : window_vector[j]) {
                sum_squared_diff += (value - mean) * (value - mean);
            }
            Double_t variance = sum_squared_diff / window_vector[j].size();
            Double_t error = (window_vector[j].size() > 1) ? sqrt(variance) / sqrt(window_vector[j].size()) : 0.0;
            
            window_means[j] = mean;
            window_errors[j] = error;
        } else {
            window_means[j] = 0.0;
            window_errors[j] = 0.0;
        }
    }

    // Calculate error in energy (ex)
    std::vector<Double_t> energy_errors(n_bins, bin_width / 2.0);

    /// PLOT ///
    TCanvas *canvas = new TCanvas("canvas", "Efficiency against energy", 800, 600);
    canvas->cd();

    // Create TGraphErrors without errors for x-axis
    TGraphErrors *graph = new TGraphErrors(n_bins, &energy_vector[0], &efficiency_means[0], 0, &efficiency_errors[0]);

    graph->SetTitle("");
    graph->GetYaxis()->SetTitle("Eficiencia");
    graph->GetXaxis()->SetTitle("Energia (keV)");
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(0.45);
    graph->SetMarkerColor(kRed-3);
    graph->GetXaxis()->SetRangeUser(0,90);
    graph->GetXaxis()->SetTitleSize(0.065);
    graph->GetXaxis()->SetLabelSize(0.065);
    graph->GetYaxis()->SetTitleSize(0.065);
    graph->GetYaxis()->SetLabelSize(0.065);
    graph->GetYaxis()->SetTitleFont(40);
    graph->GetYaxis()->SetTitleOffset(1.05);
    graph->GetXaxis()->SetTitleOffset(1.0);
    graph->GetXaxis()->SetTitleFont(40); 
    graph->GetYaxis()->SetLabelFont(40);
    graph->GetXaxis()->SetLabelFont(40);

    graph->Draw("AP");

    gPad->Update();
    TPaletteAxis *palette = (TPaletteAxis*)graph->GetListOfFunctions()->FindObject("palette");
    gPad->SetLeftMargin(0.13);
    gPad->SetBottomMargin(0.145);
    gPad->SetTopMargin(0.1);


    TGraphErrors *graphWindow = new TGraphErrors(n_bins, &energy_vector[0], &window_means[0], 0, &window_errors[0]);
    graphWindow->SetTitle("");
    graphWindow->SetMarkerStyle(8);
    graphWindow->SetMarkerSize(0.45);
    graphWindow->SetMarkerColor(kBlue);
    graphWindow->Draw("P SAME");

    // Add legend
    TLegend *legend = new TLegend(0.67, 0.74, 0.9, 0.9);
    legend->SetTextSize(0.045);
    legend->AddEntry(graph, "Optica", "lep");
    legend->AddEntry(graphWindow, "Ventana", "lep");
    legend->Draw();

    canvas->Update();

    // Save canvas as an image file
    canvas->SaveAs("Efficiency_Plot.pdf");

    delete canvas;
}




