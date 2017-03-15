#include "JetPairInfo.h"

void JetPairInfo::init() {

  iJet1 = -999;
  iJet2 = -999;
  
  mass = -999;
  pt   = -999;
  eta  = -999;
  phi  = -999;
  dR   = -999;
  dEta = -999;
  dPhi = -999;

} // End void JetPairInfo::init()
///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

Float_t JetPairInfo::getMass()
{
    return mass;
}

///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

TLorentzVector JetPairInfo::get4vec()
{
    TLorentzVector v;
    v.SetPtEtaPhiM(pt, eta, phi, getMass());
    return v;
}

///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

TString JetPairInfo::outputInfo()
{
    TString s = Form("pt: %7.3f, eta: %7.3f, phi: %7.3f, mass: %7.3f", 
                      pt, eta, phi, mass);
    return s;
}

///////////////////////////////////////////////////////////
//--------------------------------------------------------
///////////////////////////////////////////////////////////

Double_t JetPairInfo::iso()
{
    return 0.0;
}