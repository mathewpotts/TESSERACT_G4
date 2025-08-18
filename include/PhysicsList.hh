#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"
#include "QGSP_BERT_HP.hh"

class PhysicsList : public QGSP_BERT_HP
{
public:
    PhysicsList();
    ~PhysicsList();
};

#endif
