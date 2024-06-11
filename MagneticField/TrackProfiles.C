#ifdef __CLING__
#pragma cling optimize(0)
#endif
void TrackProfiles()
{
//=========Macro generated from canvas: fCanvas/
//=========  (Tue Jun  4 20:14:47 2024) by ROOT version 6.30/04
   TCanvas *fCanvas = new TCanvas("fCanvas", "",134,166,1600,600);
   fCanvas->SetHighLightColor(2);
   fCanvas->Range(0,0,1,1);
   fCanvas->SetFillColor(0);
   fCanvas->SetBorderMode(0);
   fCanvas->SetBorderSize(2);
   fCanvas->SetFrameBorderMode(0);
  
// ------------>Primitives in pad: pad1
   TPad *pad1__0 = new TPad("pad1", "This is pad1",0.01,0.02,0.99,0.97);
   pad1__0->Draw();
   pad1__0->cd();
   pad1__0->Range(0,0,1,1);
   pad1__0->SetFillColor(0);
   pad1__0->SetBorderMode(0);
   pad1__0->SetBorderSize(2);
   pad1__0->SetFrameBorderMode(0);
  
// ------------>Primitives in pad: pad1_1
   TPad *pad1_1__1 = new TPad("pad1_1", "pad1_1",0.01,0.01,0.49,0.99);
   pad1_1__1->Draw();
   pad1_1__1->cd();
   pad1_1__1->Range(-11631.58,-1482.53,10736.84,1288.554);
   pad1_1__1->SetFillColor(0);
   pad1_1__1->SetBorderMode(0);
   pad1_1__1->SetBorderSize(2);
   pad1_1__1->SetLeftMargin(0.14);
   pad1_1__1->SetTopMargin(0.05);
   pad1_1__1->SetBottomMargin(0.12);
   pad1_1__1->SetFrameBorderMode(0);
   pad1_1__1->SetFrameBorderMode(0);
   
   Double_t Graph_fx1[5] = { -6000, -6000, 6000, 6000, -6000 };
   Double_t Graph_fy1[5] = { -350, 350, 350, -350, -350 };
   TGraph *graph = new TGraph(5,Graph_fx1,Graph_fy1);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph1 = new TH1F("Graph_Graph1","",100,-8500,8500);
   Graph_Graph1->SetMinimum(-1150);
   Graph_Graph1->SetMaximum(1150);
   Graph_Graph1->SetDirectory(nullptr);
   Graph_Graph1->SetStats(0);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#000099");
   Graph_Graph1->SetLineColor(ci);
   Graph_Graph1->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph1->GetXaxis()->SetNdivisions(5);
   Graph_Graph1->GetXaxis()->SetLabelFont(42);
   Graph_Graph1->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph1->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph1->GetXaxis()->SetTitleOffset(1);
   Graph_Graph1->GetXaxis()->SetTitleFont(42);
   Graph_Graph1->GetYaxis()->SetTitle("Y [mm]");
   Graph_Graph1->GetYaxis()->SetLabelFont(42);
   Graph_Graph1->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph1->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph1->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph1->GetYaxis()->SetTitleFont(42);
   Graph_Graph1->GetZaxis()->SetLabelFont(42);
   Graph_Graph1->GetZaxis()->SetTitleOffset(1);
   Graph_Graph1->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph1);
   
   graph->Draw("al");
   
   Double_t Graph_fx2[2] = { -6100, 6100 };
   Double_t Graph_fy2[2] = { 0, 0 };
   graph = new TGraph(2,Graph_fx2,Graph_fy2);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#cccc00");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph2 = new TH1F("Graph_Graph2","",100,-7320,7320);
   Graph_Graph2->SetMinimum(0);
   Graph_Graph2->SetMaximum(1.1);
   Graph_Graph2->SetDirectory(nullptr);
   Graph_Graph2->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph2->SetLineColor(ci);
   Graph_Graph2->GetXaxis()->SetLabelFont(42);
   Graph_Graph2->GetXaxis()->SetTitleOffset(1);
   Graph_Graph2->GetXaxis()->SetTitleFont(42);
   Graph_Graph2->GetYaxis()->SetLabelFont(42);
   Graph_Graph2->GetYaxis()->SetTitleFont(42);
   Graph_Graph2->GetZaxis()->SetLabelFont(42);
   Graph_Graph2->GetZaxis()->SetTitleOffset(1);
   Graph_Graph2->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph2);
   
   graph->Draw("l");
   
   Double_t Graph_fx3[2] = { -5995, 6000 };
   Double_t Graph_fy3[2] = { 0, 0 };
   graph = new TGraph(2,Graph_fx3,Graph_fy3);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#cccc00");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph3 = new TH1F("Graph_Graph3","",100,-7194.5,7199.5);
   Graph_Graph3->SetMinimum(0);
   Graph_Graph3->SetMaximum(1.1);
   Graph_Graph3->SetDirectory(nullptr);
   Graph_Graph3->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph3->SetLineColor(ci);
   Graph_Graph3->GetXaxis()->SetLabelFont(42);
   Graph_Graph3->GetXaxis()->SetTitleOffset(1);
   Graph_Graph3->GetXaxis()->SetTitleFont(42);
   Graph_Graph3->GetYaxis()->SetLabelFont(42);
   Graph_Graph3->GetYaxis()->SetTitleFont(42);
   Graph_Graph3->GetZaxis()->SetLabelFont(42);
   Graph_Graph3->GetZaxis()->SetTitleOffset(1);
   Graph_Graph3->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph3);
   
   graph->Draw("l");
   
   Double_t Graph_fx4[2] = { -6100, 6100 };
   Double_t Graph_fy4[2] = { 350, -350 };
   graph = new TGraph(2,Graph_fx4,Graph_fy4);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#999966");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph4 = new TH1F("Graph_Graph4","",100,-7320,7320);
   Graph_Graph4->SetMinimum(-420);
   Graph_Graph4->SetMaximum(420);
   Graph_Graph4->SetDirectory(nullptr);
   Graph_Graph4->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph4->SetLineColor(ci);
   Graph_Graph4->GetXaxis()->SetLabelFont(42);
   Graph_Graph4->GetXaxis()->SetTitleOffset(1);
   Graph_Graph4->GetXaxis()->SetTitleFont(42);
   Graph_Graph4->GetYaxis()->SetLabelFont(42);
   Graph_Graph4->GetYaxis()->SetTitleFont(42);
   Graph_Graph4->GetZaxis()->SetLabelFont(42);
   Graph_Graph4->GetZaxis()->SetTitleOffset(1);
   Graph_Graph4->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph4);
   
   graph->Draw("l");
   
   Double_t Graph_fx5[2] = { -4308.368, 4313.351 };
   Double_t Graph_fy5[2] = { 247.2014, -247.4874 };
   graph = new TGraph(2,Graph_fx5,Graph_fy5);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#999966");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph5 = new TH1F("Graph_Graph5","",100,-5170.54,5175.523);
   Graph_Graph5->SetMinimum(-296.9563);
   Graph_Graph5->SetMaximum(296.6703);
   Graph_Graph5->SetDirectory(nullptr);
   Graph_Graph5->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph5->SetLineColor(ci);
   Graph_Graph5->GetXaxis()->SetLabelFont(42);
   Graph_Graph5->GetXaxis()->SetTitleOffset(1);
   Graph_Graph5->GetXaxis()->SetTitleFont(42);
   Graph_Graph5->GetYaxis()->SetLabelFont(42);
   Graph_Graph5->GetYaxis()->SetTitleFont(42);
   Graph_Graph5->GetZaxis()->SetLabelFont(42);
   Graph_Graph5->GetZaxis()->SetTitleOffset(1);
   Graph_Graph5->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph5);
   
   graph->Draw("l");
   
   Double_t Graph_fx6[2] = { -6100, 6100 };
   Double_t Graph_fy6[2] = { -350, 350 };
   graph = new TGraph(2,Graph_fx6,Graph_fy6);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#00cc00");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph6 = new TH1F("Graph_Graph6","",100,-7320,7320);
   Graph_Graph6->SetMinimum(-420);
   Graph_Graph6->SetMaximum(420);
   Graph_Graph6->SetDirectory(nullptr);
   Graph_Graph6->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph6->SetLineColor(ci);
   Graph_Graph6->GetXaxis()->SetLabelFont(42);
   Graph_Graph6->GetXaxis()->SetTitleOffset(1);
   Graph_Graph6->GetXaxis()->SetTitleFont(42);
   Graph_Graph6->GetYaxis()->SetLabelFont(42);
   Graph_Graph6->GetYaxis()->SetTitleFont(42);
   Graph_Graph6->GetZaxis()->SetLabelFont(42);
   Graph_Graph6->GetZaxis()->SetTitleOffset(1);
   Graph_Graph6->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph6);
   
   graph->Draw("l");
   
   Double_t Graph_fx7[2] = { -4308.368, 4313.351 };
   Double_t Graph_fy7[2] = { -247.2014, 247.4874 };
   graph = new TGraph(2,Graph_fx7,Graph_fy7);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#00cc00");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph7 = new TH1F("Graph_Graph7","",100,-5170.54,5175.523);
   Graph_Graph7->SetMinimum(-296.6703);
   Graph_Graph7->SetMaximum(296.9563);
   Graph_Graph7->SetDirectory(nullptr);
   Graph_Graph7->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph7->SetLineColor(ci);
   Graph_Graph7->GetXaxis()->SetLabelFont(42);
   Graph_Graph7->GetXaxis()->SetTitleOffset(1);
   Graph_Graph7->GetXaxis()->SetTitleFont(42);
   Graph_Graph7->GetYaxis()->SetLabelFont(42);
   Graph_Graph7->GetYaxis()->SetTitleFont(42);
   Graph_Graph7->GetZaxis()->SetLabelFont(42);
   Graph_Graph7->GetZaxis()->SetTitleOffset(1);
   Graph_Graph7->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph7);
   
   graph->Draw("l");
   
   Double_t Graph_fx8[2] = { -6100, 6100 };
   Double_t Graph_fy8[2] = { 20, 70 };
   graph = new TGraph(2,Graph_fx8,Graph_fy8);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#00cccc");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph8 = new TH1F("Graph_Graph8","",100,-7320,7320);
   Graph_Graph8->SetMinimum(15);
   Graph_Graph8->SetMaximum(75);
   Graph_Graph8->SetDirectory(nullptr);
   Graph_Graph8->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph8->SetLineColor(ci);
   Graph_Graph8->GetXaxis()->SetLabelFont(42);
   Graph_Graph8->GetXaxis()->SetTitleOffset(1);
   Graph_Graph8->GetXaxis()->SetTitleFont(42);
   Graph_Graph8->GetYaxis()->SetLabelFont(42);
   Graph_Graph8->GetYaxis()->SetTitleFont(42);
   Graph_Graph8->GetZaxis()->SetLabelFont(42);
   Graph_Graph8->GetZaxis()->SetTitleOffset(1);
   Graph_Graph8->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph8);
   
   graph->Draw("l");
   
   Double_t Graph_fx9[2] = { -5995, 6000 };
   Double_t Graph_fy9[2] = { 20.43033, 69.59016 };
   graph = new TGraph(2,Graph_fx9,Graph_fy9);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#00cccc");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph9 = new TH1F("Graph_Graph9","",100,-7194.5,7199.5);
   Graph_Graph9->SetMinimum(15.51434);
   Graph_Graph9->SetMaximum(74.50615);
   Graph_Graph9->SetDirectory(nullptr);
   Graph_Graph9->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph9->SetLineColor(ci);
   Graph_Graph9->GetXaxis()->SetLabelFont(42);
   Graph_Graph9->GetXaxis()->SetTitleOffset(1);
   Graph_Graph9->GetXaxis()->SetTitleFont(42);
   Graph_Graph9->GetYaxis()->SetLabelFont(42);
   Graph_Graph9->GetYaxis()->SetTitleFont(42);
   Graph_Graph9->GetZaxis()->SetLabelFont(42);
   Graph_Graph9->GetZaxis()->SetTitleOffset(1);
   Graph_Graph9->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph9);
   
   graph->Draw("l");
   
   Double_t Graph_fx10[2] = { -6100, 6100 };
   Double_t Graph_fy10[2] = { 120, -40 };
   graph = new TGraph(2,Graph_fx10,Graph_fy10);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#0000cc");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph10 = new TH1F("Graph_Graph10","",100,-7320,7320);
   Graph_Graph10->SetMinimum(-56);
   Graph_Graph10->SetMaximum(136);
   Graph_Graph10->SetDirectory(nullptr);
   Graph_Graph10->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph10->SetLineColor(ci);
   Graph_Graph10->GetXaxis()->SetLabelFont(42);
   Graph_Graph10->GetXaxis()->SetTitleOffset(1);
   Graph_Graph10->GetXaxis()->SetTitleFont(42);
   Graph_Graph10->GetYaxis()->SetLabelFont(42);
   Graph_Graph10->GetYaxis()->SetTitleFont(42);
   Graph_Graph10->GetZaxis()->SetLabelFont(42);
   Graph_Graph10->GetZaxis()->SetTitleOffset(1);
   Graph_Graph10->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph10);
   
   graph->Draw("l");
   
   Double_t Graph_fx11[2] = { -5995.001, 6000 };
   Double_t Graph_fy11[2] = { 118.623, -38.68852 };
   graph = new TGraph(2,Graph_fx11,Graph_fy11);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#0000cc");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph11 = new TH1F("Graph_Graph11","",100,-7194.501,7199.5);
   Graph_Graph11->SetMinimum(-54.41967);
   Graph_Graph11->SetMaximum(134.3541);
   Graph_Graph11->SetDirectory(nullptr);
   Graph_Graph11->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph11->SetLineColor(ci);
   Graph_Graph11->GetXaxis()->SetLabelFont(42);
   Graph_Graph11->GetXaxis()->SetTitleOffset(1);
   Graph_Graph11->GetXaxis()->SetTitleFont(42);
   Graph_Graph11->GetYaxis()->SetLabelFont(42);
   Graph_Graph11->GetYaxis()->SetTitleFont(42);
   Graph_Graph11->GetZaxis()->SetLabelFont(42);
   Graph_Graph11->GetZaxis()->SetTitleOffset(1);
   Graph_Graph11->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph11);
   
   graph->Draw("l");
   
   Double_t Graph_fx12[2] = { -6100, 6100 };
   Double_t Graph_fy12[2] = { -90, -120 };
   graph = new TGraph(2,Graph_fx12,Graph_fy12);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#cc00cc");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph12 = new TH1F("Graph_Graph12","",100,-7320,7320);
   Graph_Graph12->SetMinimum(-123);
   Graph_Graph12->SetMaximum(-87);
   Graph_Graph12->SetDirectory(nullptr);
   Graph_Graph12->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph12->SetLineColor(ci);
   Graph_Graph12->GetXaxis()->SetLabelFont(42);
   Graph_Graph12->GetXaxis()->SetTitleOffset(1);
   Graph_Graph12->GetXaxis()->SetTitleFont(42);
   Graph_Graph12->GetYaxis()->SetLabelFont(42);
   Graph_Graph12->GetYaxis()->SetTitleFont(42);
   Graph_Graph12->GetZaxis()->SetLabelFont(42);
   Graph_Graph12->GetZaxis()->SetTitleOffset(1);
   Graph_Graph12->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph12);
   
   graph->Draw("l");
   
   Double_t Graph_fx13[2] = { -5995, 6000 };
   Double_t Graph_fy13[2] = { -90.2582, -119.7541 };
   graph = new TGraph(2,Graph_fx13,Graph_fy13);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#cc00cc");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph13 = new TH1F("Graph_Graph13","",100,-7194.5,7199.5);
   Graph_Graph13->SetMinimum(-122.7037);
   Graph_Graph13->SetMaximum(-87.30861);
   Graph_Graph13->SetDirectory(nullptr);
   Graph_Graph13->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph13->SetLineColor(ci);
   Graph_Graph13->GetXaxis()->SetLabelFont(42);
   Graph_Graph13->GetXaxis()->SetTitleOffset(1);
   Graph_Graph13->GetXaxis()->SetTitleFont(42);
   Graph_Graph13->GetYaxis()->SetLabelFont(42);
   Graph_Graph13->GetYaxis()->SetTitleFont(42);
   Graph_Graph13->GetZaxis()->SetLabelFont(42);
   Graph_Graph13->GetZaxis()->SetTitleOffset(1);
   Graph_Graph13->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph13);
   
   graph->Draw("l");
   
   Double_t Graph_fx14[2] = { -6100, 6100 };
   Double_t Graph_fy14[2] = { 420, -500 };
   graph = new TGraph(2,Graph_fx14,Graph_fy14);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#336633");
   graph->SetLineColor(ci);
   
   TH1F *Graph_Graph14 = new TH1F("Graph_Graph14","",100,-7320,7320);
   Graph_Graph14->SetMinimum(-592);
   Graph_Graph14->SetMaximum(512);
   Graph_Graph14->SetDirectory(nullptr);
   Graph_Graph14->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph14->SetLineColor(ci);
   Graph_Graph14->GetXaxis()->SetLabelFont(42);
   Graph_Graph14->GetXaxis()->SetTitleOffset(1);
   Graph_Graph14->GetXaxis()->SetTitleFont(42);
   Graph_Graph14->GetYaxis()->SetLabelFont(42);
   Graph_Graph14->GetYaxis()->SetTitleFont(42);
   Graph_Graph14->GetZaxis()->SetLabelFont(42);
   Graph_Graph14->GetZaxis()->SetTitleOffset(1);
   Graph_Graph14->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph14);
   
   graph->Draw("l");
   
   Double_t Graph_fx15[2] = { -3725.127, 2514.924 };
   Double_t Graph_fy15[2] = { 240.9112, -229.65 };
   graph = new TGraph(2,Graph_fx15,Graph_fy15);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#336633");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph15 = new TH1F("Graph_Graph15","",100,-4349.132,3138.929);
   Graph_Graph15->SetMinimum(-276.7061);
   Graph_Graph15->SetMaximum(287.9674);
   Graph_Graph15->SetDirectory(nullptr);
   Graph_Graph15->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph15->SetLineColor(ci);
   Graph_Graph15->GetXaxis()->SetLabelFont(42);
   Graph_Graph15->GetXaxis()->SetTitleOffset(1);
   Graph_Graph15->GetXaxis()->SetTitleFont(42);
   Graph_Graph15->GetYaxis()->SetLabelFont(42);
   Graph_Graph15->GetYaxis()->SetTitleFont(42);
   Graph_Graph15->GetZaxis()->SetLabelFont(42);
   Graph_Graph15->GetZaxis()->SetTitleOffset(1);
   Graph_Graph15->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph15);
   
   graph->Draw("l");
   
   Double_t Graph_fx16[5] = { -6000, -6000, 6000, 6000, -6000 };
   Double_t Graph_fy16[5] = { -350, 350, 350, -350, -350 };
   graph = new TGraph(5,Graph_fx16,Graph_fy16);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph16 = new TH1F("Graph_Graph16","",100,-8500,8500);
   Graph_Graph16->SetMinimum(-1150);
   Graph_Graph16->SetMaximum(1150);
   Graph_Graph16->SetDirectory(nullptr);
   Graph_Graph16->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph16->SetLineColor(ci);
   Graph_Graph16->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph16->GetXaxis()->SetNdivisions(5);
   Graph_Graph16->GetXaxis()->SetLabelFont(42);
   Graph_Graph16->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph16->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph16->GetXaxis()->SetTitleOffset(1);
   Graph_Graph16->GetXaxis()->SetTitleFont(42);
   Graph_Graph16->GetYaxis()->SetTitle("Y [mm]");
   Graph_Graph16->GetYaxis()->SetLabelFont(42);
   Graph_Graph16->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph16->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph16->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph16->GetYaxis()->SetTitleFont(42);
   Graph_Graph16->GetZaxis()->SetLabelFont(42);
   Graph_Graph16->GetZaxis()->SetTitleOffset(1);
   Graph_Graph16->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph16);
   
   graph->Draw("l");
   
   TLegend *leg = new TLegend(0.14,0.7,0.34,0.95,NULL,"brNDC");
   leg->SetBorderSize(1);
   leg->SetTextSize(0.037);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("","Central","l");

   ci = TColor::GetColor("#cccc00");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("","Extremo1","l");

   ci = TColor::GetColor("#999966");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("","Extremo2","l");

   ci = TColor::GetColor("#00cc00");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("","Random","l");

   ci = TColor::GetColor("#00cccc");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("","Random1","l");

   ci = TColor::GetColor("#0000cc");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("","Random2","l");

   ci = TColor::GetColor("#cc00cc");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   entry=leg->AddEntry("","Fuera","l");

   ci = TColor::GetColor("#336633");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);
   entry->SetMarkerColor(1);
   entry->SetMarkerStyle(21);
   entry->SetMarkerSize(1);
   entry->SetTextFont(42);
   leg->Draw();
   pad1_1__1->Modified();
   pad1__0->cd();
  
// ------------>Primitives in pad: pad1_2
   TPad *pad1_2__2 = new TPad("pad1_2", "pad1_2",0.51,0.01,0.99,0.99);
   pad1_2__2->Draw();
   pad1_2__2->cd();
   pad1_2__2->Range(-11631.58,-0.3614458,10736.84,2.650602);
   pad1_2__2->SetFillColor(0);
   pad1_2__2->SetBorderMode(0);
   pad1_2__2->SetBorderSize(2);
   pad1_2__2->SetLeftMargin(0.14);
   pad1_2__2->SetTopMargin(0.05);
   pad1_2__2->SetBottomMargin(0.12);
   pad1_2__2->SetFrameBorderMode(0);
   pad1_2__2->SetFrameBorderMode(0);
   
   Double_t Graph_fx17[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy17[103] = { 0, 0, 0.04026705, 0.05270462, 0.06980373, 0.09401766, 0.1280315, 0.1763725, 0.245196, 0.340612, 0.4707225, 0.6353966, 0.8292118, 1.036611, 1.238215, 1.416994, 1.56517,
   1.679354, 1.762127, 1.820284, 1.860671, 1.888119, 1.906244, 1.920962, 1.930777, 1.937907, 1.943714, 1.947784, 1.951916, 1.953554, 1.956184, 1.957568, 1.959117,
   1.959825, 1.960757, 1.961601, 1.962268, 1.962742, 1.962641, 1.963438, 1.963282, 1.963731, 1.964237, 1.965353, 1.963826, 1.96424, 1.964431, 1.964201, 1.964661,
   1.964655, 1.965605, 1.96494, 1.965223, 1.964076, 1.964071, 1.964421, 1.964741, 1.964839, 1.963944, 1.963891, 1.963612, 1.964145, 1.96377, 1.963273, 1.96292,
   1.962557, 1.961923, 1.961073, 1.961016, 1.960001, 1.95813, 1.957805, 1.956496, 1.954668, 1.952554, 1.9492, 1.944312, 1.939388, 1.931621, 1.922481, 1.909557,
   1.891512, 1.866058, 1.828256, 1.772891, 1.694356, 1.585628, 1.444035, 1.26981, 1.071838, 0.8635745, 0.6662398, 0.4956286, 0.3602584, 0.2585732, 0.1864209, 0.1349409,
   0.09885986, 0.07340855, 0.05510235, 0.04208903, 0.03249795, 0 };
   graph = new TGraph(103,Graph_fx17,Graph_fy17);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#cccc00");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph17 = new TH1F("Graph_Graph17","",103,-8500,8500);
   Graph_Graph17->SetMinimum(0);
   Graph_Graph17->SetMaximum(2.5);
   Graph_Graph17->SetDirectory(nullptr);
   Graph_Graph17->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph17->SetLineColor(ci);
   Graph_Graph17->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph17->GetXaxis()->SetNdivisions(5);
   Graph_Graph17->GetXaxis()->SetLabelFont(42);
   Graph_Graph17->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph17->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph17->GetXaxis()->SetTitleOffset(1);
   Graph_Graph17->GetXaxis()->SetTitleFont(42);
   Graph_Graph17->GetYaxis()->SetTitle("B [T]");
   Graph_Graph17->GetYaxis()->SetLabelFont(42);
   Graph_Graph17->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph17->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph17->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph17->GetYaxis()->SetTitleFont(42);
   Graph_Graph17->GetZaxis()->SetLabelFont(42);
   Graph_Graph17->GetZaxis()->SetTitleOffset(1);
   Graph_Graph17->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph17);
   
   graph->Draw("al");
   
   Double_t Graph_fx18[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy18[103] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.422575,
   1.589777, 1.712292, 1.798707, 1.855348, 1.893022, 1.918789, 1.938552, 1.951504, 1.961242, 1.967381, 1.971966, 1.975499, 1.978569, 1.979844, 1.9804, 1.981527,
   1.981222, 1.981252, 1.981561, 1.980662, 1.98, 1.979162, 1.97807, 1.977622, 1.975932, 1.97513, 1.973747, 1.972091, 1.970995, 1.969773, 1.967661, 1.966074,
   1.964675, 1.96304, 1.961864, 1.959793, 1.958575, 1.955949, 1.955058, 1.953374, 1.951401, 1.95028, 1.948942, 1.947191, 1.945262, 1.944293, 1.942712, 1.941302,
   1.939616, 1.938558, 1.936781, 1.936243, 1.93426, 1.932245, 1.932063, 1.930544, 1.930059, 1.929333, 1.928093, 1.926772, 1.927464, 1.926866, 1.928907, 1.93162,
   1.938462, 1.951598, 1.975335, 2.008134, 2.029479, 2.007666, 1.911469, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0 };
   graph = new TGraph(103,Graph_fx18,Graph_fy18);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#999966");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph18 = new TH1F("Graph_Graph18","",103,-8500,8500);
   Graph_Graph18->SetMinimum(0);
   Graph_Graph18->SetMaximum(2.5);
   Graph_Graph18->SetDirectory(nullptr);
   Graph_Graph18->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph18->SetLineColor(ci);
   Graph_Graph18->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph18->GetXaxis()->SetNdivisions(5);
   Graph_Graph18->GetXaxis()->SetLabelFont(42);
   Graph_Graph18->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph18->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph18->GetXaxis()->SetTitleOffset(1);
   Graph_Graph18->GetXaxis()->SetTitleFont(42);
   Graph_Graph18->GetYaxis()->SetTitle("B [T]");
   Graph_Graph18->GetYaxis()->SetLabelFont(42);
   Graph_Graph18->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph18->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph18->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph18->GetYaxis()->SetTitleFont(42);
   Graph_Graph18->GetZaxis()->SetLabelFont(42);
   Graph_Graph18->GetZaxis()->SetTitleOffset(1);
   Graph_Graph18->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph18);
   
   graph->Draw("l");
   
   Double_t Graph_fx19[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy19[103] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.999593,
   2.030471, 2.011348, 1.980446, 1.954806, 1.940334, 1.932482, 1.929135, 1.925953, 1.926957, 1.927092, 1.928163, 1.929163, 1.930377, 1.93073, 1.931946, 1.932883,
   1.934131, 1.935207, 1.936428, 1.938185, 1.939483, 1.940299, 1.942441, 1.943569, 1.945527, 1.946483, 1.948555, 1.949838, 1.951554, 1.952891, 1.954938, 1.956087,
   1.958412, 1.95917, 1.961545, 1.963058, 1.964538, 1.966431, 1.967631, 1.969089, 1.970396, 1.971694, 1.973587, 1.974485, 1.975477, 1.976853, 1.978244, 1.979329,
   1.980052, 1.980754, 1.981136, 1.981747, 1.981138, 1.981705, 1.981127, 1.979881, 1.978159, 1.976203, 1.973208, 1.968273, 1.961962, 1.952967, 1.940806, 1.923462,
   1.898633, 1.862676, 1.808947, 1.730218, 1.613082, 1.453558, 1.255764, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0 };
   graph = new TGraph(103,Graph_fx19,Graph_fy19);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#00cc00");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph19 = new TH1F("Graph_Graph19","",103,-8500,8500);
   Graph_Graph19->SetMinimum(0);
   Graph_Graph19->SetMaximum(2.5);
   Graph_Graph19->SetDirectory(nullptr);
   Graph_Graph19->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph19->SetLineColor(ci);
   Graph_Graph19->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph19->GetXaxis()->SetNdivisions(5);
   Graph_Graph19->GetXaxis()->SetLabelFont(42);
   Graph_Graph19->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph19->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph19->GetXaxis()->SetTitleOffset(1);
   Graph_Graph19->GetXaxis()->SetTitleFont(42);
   Graph_Graph19->GetYaxis()->SetTitle("B [T]");
   Graph_Graph19->GetYaxis()->SetLabelFont(42);
   Graph_Graph19->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph19->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph19->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph19->GetYaxis()->SetTitleFont(42);
   Graph_Graph19->GetZaxis()->SetLabelFont(42);
   Graph_Graph19->GetZaxis()->SetTitleOffset(1);
   Graph_Graph19->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph19);
   
   graph->Draw("l");
   
   Double_t Graph_fx20[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy20[103] = { 0, 0, 0.04217101, 0.05504638, 0.0725334, 0.09725188, 0.1315861, 0.1802496, 0.2486812, 0.342675, 0.4707147, 0.6333793, 0.8259825, 1.034753, 1.240765, 1.424865, 1.579357,
   1.699276, 1.785947, 1.846921, 1.887217, 1.915051, 1.934684, 1.948821, 1.958735, 1.966159, 1.97104, 1.975709, 1.978872, 1.980879, 1.983262, 1.984952, 1.984924,
   1.986635, 1.987499, 1.987766, 1.988292, 1.988204, 1.988237, 1.988496, 1.98911, 1.989437, 1.988739, 1.988885, 1.988804, 1.988652, 1.988892, 1.988498, 1.988549,
   1.988228, 1.988047, 1.987576, 1.987314, 1.987191, 1.987218, 1.986635, 1.986572, 1.986876, 1.98619, 1.985369, 1.985402, 1.984435, 1.984208, 1.983828, 1.983353,
   1.982695, 1.981577, 1.980874, 1.979615, 1.978971, 1.977159, 1.975579, 1.973931, 1.971568, 1.96861, 1.964755, 1.959844, 1.954721, 1.946421, 1.936268, 1.922091,
   1.902154, 1.873644, 1.831446, 1.77019, 1.682043, 1.563267, 1.41019, 1.226634, 1.02471, 0.8198743, 0.6306541, 0.4710601, 0.3452719, 0.2508591, 0.1830153, 0.1341079,
   0.09933292, 0.07445787, 0.05643148, 0.04339567, 0.03374064, 0 };
   graph = new TGraph(103,Graph_fx20,Graph_fy20);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#00cccc");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph20 = new TH1F("Graph_Graph20","",103,-8500,8500);
   Graph_Graph20->SetMinimum(0);
   Graph_Graph20->SetMaximum(2.5);
   Graph_Graph20->SetDirectory(nullptr);
   Graph_Graph20->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph20->SetLineColor(ci);
   Graph_Graph20->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph20->GetXaxis()->SetNdivisions(5);
   Graph_Graph20->GetXaxis()->SetLabelFont(42);
   Graph_Graph20->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph20->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph20->GetXaxis()->SetTitleOffset(1);
   Graph_Graph20->GetXaxis()->SetTitleFont(42);
   Graph_Graph20->GetYaxis()->SetTitle("B [T]");
   Graph_Graph20->GetYaxis()->SetLabelFont(42);
   Graph_Graph20->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph20->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph20->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph20->GetYaxis()->SetTitleFont(42);
   Graph_Graph20->GetZaxis()->SetLabelFont(42);
   Graph_Graph20->GetZaxis()->SetTitleOffset(1);
   Graph_Graph20->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph20);
   
   graph->Draw("l");
   
   Double_t Graph_fx21[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy21[103] = { 0, 0, 0.04028122, 0.05218629, 0.06825711, 0.09074401, 0.1216518, 0.164933, 0.2254787, 0.3081253, 0.4211066, 0.5655994, 0.7404154, 0.9355216, 1.134119, 1.319139, 1.479204,
   1.60773, 1.703777, 1.773961, 1.821445, 1.85515, 1.878414, 1.895316, 1.908102, 1.917675, 1.925463, 1.93076, 1.935706, 1.939409, 1.94269, 1.945856, 1.94827,
   1.950099, 1.952893, 1.954287, 1.956025, 1.957754, 1.959565, 1.960541, 1.962206, 1.963117, 1.964372, 1.965771, 1.966934, 1.968125, 1.969445, 1.970117, 1.970956,
   1.971947, 1.973122, 1.973867, 1.974751, 1.975377, 1.976544, 1.977139, 1.978017, 1.979062, 1.979774, 1.980582, 1.980631, 1.98094, 1.981773, 1.982229, 1.983783,
   1.983408, 1.983666, 1.983491, 1.983989, 1.983831, 1.983512, 1.983155, 1.981957, 1.981312, 1.979451, 1.977184, 1.973916, 1.969481, 1.963505, 1.95534, 1.944081,
   1.927919, 1.905049, 1.870667, 1.820515, 1.746527, 1.641457, 1.49987, 1.324173, 1.121987, 0.9078153, 0.7027971, 0.5250132, 0.3831316, 0.2758267, 0.1993955, 0.1445922,
   0.1059967, 0.07878058, 0.05916077, 0.04521318, 0.03490151, 0 };
   graph = new TGraph(103,Graph_fx21,Graph_fy21);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#0000cc");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph21 = new TH1F("Graph_Graph21","",103,-8500,8500);
   Graph_Graph21->SetMinimum(0);
   Graph_Graph21->SetMaximum(2.5);
   Graph_Graph21->SetDirectory(nullptr);
   Graph_Graph21->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph21->SetLineColor(ci);
   Graph_Graph21->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph21->GetXaxis()->SetNdivisions(5);
   Graph_Graph21->GetXaxis()->SetLabelFont(42);
   Graph_Graph21->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph21->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph21->GetXaxis()->SetTitleOffset(1);
   Graph_Graph21->GetXaxis()->SetTitleFont(42);
   Graph_Graph21->GetYaxis()->SetTitle("B [T]");
   Graph_Graph21->GetYaxis()->SetLabelFont(42);
   Graph_Graph21->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph21->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph21->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph21->GetYaxis()->SetTitleFont(42);
   Graph_Graph21->GetZaxis()->SetLabelFont(42);
   Graph_Graph21->GetZaxis()->SetTitleOffset(1);
   Graph_Graph21->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph21);
   
   graph->Draw("l");
   
   Double_t Graph_fx22[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy22[103] = { 0, 0, 0.03871237, 0.05112599, 0.06832868, 0.09316658, 0.1284324, 0.1795395, 0.2531676, 0.3560322, 0.4969126, 0.6726092, 0.873375, 1.080423, 1.273397, 1.436724, 1.566871,
   1.662138, 1.728263, 1.773641, 1.80439, 1.825941, 1.841534, 1.852802, 1.860392, 1.866153, 1.87065, 1.873612, 1.876009, 1.878134, 1.879059, 1.880049, 1.880757,
   1.881709, 1.881056, 1.880877, 1.881319, 1.881286, 1.881374, 1.880682, 1.880702, 1.880484, 1.879968, 1.879887, 1.880354, 1.879241, 1.879106, 1.878827, 1.878598,
   1.878166, 1.878077, 1.877861, 1.877286, 1.877083, 1.876644, 1.876548, 1.876338, 1.876266, 1.875737, 1.875583, 1.875405, 1.875254, 1.874634, 1.874128, 1.874168,
   1.873515, 1.873378, 1.872869, 1.871756, 1.87121, 1.870543, 1.869624, 1.868422, 1.866533, 1.864577, 1.861242, 1.858097, 1.85286, 1.847454, 1.840499, 1.829953,
   1.816224, 1.796991, 1.770745, 1.732628, 1.673624, 1.588559, 1.470141, 1.314845, 1.129418, 0.9239763, 0.7193779, 0.5356126, 0.3868369, 0.2742384, 0.1947704, 0.1387419,
   0.1001355, 0.07336246, 0.05443221, 0.04114605, 0.03148896, 0 };
   graph = new TGraph(103,Graph_fx22,Graph_fy22);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#cc00cc");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph22 = new TH1F("Graph_Graph22","",103,-8500,8500);
   Graph_Graph22->SetMinimum(0);
   Graph_Graph22->SetMaximum(2.5);
   Graph_Graph22->SetDirectory(nullptr);
   Graph_Graph22->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph22->SetLineColor(ci);
   Graph_Graph22->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph22->GetXaxis()->SetNdivisions(5);
   Graph_Graph22->GetXaxis()->SetLabelFont(42);
   Graph_Graph22->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph22->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph22->GetXaxis()->SetTitleOffset(1);
   Graph_Graph22->GetXaxis()->SetTitleFont(42);
   Graph_Graph22->GetYaxis()->SetTitle("B [T]");
   Graph_Graph22->GetYaxis()->SetLabelFont(42);
   Graph_Graph22->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph22->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph22->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph22->GetYaxis()->SetTitleFont(42);
   Graph_Graph22->GetZaxis()->SetLabelFont(42);
   Graph_Graph22->GetZaxis()->SetTitleOffset(1);
   Graph_Graph22->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph22);
   
   graph->Draw("l");
   
   Double_t Graph_fx23[103] = { -6010, -6010, -5890, -5770, -5650, -5530, -5410, -5290, -5170, -5050, -4930, -4810, -4690, -4570, -4450, -4330, -4210,
   -4090, -3970, -3850, -3730, -3610, -3490, -3370, -3250, -3130, -3010, -2890, -2770, -2650, -2530, -2410, -2290,
   -2170, -2050, -1930, -1810, -1690, -1570, -1450, -1330, -1210, -1090, -970, -850, -730, -610, -490, -370,
   -250, -130, -10, 110, 230, 350, 470, 590, 710, 830, 950, 1070, 1190, 1310, 1430, 1550,
   1670, 1790, 1910, 2030, 2150, 2270, 2390, 2510, 2630, 2750, 2870, 2990, 3110, 3230, 3350, 3470,
   3590, 3710, 3830, 3950, 4070, 4190, 4310, 4430, 4550, 4670, 4790, 4910, 5030, 5150, 5270, 5390,
   5510, 5630, 5750, 5870, 5990, 6110 };
   Double_t Graph_fy23[103] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 1.955787, 2.009306, 2.052677, 2.09076, 2.123682, 2.15279, 2.179064, 2.202719, 2.222557, 2.242558, 2.258361, 2.275058,
   2.289237, 2.302619, 2.31398, 2.324289, 2.334558, 2.342234, 2.35067, 2.35591, 2.361636, 2.364898, 2.368076, 2.372157, 2.373417, 2.373322, 2.373122, 2.371705,
   2.370292, 2.368544, 2.36435, 2.359716, 2.353584, 2.346535, 2.340942, 2.333105, 2.323734, 2.313919, 2.303557, 2.292013, 2.279075, 2.264897, 2.250742, 2.234447,
   2.216974, 2.198964, 2.179148, 2.160454, 2.138746, 2.114773, 2.092996, 2.069663, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0 };
   graph = new TGraph(103,Graph_fx23,Graph_fy23);
   graph->SetName("");
   graph->SetTitle("");
   graph->SetFillStyle(1000);

   ci = TColor::GetColor("#336633");
   graph->SetLineColor(ci);
   graph->SetLineWidth(2);
   
   TH1F *Graph_Graph23 = new TH1F("Graph_Graph23","",103,-8500,8500);
   Graph_Graph23->SetMinimum(0);
   Graph_Graph23->SetMaximum(2.5);
   Graph_Graph23->SetDirectory(nullptr);
   Graph_Graph23->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph23->SetLineColor(ci);
   Graph_Graph23->GetXaxis()->SetTitle("Z [mm]");
   Graph_Graph23->GetXaxis()->SetNdivisions(5);
   Graph_Graph23->GetXaxis()->SetLabelFont(42);
   Graph_Graph23->GetXaxis()->SetLabelSize(0.055);
   Graph_Graph23->GetXaxis()->SetTitleSize(0.055);
   Graph_Graph23->GetXaxis()->SetTitleOffset(1);
   Graph_Graph23->GetXaxis()->SetTitleFont(42);
   Graph_Graph23->GetYaxis()->SetTitle("B [T]");
   Graph_Graph23->GetYaxis()->SetLabelFont(42);
   Graph_Graph23->GetYaxis()->SetLabelSize(0.055);
   Graph_Graph23->GetYaxis()->SetTitleSize(0.055);
   Graph_Graph23->GetYaxis()->SetTitleOffset(1.3);
   Graph_Graph23->GetYaxis()->SetTitleFont(42);
   Graph_Graph23->GetZaxis()->SetLabelFont(42);
   Graph_Graph23->GetZaxis()->SetTitleOffset(1);
   Graph_Graph23->GetZaxis()->SetTitleFont(42);
   graph->SetHistogram(Graph_Graph23);
   
   graph->Draw("l");
   pad1_2__2->Modified();
   pad1__0->cd();
   pad1__0->Modified();
   fCanvas->cd();
   fCanvas->Modified();
   fCanvas->SetSelected(fCanvas);
}
