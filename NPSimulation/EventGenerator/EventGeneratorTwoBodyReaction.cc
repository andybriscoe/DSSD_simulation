/*****************************************************************************
 * Copyright (C) 2009-2016   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: matta@lpccaen.in2p3.fr    *
 *                                                                           *
 * Creation Date  : January 2009                                             *
 * Last update    : January 2013                                             *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This event Generator is used to simulated two body two body reaction.    *
 *  A Relativistic computation is performed to determine angle and energy of *
 *   the different particle, knowing the ThetaCM angle given by a cross      *
 *   section shoot. Eleastic scattering can also be simulated.               *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *    + 20/01/2011: Add support for excitation energy for light ejectile     *
 *                  (N. de Sereville)                                        *
 *    + 23/01/2013: Class change name (ild name EventGeneratorTransfert)     *
 *                  (A. MATTA)                                               *
 *                                                                           *
 *****************************************************************************/
// C++ headers
#include <iostream>
#include <fstream>
#include <limits>
#include <random>

// G4 header defining G4 types
#include "globals.hh"

// G4 headers
#include "G4UImanager.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4RotationMatrix.hh"

// G4 headers including CLHEP headers
// for generating random numbers
#include "Randomize.hh"

// NPTool headers
#include "EventGeneratorTwoBodyReaction.hh"
#include "RootOutput.h"

using namespace CLHEP;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
EventGeneratorTwoBodyReaction::EventGeneratorTwoBodyReaction()
:  m_ShootLight(0),
m_ShootHeavy(0),
m_Target(0)
{
  //------------- Default Constructor -------------
  m_ParticleStack= ParticleStack::getInstance();
  m_Reaction = new NPL::Reaction();
  //radioactiveDecay->AddUserDecayDataFile(82, 188, "simplez82.a188");

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventGeneratorTwoBodyReaction::SetTarget(Target* Target) {
  if (Target != 0) {
    m_Target = Target;
  }
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
EventGeneratorTwoBodyReaction::~EventGeneratorTwoBodyReaction(){
  //------------- Default Destructor ------------
    delete m_Reaction;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventGeneratorTwoBodyReaction::InitializeRootOutput(){
  
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventGeneratorTwoBodyReaction::Print() const{
  
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//    Inherit from VEventGenerator

void EventGeneratorTwoBodyReaction::ReadConfiguration(NPL::InputParser parser){

  vector<NPL::InputBlock*> blocks = parser.GetAllBlocksWithToken("TwoBodyReaction");
    for(unsigned int i = 0 ; i < blocks.size() ; i++)
    {
        m_Z                 =blocks[i]->GetInt("Z");
        m_A                 =blocks[i]->GetInt("A");
        if(blocks[i]->HasToken("PhotonEvaporation"))
        {
            G4String command = "/grdm/setPhotoEvaporationFile ";
            G4String fileName = blocks[i]->GetString("PhotonEvaporation");
            m_PhotonEvaporation=command+
                to_string(m_Z)+" "+
                to_string(m_A)+" "+
                fileName;
            cout << "Reading file " << fileName << endl;
            G4UImanager* UI = G4UImanager::GetUIpointer();
            if (m_PhotonEvaporation) UI->ApplyCommand(m_PhotonEvaporation);
        }
    }

  m_Reaction->ReadConfigurationFile(parser);
  m_ShootLight = m_Reaction->GetShoot3();
  m_ShootHeavy = m_Reaction->GetShoot4();;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventGeneratorTwoBodyReaction::GenerateEvent(G4Event*){

  //radioactiveDecay->AddUserDecayDataFile(HeavyZ, HeavyA, "simplez82.a188");
  //////////////////////////////////////////////////
  //////Define the kind of particle to shoot////////
  //////////////////////////////////////////////////
  // Nucleus 3
  G4int LightZ = m_Reaction->GetNucleus3()->GetZ() ;
  G4int LightA = m_Reaction->GetNucleus3()->GetA() ;
  //cout << "LightZ " << LightZ << " LightA " << LightA << endl;

  
  // If a single excitation energy was given use it
  if (m_Reaction->GetInitialLevel3()->size() > 0)
  {
     m_InitalLevelLight = m_Reaction->GetInitialLevel3();
     m_InitalPopulationLight = m_Reaction->GetInitialPopulation3();
        
     G4double excitEnergy = 0;
     G4double populationSum = 0;

     float startLevel = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
     for (unsigned int i = 0; i < m_InitalLevelLight->size(); i++)
     {
         populationSum += m_InitalPopulationLight->at(i);
         //cout << populationSum << " " << startLevel << "\n";
         if (populationSum > startLevel)
         {
             excitEnergy = m_InitalLevelLight->at(i) * keV;
             break;
         }
     } 
     //LightName = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(LightZ, LightA, excitEnergy);
     m_particle_light = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(LightZ, LightA, excitEnergy);
  }
  else 
  {
      m_particle_light = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(LightZ, LightA, m_Reaction->GetExcitation3()*MeV);
      //LightName = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(LightZ, LightA, m_Reaction->GetExcitation3()*MeV);
  }

  // Nucleus 4
  G4int HeavyZ = m_Reaction->GetNucleus4()->GetZ() ;
  G4int HeavyA = m_Reaction->GetNucleus4()->GetA() ;
  
  // Generate the excitation energy if a distribution is given
  m_Reaction->ShootRandomExcitationEnergy();

  // If a single excitation energy was given use it
  if (m_Reaction->GetInitialLevel4()->size() > 0)
  {
     m_InitalLevelHeavy = m_Reaction->GetInitialLevel4();
     m_InitalPopulationHeavy = m_Reaction->GetInitialPopulation4();
        
     G4double excitEnergy = 0;
     G4double populationSum = 0;

     float startLevel = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
     for (unsigned int i = 0; i < m_InitalLevelHeavy->size(); i++)
     {
         populationSum += m_InitalPopulationHeavy->at(i);
         //cout << populationSum << " " << startLevel << "\n";
         if (populationSum > startLevel)
         {
             excitEnergy = m_InitalLevelHeavy->at(i) * keV;
             break;
         }
     } 
     //HeavyName = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(HeavyZ, HeavyA, excitEnergy);
     m_particle_heavy = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(HeavyZ, HeavyA, excitEnergy);
  }
  else
  {
      //HeavyName = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(HeavyZ, HeavyA, m_Reaction->GetExcitation4()*MeV);
      m_particle_heavy = G4ParticleTable::GetParticleTable()->GetIonTable()->GetIon(HeavyZ, HeavyA, m_Reaction->GetExcitation4()*MeV);
  }



  // Get the beam particle form the Particle Stack
  Particle BeamParticle = m_ParticleStack->SearchAndRemoveParticle(m_BeamName);
  //Particle BeamParticle = m_ParticleStack->SearchAndRemoveParticle("S32");
  //cout << "particle stack beam energy " << BeamParticle.GetParticleKineticEnergy() << endl;
  //cout << "beam particle name to come off stack " << m_BeamName << endl;
  m_Reaction->SetBeamEnergy(BeamParticle.GetParticleKineticEnergy());
  
  G4double Beam_theta = BeamParticle.GetParticleMomentumDirection().theta();
  G4double Beam_phi = BeamParticle.GetParticleMomentumDirection().phi();
  
  //////////////////////////////////////////////////////////
  ///// Build rotation matrix to go from the incident //////
  ///// beam frame to the "world" frame               //////
  //////////////////////////////////////////////////////////
  G4ThreeVector col1(cos(Beam_theta) * cos(Beam_phi),
                     cos(Beam_theta) * sin(Beam_phi),
                     -sin(Beam_theta));
  G4ThreeVector col2(-sin(Beam_phi),
                     cos(Beam_phi),
                     0);
  G4ThreeVector col3(sin(Beam_theta) * cos(Beam_phi),
                     sin(Beam_theta) * sin(Beam_phi),
                     cos(Beam_theta));
  G4RotationMatrix BeamToWorld(col1, col2, col3);
  
  /////////////////////////////////////////////////////////////////
  ///// Angles for emitted particles following Cross Section //////
  ///// Angles are in the beam frame                         //////
  /////////////////////////////////////////////////////////////////
  
  // Angles
  // Shoot and Set a Random ThetaCM
  G4double ThetaCM = m_Reaction->ShootRandomThetaCM();
  G4double phi     = RandFlat::shoot() * 2. * pi;
  //G4double phi     = RandFlat::shoot() * .1 * pi;// TODO this is a bad, bad thing
 
  //////////////////////////////////////////////////
  /////  Momentum and angles from  kinematics  /////
  /////  Angles are in the beam frame          /////
  //////////////////////////////////////////////////
  // Variable where to store results
  G4double ThetaLight, EnergyLight, ThetaHeavy, EnergyHeavy;
  
  ThetaLight = 0;
  EnergyLight = 0;
  ThetaHeavy = 0;
  EnergyHeavy = 0;
  // Compute Kinematic using previously defined ThetaCM
  //cout << "theta light " << ThetaLight << " energy light " << EnergyLight << " theta heavy " << ThetaHeavy << " energy heavy " << EnergyHeavy << endl;
  m_Reaction->KineRelativistic(ThetaLight, EnergyLight, ThetaHeavy, EnergyHeavy);
  //cout << "theta light " << ThetaLight << " energy light " << EnergyLight << " theta heavy " << ThetaHeavy << " energy heavy " << EnergyHeavy << endl;
  
  // Momentum in beam frame for light particle
  G4ThreeVector momentum_kineLight_beam(sin(ThetaLight) * cos(phi),
                                        sin(ThetaLight) * sin(phi),
                                        cos(ThetaLight));
  
  // Momentum in beam frame for heavy particle
  G4ThreeVector momentum_kineHeavy_beam(sin(ThetaHeavy) * cos(phi+pi),
                                        sin(ThetaHeavy) * sin(phi+pi),
                                        cos(ThetaHeavy));
  


  //////////////////////////////////////////////////
  ///////// Set up everything for shooting /////////
  //////////////////////////////////////////////////
  // Case of light particle
  // Particle direction
  // Kinematical angles in the beam frame are transformed
  // to the "world" frame
  G4ThreeVector momentum_kine_world = BeamToWorld * momentum_kineLight_beam;
  //Add the particle to the particle stack
  //Particle LightParticle(LightName, ThetaCM , EnergyLight,momentum_kine_world, BeamParticle.GetParticlePosition(), m_ShootLight);
  Particle LightParticle(m_particle_light, ThetaCM , EnergyLight,momentum_kine_world, BeamParticle.GetParticlePosition(), m_ShootLight);
  m_ParticleStack->AddParticleToStack(LightParticle);
  
  // ORIGINAL 
  // Case of heavy particle
  // Particle direction
  // Kinematical angles in the beam frame are transformed
  // to the "world" frame
  momentum_kine_world = BeamToWorld * momentum_kineHeavy_beam;
  //Particle HeavyParticle(HeavyName, ThetaCM + M_PI, EnergyHeavy,momentum_kine_world, BeamParticle.GetParticlePosition(), m_ShootHeavy);
  Particle HeavyParticle(m_particle_heavy, ThetaCM + M_PI, EnergyHeavy,momentum_kine_world, BeamParticle.GetParticlePosition(), m_ShootHeavy);

  m_ParticleStack->AddParticleToStack(HeavyParticle);

}
