#ifndef LISTS_H
#define LISTS_H

#include "mesh.h"   // после исправления mesh.h не включает Lists.h

typedef struct TNode {
    Mesh ObjData;
    struct TNode* PNext;
} TNode;

typedef TNode* Pt;

void AddElemToList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem);
void UpdateObjInList(const Mesh* ObjD, Pt HeadElem);
void DelElemFromList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem);
Mesh GetObjByName(Pt HeadElem, const char* Name);

#endif