#ifndef NPQFS_h
#define NPQFS_h
/*****************************************************************************
 * Copyright (C) 2009-2019   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 * Original Author :  F. Flavigny contact address: flavigny@lpccaen.in2p3.fr *
 *                                                                           *
 * Creation Date   : April 2019                                              *
 * Last update     : Nov 2019                                                *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class deal with Quasi Free Scattering Reaction in which a cluster   *
 *  or a nucleon is removed from a projectile  by interaction with a target  *
 *  nucleon (proton target in general)                                       *
 *                                                                           *
 *  First step (dissociation):  A -> B + c                                   *
 *  Second step (scattering) :  c + T -> 1 + 2                               *
 *  Labeling is:                                                             *
 *                                                                           *
 *              A --> T  ==> B + (c -> T) =>  B + 1 + 2                      *
 *                                                                           *
 *  where:                                                                   *
 *    +  A is the beam nucleus                                               *
 *    +  T is the target nucleon (proton)                                    *
 *                                                                           *
 *    +  B is the residual fragment (beam-like)                              *
 *    +  1 is the scattered target nucleon  (former T)                       *
 *    +  2 is the knocked-out cluster/nucleon (noted c) in the intermediate  *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *    +  Adapted from original event generator from V. Panin (R3B collab)    *
 *                                                                           *
 *****************************************************************************/

// C++ header
#include <string>

// NPL
#include "NPNucleus.h"
#include "NPBeam.h"
#include "NPInputParser.h"
using namespace NPL;

// ROOT header
#include "TLorentzVector.h"
#include "TLorentzRotation.h"
#include "TVector3.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TRandom3.h"

using namespace std;

namespace NPL{

  class QFS{

    public:  // Constructors and Destructors
      QFS();
      ~QFS();

    private:
    int fVerboseLevel;
    Beam     fNucleiA;                 // Beam (A)
    Nucleus  fNucleiT;                 // Target (T)
    Nucleus  fNucleiB;                 // Beam-like ejectile (B)
    Nucleus  fNuclei1;                 // Target-like ejectile (1)
    Nucleus  fNuclei2;                 // Knocked-out nucleon/cluster (2)
    double   fQValue;                  // Q-value in MeV
    double   fEcm;                     // Ecm in MeV
    double   fThetaCM;                 // Center-of-mass theta angle in radian
    double   fPhiCM;                   // Center-of-mass Phi angle in radian
    double   fBeamEnergy;              // Beam energy in MeV
    double   fExcitationA;             // Excitation energy in MeV of the beam, useful for isomers 
    double   fExcitationB;             // Excitation energy in MeV of beam-like ejectile
    double   fMomentumSigma;          // Width of the momentum distribution of the removed cluster (sigma)             
    double   fisotropic;               

    TGraph* fTheta2VsTheta1;
    TGraph* fPhi2VsPhi1;

    // used for MC simulations
    bool fshootB; // shoot beam-like ejectile
    bool fshoot1; // shoot light ejectile &
    bool fshoot2; // shoot light ejectile 2
    
    public:
    Nucleus GetNucleus(string name, NPL::InputParser parser);
    void ReadConfigurationFile(string Path);
    void ReadConfigurationFile(NPL::InputParser);
    void CalculateVariables();
    void KineRelativistic(double &ThetaLab1, double &PhiLab1, double &KineticEnergyLab1, double &ThetaLab2, double &PhiLab2, double &KineticEnergyLab2);
    double ShootRandomThetaCM();
    bool IsAllowed();

    private: // intern precompute variable
    double mA;
    double mB;
    double ma_off;
    double ma;
    double mT;
    double m1;
    double m2;

    TVector3 Pa, PB;
    double Ea_lab,EB_lab;

    // Lorentz Vector
    TLorentzVector fEnergyImpulsionLab_A;
    TLorentzVector fEnergyImpulsionLab_B;
    TLorentzVector fEnergyImpulsionLab_a;
    TLorentzVector fEnergyImpulsionLab_T;
    TLorentzVector fEnergyImpulsionLab_1;
    TLorentzVector fEnergyImpulsionLab_2;
    TLorentzVector fTotalEnergyImpulsionLab;

    TLorentzVector fEnergyImpulsionCM_a;
    TLorentzVector fEnergyImpulsionCM_T;
    TLorentzVector fEnergyImpulsionCM_1;
    TLorentzVector fEnergyImpulsionCM_2;
    TLorentzVector fTotalEnergyImpulsionCM;

    // Impulsion Vector3
    TVector3 fImpulsionLab_a;
    TVector3 fImpulsionLab_T;
    TVector3 fImpulsionLab_1;
    TVector3 fImpulsionLab_2;

    TVector3 fImpulsionCM_a;
    TVector3 fImpulsionCM_T;
    TVector3 fImpulsionCM_1;
    TVector3 fImpulsionCM_2;

    // CM Energy composante & CM impulsion norme
    Double_t ECM_a;
    Double_t ECM_T;
    Double_t ECM_1;
    Double_t ECM_2;
    Double_t pCM_a;
    Double_t pCM_T;
    Double_t pCM_1;
    Double_t pCM_2;

    // Mandelstam variable
    Double_t s;

    // Center of Mass Kinematic
    Double_t BetaCM;

    public:
    //SETTERS
    void SetBeamEnergy(const double& eBeam) {fBeamEnergy = eBeam;}
    void SetThetaCM(const double& angle) {fThetaCM = angle;}
    void SetPhiCM(const double& angle) {fPhiCM = angle;}

    //GETTERS
    Nucleus*  GetNucleusA()               {return &fNucleiA;}
    Nucleus*  GetNucleusT()               {return &fNucleiT;}
    Nucleus*  GetNucleusB()               {return &fNucleiB;}
    Nucleus*  GetNucleus1()               {return &fNuclei1;}
    Nucleus*  GetNucleus2()               {return &fNuclei2;}
    bool     GetShoot1()         const        {return fshoot1;}
    bool     GetShoot2()         const        {return fshoot2;}
    bool     GetShootB()         const        {return fshootB;}
 
    TLorentzVector*  GetEnergyImpulsionLab_B() {return &fEnergyImpulsionLab_B;}
    TGraph* GetTheta2VsTheta1(double AngleStep_CM=1);
    TGraph* GetPhi2VsPhi1(double AngleStep_CM=1);



    /*
    //TO REMOVE AT SOME POINT WHEN CLASS IS ROBUSTLY TESTED
    private:
    // R3B Methods and Variables used as a starting point for this class (useful for checks)
    void CalculateVariablesOld();
    pair<double,double> Lorentz(double, double, double, double);
    double function(double, double, double);
    void KineR3B(double, double, double, double, double);
    TVector3 Rotations(TVector3,TVector3);
    double e_clust;
    double p_clust;
    double theta_clust;
    double e_scat;
    double p_scat;
    double theta_scat;
    bool good;
    double T;
    */

    ClassDef(QFS,0)
  };
}
#endif
