///////////////////////////////////////////////////////////////////////////
// ======================================================================//
// VarSet.h                                                              //
// ======================================================================//
// TTree variables are loaded into these objects. Each sample has        //
// its own VarSet data structure.                                        //
// We also set up a map<TString,FUNCTION> to get feature values based    //
// upon the name of the feature.                                         //
// Has muons, dimuons, jets, electrons, gen muons, etc...                //
// ======================================================================//
///////////////////////////////////////////////////////////////////////////

#ifndef ADD_VARSET
#define ADD_VARSET

#include "DataFormats.h"
#include "EventInfo.h"
#include "MuonInfo.h"
#include "MuPairInfo.h"
#include "JetPairInfo.h"
#include "EleInfo.h"
#include "MhtInfo.h"
#include "MetInfo.h"
#include "SlimJetInfo.h"
#include "GenParentInfo.h"
#include "GenMuonInfo.h"
#include "GenMuPairInfo.h"
#include "TLorentzVector.h"
#include <unordered_map>
#include <map>

class VarSet
{
    public:
        VarSet();
        ~VarSet(){};

        // reco weights for mc
        float eff_wgt;            // use this if you don't match MC to trigger
                                  // scales mc to trigger passed data based on pt,eta

        float isoMu_SF_3;  // match MC to trigger, account for discrepancy from there
        float isoMu_SF_4;  // scale factors change for different eras hence the 3 and 4
        float muID_SF_3;   // match MC to muID, adjust to data from there
        float muID_SF_4;   // ...
        float muIso_SF_3;  // cut mc based on iso, adjust to data from there
        float muIso_SF_4;  // ...

        float sf()
        {
            // average the scale factors for the different eras and multiply all of them
            // to get the net scale factor
            return 0.5*(isoMu_SF_3 + isoMu_SF_4)*0.5*(muID_SF_3 + muID_SF_4)*0.5*(muIso_SF_3 + muIso_SF_4);
        }

        float pu_wgt;      // weight mc based upon PU to match data PU distribution
        

        // reco info
        Int_t nVertices;
        EventInfo* eventInfo = 0;
        MuPairInfo* dimuCand = 0; // this is a pointer to one of the dimu candidates in the vector
                                  // we don't want to copy the object for ~40 million events
                                  // the other objects/primitives are loaded via TBranch->Get();
        MhtInfo* mht = 0;
        MetInfo* met = 0;

        std::vector<MuPairInfo>* recoDimuCands = 0;
        std::vector<MuonInfo>* recoMuons = 0;
        std::vector<EleInfo>* recoElectrons = 0;
        std::vector<SlimJetInfo>* jets = 0;

        std::vector<TLorentzVector> validMuons;
        std::vector<TLorentzVector> validExtraMuons;
        std::vector<TLorentzVector> validElectrons;
        std::vector<TLorentzVector> validJets;
        std::vector<TLorentzVector> validBJets;

        // gen info
        std::vector<GenParentInfo>* genParents = 0;
        std::vector<GenMuonInfo>* genMuons = 0;
        std::vector<GenMuPairInfo>* genDimuons = 0;

        int nPU;
        float lhe_ht;

        // gen weights
        int gen_wgt;

        // cuts for vbf jets
        double cLeadPtMin = 40;
        double cDijetMassMinVBFT = 650;
        double cDijetDeltaEtaMinVBFT = 3.5;

        // set the above cuts if they change
        void setVBFcuts(double leadPtMin, double dijetMassMinVBFT, double dijetDeltaEtaMinVBFT)
        {
            cLeadPtMin = leadPtMin;
            cDijetMassMinVBFT = dijetMassMinVBFT;
            cDijetDeltaEtaMinVBFT = dijetDeltaEtaMinVBFT;
        }

        // index for vbf jets
        int vbf_j0 = -999;
        int vbf_j1 = -999;
 
        // index for our "standard" jets
        int j0 = 0;
        int j1 = 1;

        // map variable string to variable value via function pointers
        // uses functions below
        std::unordered_map<std::string, double(VarSet::*)()> varMap;

        // get the the value for some variable in one of the structs above
        // by name (string). Use the varMap to get the appropriate function
        // of those below.
        // Now we can easily output the values for training TString->Value
        // And evaluate the XML categories via getValue(varTString) > cut
        double getValue(const std::string& name) 
        {
          // Must call the function for this particular instance
          if(varMap[name])
            return (this->*varMap[name])();
          else return -999;
        }

        void setCalibrationType(TString ctype)
        {
          if(ctype == "PF") 
          {
              dimuCand->mass = dimuCand->mass_PF;
              recoMuons->at(dimuCand->iMu1).pt = recoMuons->at(dimuCand->iMu1).pt_PF;
              recoMuons->at(dimuCand->iMu2).pt = recoMuons->at(dimuCand->iMu2).pt_PF;
          }
          else if(ctype == "Roch") 
          {
              dimuCand->mass = dimuCand->mass_Roch;
              recoMuons->at(dimuCand->iMu1).pt = recoMuons->at(dimuCand->iMu1).pt_Roch;
              recoMuons->at(dimuCand->iMu2).pt = recoMuons->at(dimuCand->iMu2).pt_Roch;
          }
          else if(ctype == "KaMu") 
          {
              dimuCand->mass = dimuCand->mass_KaMu;
              recoMuons->at(dimuCand->iMu1).pt = recoMuons->at(dimuCand->iMu1).pt_KaMu;
              recoMuons->at(dimuCand->iMu2).pt = recoMuons->at(dimuCand->iMu2).pt_KaMu;
          }
        }

        // get jets that represent vbf jets
        void setVBFjets()
        {
            vbf_j0 = 0;
            vbf_j1 = 1;
            double mjj_max = -999;

            for(unsigned int i=0; i<validJets.size(); i++) 
            {
                if(!(validJets[i].Pt() > cLeadPtMin)) break;
                for(unsigned int j=i+1; j<validJets.size(); j++) 
                {
                    double dEtajj = TMath::Abs(validJets[i].Eta() - validJets[j].Eta());
                    double mjj = (validJets[i] + validJets[j]).M();
                    if(mjj > mjj_max)
                    {
                        vbf_j0 = i;
                        vbf_j1 = j;
                    }
                    if(mjj > cDijetMassMinVBFT && dEtajj > cDijetDeltaEtaMinVBFT)
                    {
                        vbf_j0 = i;
                        vbf_j1 = j;
                        return; 
                    }
                }
            }
        }

        // standard jets will be the two corresponding to the max mjj value 
        void setJets()
        {
            double mjj_max = -999;
            j0 = 0;
            j1 = 1;

            for(unsigned int i=0; i<validJets.size(); i++) 
            {
                for(unsigned int j=i+1; j<validJets.size(); j++) 
                {
                    double dEtajj = TMath::Abs(validJets[i].Eta() - validJets[j].Eta());
                    double mjj = (validJets[i] + validJets[j]).M();
                    if(mjj > mjj_max)
                    {
                        j0 = i;
                        j1 = j;
                    }
                }
            }
        }

        // muon variables
        // indexed by 1,2 in analyzer instead of 0,1 so there is a mismatch in naming convention here
        double dimu_pt()   {  return dimuCand->pt;                        };
        double mu1_pt()    {  return recoMuons->at(dimuCand->iMu1).pt;    };
        double mu2_pt()    {  return recoMuons->at(dimuCand->iMu2).pt;    };
        double mu1_eta()   {  return recoMuons->at(dimuCand->iMu1).eta;   };
        double mu2_eta()   {  return recoMuons->at(dimuCand->iMu2).eta;   };
        double mu_res_eta(){  return (TMath::Abs(mu1_eta()) + TMath::Abs(mu2_eta()))/2;   };

        // measure of the muon phi separation in the parent's rest frame
        double phi_star()
        {
            double phi_star = 0;
            double mu_dPhi = TMath::Abs(recoMuons->at(dimuCand->iMu1).phi - recoMuons->at(dimuCand->iMu2).phi);
            if(mu_dPhi > TMath::Pi()) mu_dPhi = 2*TMath::Pi() - mu_dPhi;
            double phiACOP = TMath::Pi() - mu_dPhi;
            double thetaStarEta = TMath::ACos(TMath::TanH((recoMuons->at(dimuCand->iMu1).eta - recoMuons->at(dimuCand->iMu2).eta)/2));
            phi_star = TMath::Tan(phiACOP/2)*TMath::Sin(thetaStarEta);
            return phi_star;
        };

        // jet variables
        double jet0_pt() { return (validJets.size()>=1)?validJets[j0].Pt():-999;  };
        double jet1_pt() { return (validJets.size()>=2)?validJets[j1].Pt():-999;  };
        double jet0_eta(){ return (validJets.size()>=1)?validJets[j0].Eta():-999; };
        double jet1_eta(){ return (validJets.size()>=2)?validJets[j1].Eta():-999; };

        double m_jj()        { return (validJets.size()>=2)?(validJets[j0]+validJets[j1]).M():-999; };
        double dEta_jj()     { return (validJets.size()>=2)?TMath::Abs(validJets[j0].Eta()-validJets[j1].Eta()):-999; };
        double dEta_jj_mumu(){ return (validJets.size()>=2)?TMath::Abs((validJets[j0]+validJets[j1]).Eta()-dimuCand->eta):-999; };

        double dPhi_jj_mumu()    
        { 
            if(validJets.size() < 2) return -999;
            double dphi = TMath::Abs((validJets[j0] + validJets[j1]).Phi() - dimuCand->phi);
            if(dphi > TMath::Pi()) dphi = 2*TMath::Pi() - dphi;
        };
        double zep()
        { 
            if(validJets.size() < 2) return -999; 
            double meanEta = (validJets[j0].Eta() +validJets[j1].Eta())/2;
            return validJets[j0].Eta() - meanEta;
        };

        // vbf jet variables
        double vbf_jet0_pt() { return (validJets.size()>=1)?validJets[vbf_j0].Pt():-999;  };
        double vbf_jet1_pt() { return (validJets.size()>=2)?validJets[vbf_j1].Pt():-999;  };
        double vbf_jet0_eta(){ return (validJets.size()>=1)?validJets[vbf_j0].Eta():-999; };
        double vbf_jet1_eta(){ return (validJets.size()>=2)?validJets[vbf_j1].Eta():-999; };

        double vbf_m_jj()        { return (validJets.size()>=2)?(validJets[vbf_j0]+validJets[vbf_j1]).M():-999; };
        double vbf_dEta_jj()     { return (validJets.size()>=2)?TMath::Abs(validJets[vbf_j0].Eta()-validJets[vbf_j1].Eta()):-999; };
        double vbf_dEta_jj_mumu(){ return (validJets.size()>=2)?TMath::Abs((validJets[vbf_j0]+validJets[vbf_j1]).Eta()-dimuCand->eta):-999; };

        // bjet variables
        double bjet0_pt() { return (validBJets.size()>=1)?validBJets[0].Pt():-999;  };
        double bjet1_pt() { return (validBJets.size()>=2)?validBJets[1].Pt():-999;  };
        double bjet0_eta(){ return (validBJets.size()>=1)?validBJets[0].Eta():-999; };
        double bjet1_eta(){ return (validBJets.size()>=2)?validBJets[1].Eta():-999; };

        double m_bb()   { return (validBJets.size()>=2)?(validBJets[0]+validBJets[1]).M():-999; };
        double dEta_bb(){ return (validBJets.size()>=2)?TMath::Abs(validBJets[0].Eta()-validBJets[1].Eta()):-999; };

        double vbf_dPhi_jj_mumu()    
        { 
            if(validJets.size() < 2) return -999;
            double dphi = TMath::Abs((validJets[vbf_j0] + validJets[vbf_j1]).Phi() - dimuCand->phi);
            if(dphi > TMath::Pi()) dphi = 2*TMath::Pi() - dphi;
        };

        double vbf_zep()
        { 
            if(validJets.size() < 2) return -999; 
            double meanEta = (validJets[vbf_j0].Eta() +validJets[vbf_j1].Eta())/2;
            return validJets[vbf_j0].Eta() - meanEta;
        };

        // # variables
        double N_valid_jets()         { return validJets.size();          };
        double N_valid_bjets()        { return validBJets.size();         };
        double N_valid_extra_muons()  { return validExtraMuons.size();    };
        double N_valid_electrons()    { return validElectrons.size();     };
        double N_valid_extra_leptons(){ return validExtraMuons.size() + validElectrons.size(); };

        // MET
        double MET(){ return met->pt; };
        double MHT(){ return mht->pt; };

        double extra_muon0_pt() { return (validExtraMuons.size()>=1)?validExtraMuons[0].Pt():-999; };
        double extra_muon1_pt() { return (validExtraMuons.size()>=2)?validExtraMuons[1].Pt():-999; };
        double extra_muon0_eta(){ return (validExtraMuons.size()>=1)?validExtraMuons[0].Eta():-999; };
        double extra_muon1_eta(){ return (validExtraMuons.size()>=2)?validExtraMuons[1].Eta():-999; };

        double electron0_pt() { return (validElectrons.size()>=1)?validElectrons[0].Pt():-999; };
        double electron1_pt() { return (validElectrons.size()>=2)?validElectrons[1].Pt():-999; };
        double electron0_eta(){ return (validElectrons.size()>=1)?validElectrons[0].Eta():-999; };
        double electron1_eta(){ return (validElectrons.size()>=2)?validElectrons[1].Eta():-999; };

        double mT_b_MET()
        { 
            if(validBJets.size() < 1) return -999;
            TLorentzVector metv(met->pt*TMath::Cos(met->phi), met->pt*TMath::Cos(met->phi), 0, met->pt);                  
            TLorentzVector bjet = validBJets[0];
            TLorentzVector bjet_t(bjet.Px(), bjet.Py(), 0, bjet.Et());
            TLorentzVector bmet_t = metv + bjet_t;
            return bmet_t.M();
        };

};

#endif
