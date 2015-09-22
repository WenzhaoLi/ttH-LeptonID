// -*- C++ -*-
//
// Package:    ttH/LeptonIdentifier
// Class:      LeptonIdentifier
// 
/**\class LeptonIdentifier LeptonIdentifier.cc ttH/LeptonID/plugins/LeptonIdentifier.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Matthias Wolf
//         Created:  Mon, 27 Jul 2015 12:22:46 GMT
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

#include "MiniAOD/MiniAODHelper/interface/MiniAODHelper.h"

#include "TMVA/Reader.h"

#include "../interface/EGammaMvaEleEstimatorFWLite.h"

//
// class declaration
//

enum ID {
   preselection,
   looseCut,
   looseMVA,
   tightCut,
   tightMVA
};

class LeptonIdentifier : public edm::EDProducer {
   public:
      explicit LeptonIdentifier(const edm::ParameterSet&);
      ~LeptonIdentifier();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      bool passes(const pat::Electron& e, ID id);
      bool passes(const pat::Muon& mu, ID id);

      float mva(const pat::Muon& mu);
      float mva(const pat::Electron& ele);

      // ----------member data ---------------------------
      MiniAODHelper helper_;

      EGammaMvaEleEstimatorFWLite* mvaID_;

      TMVA::Reader* mu_reader_high_b_;
      TMVA::Reader* mu_reader_high_e_;
      TMVA::Reader* mu_reader_medium_b_;
      TMVA::Reader* mu_reader_medium_e_;
      TMVA::Reader* mu_reader_low_;
      TMVA::Reader* ele_reader_high_cb_;
      TMVA::Reader* ele_reader_high_fb_;
      TMVA::Reader* ele_reader_high_ec_;
      TMVA::Reader* ele_reader_medium_cb_;
      TMVA::Reader* ele_reader_medium_fb_;
      TMVA::Reader* ele_reader_medium_ec_;
      TMVA::Reader* ele_reader_low_;

      Float_t varneuRelIso;
      Float_t varchRelIso;
      Float_t varjetDR_in;
      Float_t varjetPtRatio_in;
      Float_t varjetBTagCSV_in;
      Float_t varsip3d;
      Float_t varmvaId;
      Float_t vardxy;
      Float_t vardz;
      Float_t varSegCompat;

      edm::EDGetTokenT<double> rho_token_;
      edm::EDGetTokenT<pat::PackedCandidateCollection> packedCand_token_;
      edm::EDGetTokenT<pat::ElectronCollection> ele_token_;
      edm::EDGetTokenT<pat::JetCollection> jet_token_;
      edm::EDGetTokenT<pat::MuonCollection> mu_token_;
      edm::EDGetTokenT<reco::VertexCollection> vtx_token_;

      reco::Vertex vertex_;
      pat::JetCollection jets_;

      double mu_minpt_;
      double ele_minpt_;
};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
LeptonIdentifier::LeptonIdentifier(const edm::ParameterSet& config) :
   mu_minpt_(config.getParameter<double>("muonMinPt")),
   ele_minpt_(config.getParameter<double>("electronMinPt"))
{
   produces<pat::ElectronCollection>();
   produces<pat::MuonCollection>();

   rho_token_ = consumes<double>(edm::InputTag("fixedGridRhoFastjetAll"));
   packedCand_token_ = consumes<pat::PackedCandidateCollection>(edm::InputTag("packedPFCandidates"));
   ele_token_ = consumes<pat::ElectronCollection>(config.getParameter<edm::InputTag>("electrons"));
   jet_token_ = consumes<pat::JetCollection>(config.getParameter<edm::InputTag>("jets"));
   mu_token_ = consumes<pat::MuonCollection>(config.getParameter<edm::InputTag>("muons"));
   vtx_token_ = consumes<reco::VertexCollection>(edm::InputTag("offlineSlimmedPrimaryVertices"));

   // Who gives a FUCK about these parameters?  They are not used in the
   // methods we access, which could be spun off, anyways.
   helper_.SetUp("2015_74x", 666, analysisType::LJ, false);

   mvaID_ = new EGammaMvaEleEstimatorFWLite();
   bool useBinnedVersion_ = true;
   std::string method_ = "BDT";
   EGammaMvaEleEstimatorFWLite::MVAType type_ = EGammaMvaEleEstimatorFWLite::kNonTrigPhys14;
   std::vector<std::string> mvaWeightFiles_ = {
      "EgammaAnalysis/ElectronTools/data/PHYS14/EIDmva_EB1_5_oldscenario2phys14_BDT.weights.xml",
      "EgammaAnalysis/ElectronTools/data/PHYS14/EIDmva_EB2_5_oldscenario2phys14_BDT.weights.xml",
      "EgammaAnalysis/ElectronTools/data/PHYS14/EIDmva_EE_5_oldscenario2phys14_BDT.weights.xml",
      "EgammaAnalysis/ElectronTools/data/PHYS14/EIDmva_EB1_10_oldscenario2phys14_BDT.weights.xml",
      "EgammaAnalysis/ElectronTools/data/PHYS14/EIDmva_EB2_10_oldscenario2phys14_BDT.weights.xml",
      "EgammaAnalysis/ElectronTools/data/PHYS14/EIDmva_EE_10_oldscenario2phys14_BDT.weights.xml"
   };

   mvaID_->initialize(method_, type_, useBinnedVersion_, mvaWeightFiles_);

   ele_reader_high_cb_ = new TMVA::Reader( "!Color:!Silent" );
   ele_reader_high_fb_ = new TMVA::Reader( "!Color:!Silent" );
   ele_reader_high_ec_ = new TMVA::Reader( "!Color:!Silent" );
   ele_reader_medium_cb_ = new TMVA::Reader( "!Color:!Silent" );
   ele_reader_medium_fb_ = new TMVA::Reader( "!Color:!Silent" );
   ele_reader_medium_ec_ = new TMVA::Reader( "!Color:!Silent" );
   ele_reader_low_ = new TMVA::Reader( "!Color:!Silent" );

   mu_reader_high_b_ = new TMVA::Reader( "!Color:!Silent" );
   mu_reader_high_e_ = new TMVA::Reader( "!Color:!Silent" );
   mu_reader_medium_b_ = new TMVA::Reader( "!Color:!Silent" );
   mu_reader_medium_e_ = new TMVA::Reader( "!Color:!Silent" );
   mu_reader_low_ = new TMVA::Reader( "!Color:!Silent" );

   std::vector<TMVA::Reader*> ele_mvas = {
      ele_reader_high_cb_,
      ele_reader_high_fb_,
      ele_reader_high_ec_,
      ele_reader_medium_cb_,
      ele_reader_medium_fb_,
      ele_reader_medium_ec_,
      ele_reader_low_
   };

   for (auto& m: ele_mvas) {
      m->AddVariable( "LepGood_relIso03-LepGood_chargedHadRelIso03", &varneuRelIso );
      m->AddVariable( "LepGood_chargedHadRelIso03", &varchRelIso );
      m->AddVariable( "min(LepGood_jetDR,0.5)", &varjetDR_in );
      m->AddVariable( "min(LepGood_jetPtRatio,1.5)", &varjetPtRatio_in );
      m->AddVariable( "max(LepGood_jetBTagCSV,0)", &varjetBTagCSV_in );
      m->AddVariable( "LepGood_sip3d", &varsip3d );
      m->AddVariable( "log(abs(LepGood_dxy))", &vardxy );
      m->AddVariable( "log(abs(LepGood_dz))", &vardz );
      m->AddVariable( "LepGood_mvaIdPhys14", &varmvaId );
   }

   std::vector<TMVA::Reader*> mu_mvas = {
      mu_reader_high_b_,
      mu_reader_high_e_,
      mu_reader_medium_b_,
      mu_reader_medium_e_,
      mu_reader_low_
   };

   for (auto& m: mu_mvas) {
      m->AddVariable( "LepGood_relIso03-LepGood_chargedHadRelIso03", &varneuRelIso );
      m->AddVariable( "LepGood_chargedHadRelIso03", &varchRelIso );
      m->AddVariable( "min(LepGood_jetDR,0.5)", &varjetDR_in );
      m->AddVariable( "min(LepGood_jetPtRatio,1.5)", &varjetPtRatio_in );
      m->AddVariable( "max(LepGood_jetBTagCSV,0)", &varjetBTagCSV_in );
      m->AddVariable( "LepGood_sip3d", &varsip3d );
      m->AddVariable( "log(abs(LepGood_dxy))", &vardxy );
      m->AddVariable( "log(abs(LepGood_dz))", &vardz );
      m->AddVariable( "LepGood_segmentCompatibility", &varSegCompat );
   }

   const std::string base = std::string(getenv("CMSSW_BASE")) + "/src/CMGTools/TTHAnalysis/data/leptonMVA/tth";

   mu_reader_high_b_->BookMVA("BDTG method", base + "/mu_pteta_high_b_BDTG.weights.xml");
   mu_reader_high_e_->BookMVA("BDTG method", base + "/mu_pteta_high_e_BDTG.weights.xml");
   mu_reader_medium_b_->BookMVA("BDTG method", base + "/mu_pteta_medium_b_BDTG.weights.xml");
   mu_reader_medium_e_->BookMVA("BDTG method", base + "/mu_pteta_medium_e_BDTG.weights.xml");
   mu_reader_low_->BookMVA("BDTG method", base + "/mu_pteta_low_BDTG.weights.xml");

   ele_reader_high_cb_->BookMVA("BDTG method", base + "/el_pteta_high_cb_BDTG.weights.xml");
   ele_reader_high_fb_->BookMVA("BDTG method", base + "/el_pteta_high_fb_BDTG.weights.xml");
   ele_reader_high_ec_->BookMVA("BDTG method", base + "/el_pteta_high_ec_BDTG.weights.xml");
   ele_reader_medium_cb_->BookMVA("BDTG method", base + "/el_pteta_medium_cb_BDTG.weights.xml");
   ele_reader_medium_fb_->BookMVA("BDTG method", base + "/el_pteta_medium_fb_BDTG.weights.xml");
   ele_reader_medium_ec_->BookMVA("BDTG method", base + "/el_pteta_medium_ec_BDTG.weights.xml");
   ele_reader_low_->BookMVA("BDTG method", base + "/el_pteta_low_BDTG.weights.xml");
}


LeptonIdentifier::~LeptonIdentifier()
{
   delete mvaID_;

   delete ele_reader_high_cb_;
   delete ele_reader_high_fb_;
   delete ele_reader_high_ec_;
   delete ele_reader_medium_cb_;
   delete ele_reader_medium_fb_;
   delete ele_reader_medium_ec_;
   delete ele_reader_low_;

   delete mu_reader_high_b_;
   delete mu_reader_high_e_;
   delete mu_reader_medium_b_;
   delete mu_reader_medium_e_;
   delete mu_reader_low_;
}


//
// member functions
//

float
LeptonIdentifier::mva(const pat::Muon& mu)
{
  varchRelIso = mu.userFloat("chargedRelIso");
  varneuRelIso = mu.userFloat("neutralRelIso");
  varjetDR_in = mu.userFloat("nearestJetDr");
  varjetPtRatio_in = mu.userFloat("nearestJetPtRatio");
  varjetBTagCSV_in = mu.userFloat("nearestJetCsv");
  varsip3d = mu.userFloat("sip3D");
  vardxy = log(mu.userFloat("dxy"));
  vardz = log(mu.userFloat("dz"));
  varSegCompat = mu.segmentCompatibility();

   if (mu.pt() <= 10){
      return mu_reader_low_->EvaluateMVA( "BDTG method" );
   } else if (mu.pt() > 10 && mu.pt() <= 25 && fabs(mu.eta()) < 1.5) {
      return mu_reader_medium_b_->EvaluateMVA( "BDTG method" );
   } else if (mu.pt() > 10 && mu.pt() <= 25 && fabs(mu.eta()) >= 1.5) {
      return mu_reader_medium_e_->EvaluateMVA( "BDTG method" );
   } else if (mu.pt() > 25 && fabs(mu.eta()) < 1.5) {
      return mu_reader_high_b_->EvaluateMVA( "BDTG method" );
   } else if (mu.pt() > 25 && fabs(mu.eta()) >= 1.5) {
      return mu_reader_high_e_->EvaluateMVA( "BDTG method" );
   } else {
      return -99999.;
   }
}

float
LeptonIdentifier::mva(const pat::Electron& ele)
{
  varneuRelIso = ele.userFloat("neutralRelIso");
  varchRelIso = ele.userFloat("chargedRelIso");
  varjetDR_in = ele.userFloat("nearestJetDr");
  varjetPtRatio_in = ele.userFloat("nearestJetPtRatio");
  varjetBTagCSV_in = ele.userFloat("nearestJetCsv");
  varsip3d = ele.userFloat("sip3D");
  vardxy = log(ele.userFloat("dxy"));
  vardz = log(ele.userFloat("dz"));
  varmvaId = ele.userFloat("eleMvaId");
  
  float lepMVA;
  
  if (ele.pt() <= 10){
    lepMVA = ele_reader_low_->EvaluateMVA( "BDTG method" );
  } else if (ele.pt() > 10 && ele.pt() <= 25 && fabs(ele.eta()) < 0.8){
    lepMVA = ele_reader_medium_cb_->EvaluateMVA( "BDTG method" );
  } else if (ele.pt() > 10 && ele.pt() <= 25 && fabs(ele.eta()) >= 0.8 && fabs(ele.eta()) < 1.479){
    lepMVA = ele_reader_medium_fb_->EvaluateMVA( "BDTG method" );
  } else if (ele.pt() > 10 && ele.pt() <= 25 && fabs(ele.eta()) >= 1.479) {
    lepMVA = ele_reader_medium_ec_->EvaluateMVA( "BDTG method" );
  } else if (ele.pt() > 25 && fabs(ele.eta()) < 0.8) {
    lepMVA = ele_reader_high_cb_->EvaluateMVA( "BDTG method" );
  } else if (ele.pt() > 25 && fabs(ele.eta()) >= 0.8 && fabs(ele.eta()) < 1.479) {
      lepMVA = ele_reader_high_fb_->EvaluateMVA( "BDTG method" );
  } else if (ele.pt() > 25 && fabs(ele.eta()) >= 1.479) {
    lepMVA = ele_reader_high_ec_->EvaluateMVA( "BDTG method" );
  } else {
    lepMVA = -99999.;
  }
  
  return lepMVA;
}

// ------------ id functions ------------
bool
LeptonIdentifier::passes(const pat::Muon& mu, ID id)
{
   double minMuonPt = 5.0; // iMinPt;

   bool passesKinematics = false;
   bool passesIso        = false;
   bool passesID         = false;

   bool passesMuonBestTrackID = false;
   bool mediumID              = false;
   bool goodGlb               = false;
   bool passesCuts = false;

   switch (id) {
      case looseMVA:
         passesKinematics = true;
         passesIso = true;
         goodGlb = (mu.isGlobalMuon() &&  mu.userFloat("normalizedChiSq") < 3
		    && mu.userFloat("localChiSq") < 12 && mu.userFloat("trackKink") < 20);
         mediumID = (mu.userFloat("validFraction") >= 0.8 &&
               mu.segmentCompatibility() >= (goodGlb ? 0.303 : 0.451));
         passesID = (mva(mu) > 0.5 && mediumID );
         break;
      case tightMVA:
         passesKinematics = true;
         passesIso = true;
         goodGlb = (mu.isGlobalMuon() &&  mu.userFloat("normalizedChiSq") < 3
		    && mu.userFloat("localChiSq") < 12 && mu.userFloat("trackKink") < 20);
         mediumID = (mu.userFloat("validFraction") >= 0.8 &&
               mu.segmentCompatibility() >= (goodGlb ? 0.303 : 0.451));
         passesID = (mva(mu) > 0.8 && mediumID );
         break;
      case looseCut:
         passesKinematics = ((mu.pt() >= minMuonPt) && (fabs(mu.eta()) < 2.4));
         passesIso = (mu.userFloat("relIso") < 0.5);
         if( mu.innerTrack().isAvailable() ){
	   passesMuonBestTrackID = (mu.userFloat("dxy") < 0.05 && mu.userFloat("dz") < 0.1);
         }
         passesID = (passesMuonBestTrackID && (mu.isGlobalMuon() || mu.isTrackerMuon()) && mu.isPFMuon());
         break;
      case tightCut:
         passesKinematics = ((mu.pt() >= minMuonPt) && (fabs(mu.eta()) < 2.4));
         passesIso = (mu.userFloat("relIso") < 0.1);

         if( mu.innerTrack().isAvailable() && mu.globalTrack().isAvailable() ){
	    passesMuonBestTrackID = (mu.userFloat("dxy") < 0.05 && mu.userFloat("dz") < 0.1);
            passesCuts = (mu.isGlobalMuon() && mu.isPFMuon() && 
			  mu.userFloat("normalizedChiSq") < 10. &&
			  mu.userFloat("numValidMuonHits") > 0 &&
			  mu.numberOfMatchedStations() > 1 &&
			  mu.userFloat("numValidPixelHits") > 0 &&
			  mu.userFloat("trackerLayersWithMeasurement") > 5 && 
			  mu.userFloat("sip3D") < 4.);
         }
         passesID = (passesMuonBestTrackID && passesCuts);
         break;
      case preselection:
         passesKinematics = ((mu.pt() > minMuonPt) && (fabs(mu.eta()) < 2.4));
         passesIso = (mu.userFloat("relIso") < 0.5);
         if( mu.innerTrack().isAvailable() ){
	   passesMuonBestTrackID = ( mu.userFloat("dxy") < 0.05	&& mu.userFloat("dz") < 0.1);
         }
         passesID = (( mu.isGlobalMuon() || mu.isTrackerMuon() ) && mu.isPFMuon() && passesMuonBestTrackID );
         break;
   }

   return (passesKinematics && passesIso && passesID);
}

bool
LeptonIdentifier::passes(const pat::Electron& ele, ID id)
{
   double minElectronPt = 5; // iMinPt;

   // Be skeptical about this electron making it through
   bool passesKinematics	= false;
   bool passesIso        = false;
   bool passesID         = false;

   bool passGsfTrackID = false;
   bool passesCuts = false; 

   bool passesMVA = false;

   double eleMvaNonTrig = ele.userFloat("eleMvaId");
   float scEta = ele.userFloat("superClusterEta");

   switch(id){
   case looseMVA:
     passesKinematics = true;
     passesIso = true;
     passesID = (mva(ele) > 0.5 && ele.userFloat("numMissingHits") == 0 && ele.passConversionVeto());
     break;
   case tightMVA:
     passesKinematics = true;
     passesIso = true;
     passesID = (mva(ele) > 0.8 && ele.userFloat("numMissingHits") == 0 && ele.passConversionVeto());
     break;
   case looseCut:
     passesKinematics = ((ele.pt() >= minElectronPt) && (fabs(ele.eta()) < 2.5));
     passGsfTrackID = ( ele.userFloat("dxy") < 0.05 && ele.userFloat("dz") < 0.1 && ele.userFloat("numMissingHits") <= 1 );
     passesIso        =  (ele.userFloat("relIso") < 0.5);
     if (scEta <= 1.479)
       {
	 passesCuts = ( fabs(ele.deltaEtaSuperClusterTrackAtVtx()) < 0.007 &&
			fabs(ele.deltaPhiSuperClusterTrackAtVtx()) < 0.8 &&
			ele.full5x5_sigmaIetaIeta() < 0.01 &&
			ele.hadronicOverEm() < 0.15
			);
       }
     else if (scEta > 1.479 && scEta < 2.5)
       {
	 passesCuts = ( fabs(ele.deltaEtaSuperClusterTrackAtVtx()) < 0.01 &&
			fabs(ele.deltaPhiSuperClusterTrackAtVtx()) < 0.7 &&
			ele.full5x5_sigmaIetaIeta() < 0.03
			);
       }
     passesID = (passesCuts && passGsfTrackID);
     break;
   case tightCut:
     passesKinematics = ((ele.pt() >= minElectronPt) && (fabs(ele.eta()) < 2.5));
     passGsfTrackID = ( ele.userFloat("dxy") < 0.05 &&
			ele.userFloat("dz") < 0.1 &&
			ele.isGsfCtfScPixChargeConsistent() &&
			ele.userFloat("numMissingHits") == 0 && 
			ele.userFloat("sip3D") < 4 );
     passesIso        =  (ele.userFloat("relIso") < 0.1);
     
     if (scEta <= 1.479) 
       {
	 passesCuts = ( fabs(ele.deltaEtaSuperClusterTrackAtVtx()) < 0.004 &&
			fabs(ele.deltaPhiSuperClusterTrackAtVtx()) < 0.06 &&
			ele.full5x5_sigmaIetaIeta() < 0.01 &&
			ele.hadronicOverEm() < 0.12);
       }
     else if (scEta > 1.479 && scEta < 2.5)
       {
	 passesCuts = ( fabs(ele.deltaEtaSuperClusterTrackAtVtx()) < 0.007 &&
			fabs(ele.deltaPhiSuperClusterTrackAtVtx()) < 0.03 &&
			ele.full5x5_sigmaIetaIeta() < 0.03 && 
			ele.hadronicOverEm() < 0.10);
       }
     passesID = (passesCuts && passGsfTrackID);
     break;
   case preselection:
     //Phys14 MVA ID (only for pT > 10 GeV) for now
     if (ele.pt() > std::max(10., minElectronPt)) {
       if ( scEta < 0.8) passesMVA = ( eleMvaNonTrig > 0.35 );
       else if ( scEta < 1.479) passesMVA = ( eleMvaNonTrig > 0.2 );
       else passesMVA = ( eleMvaNonTrig > -0.52 );
     }
     if (ele.gsfTrack().isAvailable()) {
       passGsfTrackID = ( ele.userFloat("dxy") < 0.05 && ele.userFloat("dz") < 0.1 && ele.userFloat("numMissingHits") <= 1 );
     }
     passesKinematics = ((ele.pt() > minElectronPt) && (fabs(ele.eta()) < 2.5));
     passesIso = ele.userFloat("relIso") < 0.5;
     passesID = (passGsfTrackID && passesMVA);
     break;
   }
   
   return (passesKinematics && passesIso && passesID);
}

// ------------ method called to produce the data  ------------
void
LeptonIdentifier::produce(edm::Event& event, const edm::EventSetup& setup)
{
   std::unique_ptr<pat::ElectronCollection> eles(new pat::ElectronCollection());
   std::unique_ptr<pat::MuonCollection> mus(new pat::MuonCollection());

   edm::Handle<double> rho;
   edm::Handle<pat::PackedCandidateCollection> packedCands;
   edm::Handle<pat::ElectronCollection> input_ele;
   edm::Handle<pat::JetCollection> input_jet;
   edm::Handle<pat::MuonCollection> input_mu;
   edm::Handle<reco::VertexCollection> input_vtx;

   event.getByToken(rho_token_, rho);
   event.getByToken(packedCand_token_, packedCands);
   event.getByToken(ele_token_, input_ele);
   event.getByToken(jet_token_, input_jet);
   event.getByToken(mu_token_, input_mu);
   event.getByToken(vtx_token_, input_vtx);

   helper_.SetRho(*rho);
   helper_.SetPackedCandidates(*packedCands);

   // determine primary vertex
   for (const auto& v: *input_vtx) {
      if (!v.isFake() && v.ndof() >= 4 && abs(v.z()) <= 24. && abs(v.position().Rho()) <= 2.) {
         helper_.SetVertex(v);
         vertex_ = v;
         break;
      }
   }

   const JetCorrector* corrector = JetCorrector::getJetCorrector("ak4PFchsL1L2L3", setup);
   helper_.SetJetCorrector(corrector);

   auto raw_jets = helper_.GetUncorrectedJets(*input_jet);
   auto corr_jets = helper_.GetCorrectedJets(raw_jets, event, setup);
   jets_ = helper_.GetSelectedJets(corr_jets, 5., 2.4, jetID::none, '-');

   for (auto mu: *input_mu) helper_.addVetos(mu);
   for (auto ele: *input_ele) helper_.addVetos(ele);

   for (auto mu: *input_mu) {
      if (mu.pt() < mu_minpt_) continue;

      pat::Jet matchedJet;
      double dR = 666.;
      for (const auto& j: jets_) {
	double newDR = helper_.DeltaR(&j, &mu);
	if (newDR < dR) {
	  dR = newDR;
	  matchedJet = j;
	}
      }

      //add members
      if (mu.innerTrack().isAvailable())
	{
	  mu.addUserFloat("dxy",fabs(mu.innerTrack()->dxy(vertex_.position())));
	  mu.addUserFloat("dz",fabs(mu.innerTrack()->dz(vertex_.position())));
	  mu.addUserFloat("numValidPixelHits",mu.innerTrack()->hitPattern().numberOfValidPixelHits());
	  mu.addUserFloat("trackerLayersWithMeasurement",mu.innerTrack()->hitPattern().trackerLayersWithMeasurement());
	  mu.addUserFloat("chargeFlip",mu.innerTrack()->ptError()/mu.innerTrack()->pt());
	  mu.addUserFloat("validFraction",mu.innerTrack()->validFraction());
	}
      else
	{
	  mu.addUserFloat("dxy",-666.);
	  mu.addUserFloat("dz",-666.);
	  mu.addUserFloat("numValidPixelHits",-666.);
	  mu.addUserFloat("trackerLayersWithMeasurement",-666.);
	  mu.addUserFloat("chargeFlip",-666.);
	  mu.addUserFloat("validFraction",-666.);
	}

      if (mu.globalTrack().isAvailable())
	{
	  mu.addUserFloat("normalizedChiSq",mu.globalTrack()->normalizedChi2());
	  mu.addUserFloat("numValidMuonHits",mu.globalTrack()->hitPattern().numberOfValidMuonHits());
	}
      else
	{
	  mu.addUserFloat("normalizedChiSq",-666.);
	  mu.addUserFloat("numValidMuonHits",-666.);
	}

      mu.addUserFloat("relIso", helper_.GetMuonRelIso(mu, coneSize::R03, corrType::rhoEA));
      mu.addUserFloat("miniIso", helper_.GetMuonRelIso(mu, coneSize::miniIso, corrType::rhoEA));
      mu.addUserFloat("localChiSq",mu.combinedQuality().chi2LocalPosition);
      mu.addUserFloat("trackKink",mu.combinedQuality().trkKink);
      //lepMVA input vars
      mu.addUserFloat("chargedRelIso",mu.pfIsolationR03().sumChargedHadronPt/mu.pt());
      mu.addUserFloat("neutralRelIso",mu.userFloat("relIso") - mu.userFloat("chargedRelIso"));
      mu.addUserFloat("nearestJetDr",min(dR,0.5));
      mu.addUserFloat("nearestJetPtRatio",std::min(mu.pt()/matchedJet.pt(), 1.5));
      mu.addUserFloat("nearestJetCsv",max(matchedJet.bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"), float(0.0)));
      mu.addUserFloat("sip3D",fabs(mu.dB(pat::Muon::PV3D)/mu.edB(pat::Muon::PV3D)));
      
      mu.addUserFloat("idPreselection", passes(mu, preselection));
      mu.addUserFloat("idLooseCut", passes(mu, looseCut));
      mu.addUserFloat("idTightCut", passes(mu, tightCut));

      if (mu.userFloat("idPreselection") > .5) {
         mu.addUserFloat("leptonMVA", mva(mu));
         mu.addUserFloat("idLooseMVA", passes(mu, looseMVA));
         mu.addUserFloat("idTightMVA", passes(mu, tightMVA));
      } else {
         mu.addUserFloat("leptonMVA", -666.);
         mu.addUserFloat("idLooseMVA", -666.);
         mu.addUserFloat("idTightMVA", -666.);
      }
      mus->push_back(mu);
   }

   for (auto ele: *input_ele) {
      if (ele.pt() < ele_minpt_) continue;
      
      pat::Jet matchedJet;
      double dR = 666.;
      for (const auto& j: jets_) {
	double newDR = helper_.DeltaR(&j, &ele);
	if (newDR < dR) {
	  dR = newDR;
	  matchedJet = j;
	}
      }
      
      ele.addUserFloat("superClusterEta",abs(ele.superCluster()->position().eta()));
      ele.addUserFloat("relIso", helper_.GetElectronRelIso(ele, coneSize::R03, corrType::rhoEA));
      ele.addUserFloat("miniIso", helper_.GetElectronRelIso(ele, coneSize::miniIso, corrType::rhoEA));
      ele.addUserFloat("dxy",fabs(ele.gsfTrack()->dxy(vertex_.position())));
      ele.addUserFloat("dz",fabs(ele.gsfTrack()->dz(vertex_.position())));
      ele.addUserFloat("numMissingHits",ele.gsfTrack()->hitPattern().numberOfHits(reco::HitPattern::MISSING_INNER_HITS));
      //leptonMVA vars
      ele.addUserFloat("chargedRelIso", ele.pfIsolationVariables().sumChargedHadronPt/ele.pt());
      ele.addUserFloat("neutralRelIso",ele.userFloat("relIso") - ele.userFloat("chargedRelIso"));
      ele.addUserFloat("nearestJetDr",min(dR,0.5));
      ele.addUserFloat("nearestJetPtRatio",std::min(ele.pt()/matchedJet.pt(), 1.5));
      ele.addUserFloat("nearestJetCsv",max(matchedJet.bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"), float(0.0)));
      ele.addUserFloat("sip3D",fabs(ele.dB(pat::Electron::PV3D)/ele.edB(pat::Electron::PV3D)));
      bool mvaDebug = false;
      ele.addUserFloat("eleMvaId",mvaID_->mvaValue(ele,mvaDebug));

      ele.addUserFloat("leptonMVA", mva(ele));

      ele.addUserFloat("idPreselection", passes(ele, preselection));
      ele.addUserFloat("idLooseCut", passes(ele, looseCut));
      ele.addUserFloat("idLooseMVA", passes(ele, looseMVA));
      ele.addUserFloat("idTightCut", passes(ele, tightCut));
      ele.addUserFloat("idTightMVA", passes(ele, tightMVA));
      eles->push_back(ele);
   }

   event.put(std::move(eles));
   event.put(std::move(mus));
}

// ------------ method called once each job just before starting event loop  ------------
void 
LeptonIdentifier::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LeptonIdentifier::endJob() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
LeptonIdentifier::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
LeptonIdentifier::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
LeptonIdentifier::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
LeptonIdentifier::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
LeptonIdentifier::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(LeptonIdentifier);
