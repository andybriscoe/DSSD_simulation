/*****************************************************************************
 * Copyright (C) 2009-2016   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: matta@lpccaen.in2p3.fr    *
 *                                                                           *
 * Creation Date  : Octobre 2017                                             *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 * Use to kill the beam track and replace it with the reaction product       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *****************************************************************************/

#include "BeamReaction.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "NPFunction.h"
#include "NPInputParser.h"
#include "NPOptionManager.h"
#include "RootOutput.h"
#include <iostream>
#include <string>
////////////////////////////////////////////////////////////////////////////////
NPS::BeamReaction::BeamReaction(G4String modelName, G4Region* envelope)
    : G4VFastSimulationModel(modelName, envelope) {
  ReadConfiguration();
  m_PreviousEnergy = 0;
  m_PreviousLength = 0;
}

////////////////////////////////////////////////////////////////////////////////
NPS::BeamReaction::BeamReaction(G4String modelName)
    : G4VFastSimulationModel(modelName) {}

////////////////////////////////////////////////////////////////////////////////
NPS::BeamReaction::~BeamReaction() {}

////////////////////////////////////////////////////////////////////////////////
void NPS::BeamReaction::AttachReactionConditions() {
  // Reasssigned the branch address
  if (RootOutput::getInstance()->GetTree()->FindBranch("ReactionConditions"))
    RootOutput::getInstance()->GetTree()->SetBranchAddress(
        "ReactionConditions", &m_ReactionConditions);
}

////////////////////////////////////////////////////////////////////////////////
void NPS::BeamReaction::ReadConfiguration() {
  NPL::InputParser input(NPOptionManager::getInstance()->GetReactionFile());

  //vector<NPL::InputBlock*> blocks;
  //blocks = input.GetAllBlocksWithToken("TwoBodyReaction");
  //if(blocks.size()>0) m_ReactionType ="TwoBodyReaction";
  //
  //blocks = input.GetAllBlocksWithToken("QFSReaction");
  //if(blocks.size()>0) m_ReactionType ="QFSReaction";

  if(input.GetAllBlocksWithToken("TwoBodyReaction").size()>0) m_ReactionType ="TwoBodyReaction";
  if(input.GetAllBlocksWithToken("QFSReaction").size()>0) m_ReactionType ="QFSReaction";


  if (m_ReactionType=="TwoBodyReaction" ) {
      m_Reaction.ReadConfigurationFile(input);
      m_BeamName = NPL::ChangeNameToG4Standard(m_Reaction.GetNucleus1()->GetName());
      if(m_Reaction.GetNucleus3()->GetName() != ""){
          m_active             = true;
          m_ReactionConditions = new TReactionConditions();
          AttachReactionConditions();
          if (!RootOutput::getInstance()->GetTree()->FindBranch("ReactionConditions"))
              RootOutput::getInstance()->GetTree()->Branch(
                      "ReactionConditions", "TReactionConditions", &m_ReactionConditions);
      }
  } else  if (m_ReactionType=="QFSReaction") {
      m_QFS.ReadConfigurationFile(input);
      m_BeamName = NPL::ChangeNameToG4Standard(m_QFS.GetNucleusA()->GetName());
      m_active             = true;
      m_ReactionConditions = new TReactionConditions();
  }
  else {
      m_active = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
G4bool
NPS::BeamReaction::IsApplicable(const G4ParticleDefinition& particleType) {
  if (!m_active)
    return false;

  static std::string particleName;
  particleName = particleType.GetParticleName();
  if (particleName.find(m_BeamName) != std::string::npos) {
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
G4bool NPS::BeamReaction::ModelTrigger(const G4FastTrack& fastTrack) {

  //cout<< "MODEL TRIG"<<endl;
  static bool    shoot        = false;
  static double  rand         = 0;
  const G4Track* PrimaryTrack = fastTrack.GetPrimaryTrack();
  G4ThreeVector  V            = PrimaryTrack->GetMomentum().unit();
  G4ThreeVector  P            = fastTrack.GetPrimaryTrackLocalPosition();
  G4VSolid*      solid        = fastTrack.GetPrimaryTrack()
                        ->GetVolume()
                        ->GetLogicalVolume()
                        ->GetSolid();
  double in    = solid->DistanceToOut(P, V);
  double out   = solid->DistanceToOut(P, -V);
  double ratio = in / (out + in);

  //cout<< "in:"<<in<<std::scientific<<endl;
  //cout<< "ou:"<<out<<std::scientific<<endl;
  //cout<< "ratio:"<<ratio<<std::scientific<<endl;
  // m_StepSize = m_StepSize/100000.;

  if (out == 0) { // first step of current event
    //cout<< "FIRST"<<endl;
    rand             = G4RandFlat::shoot();
    m_PreviousLength = m_StepSize;
    m_PreviousEnergy = PrimaryTrack->GetKineticEnergy();
    // Clear Previous Event
    m_ReactionConditions->Clear();
    shoot = true;
  }
  else if ((in-m_StepSize) <= 1E-9) { // last step
    //cout<< "LAST"<<endl;
    return true;
  }

  //cout.precision(17); 
  //cout<< "rand:"<<rand<<std::scientific<<endl;

  // If the condition is met, the event is generated
  if (ratio < rand) {

    // Reset the static for next event
    //  shoot = false;
    if(m_ReactionType=="QFSReaction"){
        if ( shoot && m_QFS.IsAllowed() ) {
            shoot = false;
            return true;
        } else {
            return false;
        }
    }else if(m_ReactionType=="TwoBodyReaction"){
        if ( shoot && m_Reaction.IsAllowed(PrimaryTrack->GetKineticEnergy()) ) {
            shoot = false;
            return true;
        } else {
            return false;
        }
    }else{
        return false;
    }
  }

  // Record the situation of the current step
  // so it can be used in the next one
  if (!PrimaryTrack->GetStep()->IsLastStepInVolume()) {
    m_PreviousLength = PrimaryTrack->GetStep()->GetStepLength();
    //cout<< "PreviousLength="<<m_PreviousLength<<endl;
    m_PreviousEnergy = PrimaryTrack->GetKineticEnergy();
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void NPS::BeamReaction::DoIt(const G4FastTrack& fastTrack,
        G4FastStep&        fastStep) {

    // Get the track info
    const G4Track* PrimaryTrack = fastTrack.GetPrimaryTrack();
    G4ThreeVector  pdirection   = PrimaryTrack->GetMomentum().unit();
    G4ThreeVector  localdir     = fastTrack.GetPrimaryTrackLocalDirection();

    G4ThreeVector worldPosition = PrimaryTrack->GetPosition();
    G4ThreeVector localPosition = fastTrack.GetPrimaryTrackLocalPosition();

    double energy = PrimaryTrack->GetKineticEnergy();
    double time   = PrimaryTrack->GetGlobalTime();

    // Randomize within the step
    // Assume energy loss is linear within the step
    // Assume no scattering
    double rand   = G4RandFlat::shoot();
    double length = rand * (m_PreviousLength);
    energy -= (1 - rand) * (m_PreviousEnergy - energy);
    G4ThreeVector ldir = pdirection;
    ldir *= length;
    localPosition = localPosition - ldir;
    // Set the end of the step conditions
    fastStep.SetPrimaryTrackFinalKineticEnergyAndDirection(0, pdirection);
    fastStep.SetPrimaryTrackFinalPosition(worldPosition);
    fastStep.SetTotalEnergyDeposited(0);
    fastStep.SetPrimaryTrackFinalTime(time);
    fastStep.KillPrimaryTrack();
    fastStep.SetPrimaryTrackPathLength(0.0);


    if (m_ReactionType=="TwoBodyReaction" ) {

        ///////////////////////////////
        // Two-Body Reaction Case /////
        ///////////////////////////////

        //////Define the kind of particle to shoot////////
        // Nucleus 3
        int LightZ = m_Reaction.GetNucleus3()->GetZ();
        int LightA = m_Reaction.GetNucleus3()->GetA();
        static G4IonTable* IonTable
            = G4ParticleTable::GetParticleTable()->GetIonTable();

        G4ParticleDefinition* LightName;

        if (LightZ == 0 && LightA == 1) // neutron is special case
        {
            LightName = G4Neutron::Definition();
        } else {
            if (m_Reaction.GetUseExInGeant4())
                LightName
                    = IonTable->GetIon(LightZ, LightA, m_Reaction.GetExcitation3() * MeV);
            else
                LightName = IonTable->GetIon(LightZ, LightA);
        }

        // Nucleus 4
        G4int HeavyZ = m_Reaction.GetNucleus4()->GetZ();
        G4int HeavyA = m_Reaction.GetNucleus4()->GetA();

        // Generate the excitation energy if a distribution is given
        m_Reaction.ShootRandomExcitationEnergy();

        // Use to clean up the IonTable in case of the Ex changing at every event
        G4ParticleDefinition* HeavyName;

        if (m_Reaction.GetUseExInGeant4())
            HeavyName
                = IonTable->GetIon(HeavyZ, HeavyA, m_Reaction.GetExcitation4() * MeV);
        else
            HeavyName = IonTable->GetIon(HeavyZ, HeavyA);

        // Set the Energy of the reaction
        m_Reaction.SetBeamEnergy(energy);

        double Beam_theta = pdirection.theta();
        double Beam_phi   = pdirection.phi();

        ///////////////////////////
        ///// Beam Parameters /////
        ///////////////////////////
        m_ReactionConditions->SetBeamParticleName(
                PrimaryTrack->GetParticleDefinition()->GetParticleName());

        m_ReactionConditions->SetBeamReactionEnergy(energy);
        m_ReactionConditions->SetVertexPositionX(localPosition.x());
        m_ReactionConditions->SetVertexPositionY(localPosition.y());
        m_ReactionConditions->SetVertexPositionZ(localPosition.z());

        G4ThreeVector U(1, 0, 0);
        G4ThreeVector V(0, 1, 0);
        G4ThreeVector ZZ(0, 0, 1);
        m_ReactionConditions->SetBeamEmittanceTheta(
                PrimaryTrack->GetMomentumDirection().theta() / deg);
        m_ReactionConditions->SetBeamEmittancePhi(
                PrimaryTrack->GetMomentumDirection().phi() / deg);
        m_ReactionConditions->SetBeamEmittanceThetaX(
                PrimaryTrack->GetMomentumDirection().angle(U) / deg);
        m_ReactionConditions->SetBeamEmittancePhiY(
                PrimaryTrack->GetMomentumDirection().angle(V) / deg);

        //////////////////////////////////////////////////////////
        ///// Build rotation matrix to go from the incident //////
        ///// beam frame to the "world" frame               //////
        //////////////////////////////////////////////////////////

        
        //   G4ThreeVector col1(cos(Beam_theta) * cos(Beam_phi),
        //   cos(Beam_theta) * sin(Beam_phi),
        //   -sin(Beam_theta));
        //   G4ThreeVector col2(-sin(Beam_phi),
        //   cos(Beam_phi),
        //   0);
        //   G4ThreeVector col3(sin(Beam_theta) * cos(Beam_phi),
        //   sin(Beam_theta) * sin(Beam_phi),
        //   cos(Beam_theta));
        //   G4RotationMatrix BeamToWorld(col1, col2, col3);

        /////////////////////////////////////////////////////////////////
        ///// Angles for emitted particles following Cross Section //////
        ///// Angles are in the beam frame                         //////
        /////////////////////////////////////////////////////////////////

        // Angles
        // Shoot and Set a Random ThetaCM
        m_Reaction.ShootRandomThetaCM();
        double phi = RandFlat::shoot() * 2. * pi;

        //////////////////////////////////////////////////
        /////  Momentum and angles from  kinematics  /////
        /////  Angles are in the beam frame          /////
        //////////////////////////////////////////////////
        // Variable where to store results
        double Theta3, Energy3, Theta4, Energy4;

        // Compute Kinematic using previously defined ThetaCM
        m_Reaction.KineRelativistic(Theta3, Energy3, Theta4, Energy4);
        // Momentum in beam frame for light particle
        G4ThreeVector momentum_kine3_beam(sin(Theta3) * cos(phi),
                sin(Theta3) * sin(phi), cos(Theta3));
        // Momentum in World frame //to go from the incident beam frame to the "world"
        // frame
        G4ThreeVector momentum_kine3_world = momentum_kine3_beam;
        momentum_kine3_world.rotate(Beam_theta,
                V); // rotation of Beam_theta on Y axis
        momentum_kine3_world.rotate(Beam_phi, ZZ); // rotation of Beam_phi on Z axis

        // Momentum in beam frame for heavy particle
        G4ThreeVector momentum_kine4_beam(sin(Theta4) * cos(phi + pi),
                sin(Theta4) * sin(phi + pi), cos(Theta4));
        // Momentum in World frame
        G4ThreeVector momentum_kine4_world = momentum_kine4_beam;
        momentum_kine4_world.rotate(Beam_theta,
                V); // rotation of Beam_theta on Y axis
        momentum_kine4_world.rotate(Beam_phi, ZZ); // rotation of Beam_phi on Z axis

        // Emitt secondary
        if (m_Reaction.GetShoot3()) {
            G4DynamicParticle particle3(LightName, momentum_kine3_world, Energy3);
            fastStep.CreateSecondaryTrack(particle3, localPosition, time);
        }

        if (m_Reaction.GetShoot4()) {
            G4DynamicParticle particle4(HeavyName, momentum_kine4_world, Energy4);
            fastStep.CreateSecondaryTrack(particle4, localPosition, time);
        }

        // Reinit for next event
        m_PreviousEnergy = 0;
        m_PreviousLength = 0;

        ///////////////////////////////////////
        ///// Emitted particle Parameters /////
        ///////////////////////////////////////
        // Names 3 and 4//
        m_ReactionConditions->SetParticleName(LightName->GetParticleName());
        m_ReactionConditions->SetParticleName(HeavyName->GetParticleName());
        // Angle 3 and 4 //
        m_ReactionConditions->SetTheta(Theta3 / deg);
        m_ReactionConditions->SetTheta(Theta4 / deg);

        m_ReactionConditions->SetPhi(phi / deg);
        if ((phi + pi) / deg > 360)
            m_ReactionConditions->SetPhi((phi - pi) / deg);
        else
            m_ReactionConditions->SetPhi((phi + pi) / deg);

        // Energy 3 and 4 //
        m_ReactionConditions->SetKineticEnergy(Energy3);
        m_ReactionConditions->SetKineticEnergy(Energy4);
        // ThetaCM and Ex//
        m_ReactionConditions->SetThetaCM(m_Reaction.GetThetaCM() / deg);
        m_ReactionConditions->SetExcitationEnergy3(m_Reaction.GetExcitation3());
        m_ReactionConditions->SetExcitationEnergy4(m_Reaction.GetExcitation4());
        // Momuntum X 3 and 4 //
        m_ReactionConditions->SetMomentumDirectionX(momentum_kine3_world.x());
        m_ReactionConditions->SetMomentumDirectionX(momentum_kine4_world.x());
        // Momuntum Y 3 and 4 //
        m_ReactionConditions->SetMomentumDirectionY(momentum_kine3_world.y());
        m_ReactionConditions->SetMomentumDirectionY(momentum_kine4_world.y());
        // Momuntum Z 3 and 4 //
        m_ReactionConditions->SetMomentumDirectionZ(momentum_kine3_world.z());
        m_ReactionConditions->SetMomentumDirectionZ(momentum_kine4_world.z());

    }// end if TwoBodyReaction

    else if (m_ReactionType=="QFSReaction" ) {

        //////Define the kind of particle to shoot////////
        //    A --> T  ==> B + (c -> T) =>  B + 1 + 2      
           
        int Light1_Z = m_QFS.GetNucleus1()->GetZ();
        int Light1_A = m_QFS.GetNucleus1()->GetA();
        int Light2_Z = m_QFS.GetNucleus2()->GetZ();
        int Light2_A = m_QFS.GetNucleus2()->GetA();

        static G4IonTable* IonTable
            = G4ParticleTable::GetParticleTable()->GetIonTable();

        G4ParticleDefinition* Light1Name;
        G4ParticleDefinition* Light2Name;

        if (Light1_Z == 0 && Light1_A == 1) // neutron is special case
        {
            Light1Name = G4Neutron::Definition();
        } else {
            Light1Name = IonTable->GetIon(Light1_Z, Light1_A);
        }

        if (Light2_Z == 0 && Light2_A == 1) // neutron is special case
        {
            Light2Name = G4Neutron::Definition();
        } else {
            Light2Name = IonTable->GetIon(Light2_Z, Light2_A);
        }

        // Nucleus B
        G4int Heavy_Z = m_QFS.GetNucleusB()->GetZ();
        G4int Heavy_A = m_QFS.GetNucleusB()->GetA();

        G4ParticleDefinition* HeavyName;
        HeavyName = IonTable->GetIon(Heavy_Z, Heavy_A);

        // Set the Energy of the reaction
        m_QFS.SetBeamEnergy(energy);

        double Beam_theta = pdirection.theta();
        double Beam_phi   = pdirection.phi();


        /////////////////////////////////////////////////////////////////
        ///// Angles for emitted particles following Cross Section //////
        ///// Angles are in the beam frame                         //////
        /////////////////////////////////////////////////////////////////

        // Angles
        // Shoot and Set a Random ThetaCM
        //m_QFS.ShootRandomThetaCM();
        //m_QFS.ShootRandomPhiCM();
        double theta = RandFlat::shoot() *  pi;
        double phi = RandFlat::shoot() * 2. * pi;

        m_QFS.SetThetaCM(theta);
        m_QFS.SetPhiCM(phi);

        //////////////////////////////////////////////////
        /////  Momentum and angles from  kinematics  /////
        //////////////////////////////////////////////////
        // Variable where to store results
        double Theta1, Phi1, Energy1, Theta2, Phi2, Energy2;

        // Compute Kinematic using previously defined ThetaCM
        m_QFS.KineRelativistic(Theta1, Phi1, Energy1, Theta2, Phi2, Energy2);

        G4ThreeVector U(1, 0, 0);
        G4ThreeVector V(0, 1, 0);
        G4ThreeVector ZZ(0, 0, 1);

        // Momentum in beam and world frame for light particle 1
        G4ThreeVector momentum_kine1_beam(sin(Theta1) * cos(Phi1),
                sin(Theta1) * sin(Phi1), cos(Theta1));
        G4ThreeVector momentum_kine1_world = momentum_kine1_beam;
        momentum_kine1_world.rotate(Beam_theta, V); // rotation of Beam_theta on Y axis
        momentum_kine1_world.rotate(Beam_phi, ZZ); // rotation of Beam_phi on Z axis

       // Momentum in beam and world frame for light particle 2
        G4ThreeVector momentum_kine2_beam(sin(Theta2) * cos(Phi2),
                sin(Theta2) * sin(Phi2), cos(Theta2));
        G4ThreeVector momentum_kine2_world = momentum_kine2_beam;
        momentum_kine2_world.rotate(Beam_theta, V); // rotation of Beam_theta on Y axis
        momentum_kine2_world.rotate(Beam_phi, ZZ); // rotation of Beam_phi on Z axis

        // Momentum in beam and world frame for heavy residual
        //
        //G4ThreeVector momentum_kineB_beam(sin(ThetaB) * cos(PhiB + pi),
        //        sin(ThetaB) * sin(PhiB + pi), cos(ThetaB));
        //G4ThreeVector momentum_kineB_world =  momentum_kineB_beam;
        //momentum_kineB_world.rotate(Beam_theta, V); // rotation of Beam_theta on Y axis
        //momentum_kineB_world.rotate(Beam_phi, ZZ); // rotation of Beam_phi on Z axis
        
        TLorentzVector* P_B = m_QFS.GetEnergyImpulsionLab_B();
        
        G4ThreeVector momentum_kineB_beam( P_B->Px(), P_B->Py(), P_B->Pz() );
        momentum_kineB_beam = momentum_kineB_beam.unit();
        double EnergyB = m_QFS.GetEnergyImpulsionLab_B()->Energy();
        G4ThreeVector momentum_kineB_world =  momentum_kineB_beam;
        momentum_kineB_world.rotate(Beam_theta, V); // rotation of Beam_theta on Y axis
        momentum_kineB_world.rotate(Beam_phi, ZZ); // rotation of Beam_phi on Z axis
  
        // Emitt secondary
        if (m_QFS.GetShoot1()) {
            G4DynamicParticle particle1(Light1Name, momentum_kine1_world, Energy1);
            fastStep.CreateSecondaryTrack(particle1, localPosition, time);
        }

        if (m_QFS.GetShoot2()) {
            G4DynamicParticle particle2(Light2Name, momentum_kine2_world, Energy2);
            fastStep.CreateSecondaryTrack(particle2, localPosition, time);
        }
        if (m_QFS.GetShootB()) {
            G4DynamicParticle particleB(HeavyName, momentum_kineB_world, EnergyB);
            fastStep.CreateSecondaryTrack(particleB, localPosition, time);
        }


        // Reinit for next event
        m_PreviousEnergy = 0;
        m_PreviousLength = 0;



    }
}
