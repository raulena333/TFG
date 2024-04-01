#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <map>
#include <numeric>
#include <TLatex.h>
#include "TRestAxionMagneticField.h"
#include "TRestAxionBufferGas.h"
#include "TRestAxionField.h"

Int_t Test(){

    TRestAxionMagneticField *field = new TRestAxionMagneticField("fields.rml", "babyIAXO_2024_cutoff");

    std::vector<TVector3> startPoints = {
        TVector3(250, 0, -6100),
        TVector3(350, 350, -6100),
        TVector3(-350, -350, -6100),
        TVector3(-150, 20, -6100),
        TVector3(-20, 220, -6100),
        TVector3(-50, -90, -6100),
        TVector3(-150, 420, -6100)
    };

    std::vector<TVector3> endPoints = {
        TVector3(-250, 0, 6100),
        TVector3(-350, -350, 6100),
        TVector3(350, 350, 6100),
        TVector3(-120, 70, 6100),
        TVector3(-100, -170, 6100),
        TVector3(70, -120, 6100),
        TVector3(-270, -500, 6100)
    };

    field->DrawTracks(startPoints, endPoints, 100);
    return 0;
}