#include "Trapped_Electrons.hh"
#include <cmath>
#include <iomanip>
using namespace CLHEP;

Trapped_Electrons::Trapped_Electrons(const G4String& processName) : G4VDiscreteProcess(processName)
{;}
Trapped_Electrons::~Trapped_Electrons()
{;}
G4bool Trapped_Electrons::IsApplicable(const G4ParticleDefinition& particle)
{ 
// returns "true" if this model can be applied to the particle-type
  //G4cout << "IsApplicable loaded!" << G4endl;
return true;
  //if (particle == *(       G4Electron::Electron()       )) 
  //  {
  //    //G4cout << "Found the Neutron ! " << G4endl;
  //    return true;
  //  }
  //else
  //  {   enum G4ExceptionSeverity severity;
  //      severity = JustWarning;
  //      G4Exception("Model \"Trapped_Electrons\" can only be used for Stopped elecrtons that are stuck in magnetic fields for too long!","",severity,"");}
  //return false;
}


G4double Trapped_Electrons::GetMeanFreePath(const G4Track& aTrack, G4double, // previousStepSize, 
                                      G4ForceCondition* condition) 
{
  //G4cout << "Trapped_Electrons GetMeanFreePath called!" << G4endl;
  
  //if (aTrack.GetCurrentStepNumber() > 100) G4cout << "Total steps: " << aTrack.GetCurrentStepNumber() << G4endl;
  //G4cout << "Total steps: " << aTrack.GetCurrentStepNumber() << G4endl;
  //if (aTrack.GetCurrentStepNumber() > 50000) printf("Total Steps: %d\n", aTrack.GetCurrentStepNumber());
  //if (aTrack.GetCurrentStepNumber() > 50000) aTrack.SetTrackStatus(fStopAndKill);
  const G4Step* theStep = aTrack.GetStep();
  G4StepPoint* thePoint = theStep->GetPreStepPoint();
  G4String theVolume =  thePoint->GetPhysicalVolume()->GetName();
 
  const G4DynamicParticle* projectile = aTrack.GetDynamicParticle();
  const G4ParticleDefinition* particle = projectile->GetDefinition();
  std::string type = particle->GetParticleType();
  
  if(type == "nucleus")  
    {*condition = Forced;}

  ///if(theVolume == "target")  
  ///  {*condition = Forced;}
  ///else
  ///  {*condition = NotForced;}
    
  if (aTrack.GetCurrentStepNumber() > 29999) *condition = Forced;

  //*condition = NotForced;
  
//  G4double previousStepSize = 0.1*mm;

  //const G4DynamicParticle* projectile = aTrack.GetDynamicParticle();
  //G4double theKinEng = projectile->GetKineticEnergy();

  return DBL_MAX;
}

G4VParticleChange* Trapped_Electrons::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep)
{
  // Now we tell GEANT what to do if MeanFreePath condition is satisfied!
  // Overrides PostStepDoIt function in G4VDiscreteProcess

  const G4DynamicParticle* projectile = aTrack.GetDynamicParticle();
  const G4ParticleDefinition* particle = projectile->GetDefinition();

  //std::cout << particle->GetParticleType() << std::endl;
  //G4cout << "Total steps: " << aTrack.GetCurrentStepNumber() << G4endl;
  //const G4Material* theMaterial = aTrack.GetMaterial();
  //G4String MaterialName = theMaterial->GetName();
  if (aTrack.GetCurrentStepNumber() > 30000) 
  {

  G4double GlobalTime = aTrack.GetGlobalTime(); 
  printf("%g\n", GlobalTime);
  std::cout << particle->GetParticleName() << std::endl;
      printf("Total Steps: %d\n",  aTrack.GetCurrentStepNumber());
  }

  //G4double KinEng_Int = projectile->GetKineticEnergy();
  //G4ThreeVector MomDir_Int = projectile->GetMomentumDirection();
  //G4ThreeVector thePosition = aTrack.GetPosition();
  //G4TouchableHandle theTouchable = aTrack.GetTouchableHandle();

  if (aTrack.GetCurrentStepNumber() > 30001) 
  {
      aParticleChange.ProposeTrackStatus(fStopAndKill);
      //aParticleChange.ProposeEnergy(0.);
  }

  //zwk Found that Genat4 gets hung up randomly with a position of (nan,nan,nan)
  //Therefore if this happens reset position to previous set
//  if(std::isnan(thePosition.x())||std::isnan(thePosition.y())||std::isnan(thePosition.z())){ 
//    //G4cout << "MENATE_R: Current position is (nan,nan,nan): resetting to presteppoint" << G4endl;
//    G4cout << aTrack.GetMaterial()->GetName() << G4endl;
//    G4cout << aTrack.GetNextMaterial()->GetName() << G4endl;
//    G4cout << aStep.GetPreStepPoint()->GetPosition().z()/cm << G4endl;
//    G4cout << aStep.GetPostStepPoint()->GetPosition().z()/cm << G4endl;
//    G4cout << aStep.GetPreStepPoint()->GetMaterial()->GetName() << G4endl;
//    G4cout << aStep.GetPostStepPoint()->GetMaterial()->GetName() << G4endl;
//
//    thePosition = aStep.GetPreStepPoint()->GetPosition();
//
//  }

  /*
  // These variables not used...
  G4double LocalTime = aTrack.GetLocalTime();
  G4double ProperTime = aTrack.GetProperTime();
  G4double theProjMass = projectile->GetMass();
  */



 return pParticleChange;
}
