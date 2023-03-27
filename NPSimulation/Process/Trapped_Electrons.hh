#ifndef Trapped_electrons_hh 
#define Trapped_electrons_hh

#include "globals.hh"
#include "G4VDiscreteProcess.hh"
#include "G4ios.hh"
#include "Randomize.hh" 
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
//#include "G4IonTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4DynamicParticle.hh"
//#include "G4ThreeVector.hh"
//#include "G4LorentzVector.hh"
#include "G4VParticleChange.hh"

#include "G4Material.hh"
#include "G4UnitsTable.hh"


class Trapped_Electrons: public G4VDiscreteProcess
{
public:

  // constructor
  Trapped_Electrons(const G4String& processName="Trapped_Electrons");

  // destructor
  ~Trapped_Electrons();

public:
  // These are the functions in the Process: Derived from G4VDiscreteProcess.hh

  G4bool IsApplicable(const G4ParticleDefinition& aParticle);
  // Decides if process applicable or not. 

  G4double GetMeanFreePath(const G4Track& aTrack, G4double previousStepSize,
                           G4ForceCondition* condition); 
  // Returns MeanFreePath for a particle in a given material in mm
  
  G4VParticleChange* PostStepDoIt(const G4Track &aTrack, const G4Step &aStep);
  // This is the important function where you define the process
  // Returns steps, track and secondary particles

  void SetKill();
private:

 // Hide assignment operator as private 
  Trapped_Electrons& operator=(const Trapped_Electrons&right);

  // Copy constructor
  Trapped_Electrons(const Trapped_Electrons&);

};
#endif

