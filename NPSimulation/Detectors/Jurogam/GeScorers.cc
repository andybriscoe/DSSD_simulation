/*****************************************************************************
 * Copyright (C) 2009-2016   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

#include <algorithm>
#include "GeScorers.hh"
#include "G4UnitsTable.hh"
using namespace std;
using namespace CLHEP;
using namespace GESCORERS;



//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
PS_Ge::PS_Ge(G4String name, vector<G4int> NestingLevel,G4int depth)
    :G4VPrimitiveScorer(name, depth){
        HCID=-1;
        m_Position = G4ThreeVector(-1000,-1000,-1000);
        m_LocalPosition = G4ThreeVector(-1000,-1000,-1000);
        m_GeNumber  = -1;
        m_CrystalNumber = -1;
        m_Index = -1 ;
        m_NestingLevel = NestingLevel;
    }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
PS_Ge::~PS_Ge(){
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4bool PS_Ge::ProcessHits(G4Step* aStep, G4TouchableHistory*){
    // contain Energy Time, DetNbr, StripFront and StripBack
    unsigned int mysize = m_NestingLevel.size();
    //G4double* Infos = new G4double[2+mysize];
    G4double* Infos = new G4double[4];
    Infos[0] = aStep->GetTotalEnergyDeposit();
    Infos[1] = aStep->GetPreStepPoint()->GetGlobalTime();
    
    //Infos[2] = aStep->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber(m_NestingLevel[1]);
    for (unsigned int i = 0 ; i < mysize ; i++)
    {
        Infos[2] = aStep->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber(m_NestingLevel[i]);
    }

    m_Index = 0;
    G4int multiplier = 1;
    for (unsigned int i = 0 ; i < mysize ; i++)
    {
        m_Index+= aStep->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber(m_NestingLevel[i])*multiplier;
        multiplier*=10;
    }

    G4String VolumeName = aStep->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();
    int crystal = -1;
    size_t found=VolumeName.find("clover");
    if (found!=std::string::npos)
    {
        vector<G4String> v_VolumeName;
        G4String token;
        istringstream tokenStream(VolumeName);

        while (getline(tokenStream, token, '_'))
        {
            v_VolumeName.push_back(token);
        }

        crystal = stoi(v_VolumeName[2]);
    }
    Infos[3] = crystal;


    map<G4int, G4double**>::iterator it;
    it= EvtMap->GetMap()->find(m_Index);
    if(it!=EvtMap->GetMap()->end()){
        G4double* dummy = *(it->second);
        Infos[0]+=dummy[0];
        delete dummy;
    }
    EvtMap->set(m_Index, Infos);
    return TRUE;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PS_Ge::Initialize(G4HCofThisEvent* HCE){
    EvtMap = new NPS::HitsMap<G4double*>(GetMultiFunctionalDetector()->GetName(), GetName());
    if (HCID < 0) {
        HCID = GetCollectionID(0);
    }
    HCE->AddHitsCollection(HCID, (G4VHitsCollection*)EvtMap);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PS_Ge::EndOfEvent(G4HCofThisEvent*){
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PS_Ge::clear(){
    std::map<G4int, G4double**>::iterator    MapIterator;
    for (MapIterator = EvtMap->GetMap()->begin() ; MapIterator != EvtMap->GetMap()->end() ; MapIterator++){
        delete *(MapIterator->second);
    }

    EvtMap->clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PS_Ge::DrawAll(){

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void PS_Ge::PrintAll(){
    G4cout << " MultiFunctionalDet  " << detector->GetName() << G4endl ;
    G4cout << " PrimitiveScorer " << GetName() << G4endl               ;
    G4cout << " Number of entries " << EvtMap->entries() << G4endl     ;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//Info_Calorimeter::GetVolumeName(){return VolumeName; }
