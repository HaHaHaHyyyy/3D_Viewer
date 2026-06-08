#include "Lists.h"
#include <cstdio>
#include "mesh.h"   // здесь уже нужно полное определение Mesh для доступа к полям (например, name)
#include <stdlib.h>
#include <string.h>

void AddElemToList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem) {
    printf("AddElemToList: allocating new node\n");
    fflush(stdout);
    Pt NewNode = (Pt)malloc(sizeof(TNode));
    if (!NewNode) { printf("malloc failed!\n"); return; }
    NewNode->ObjData = *Elem;
    NewNode->PNext = NULL;
    if (*HeadElem == NULL) {
        *HeadElem = NewNode;
    } else {
        (*TailElem)->PNext = NewNode;
    }
    *TailElem = NewNode;
}

void DelElemFromList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem) {
    Pt cur = *HeadElem, prev = NULL;
    while (cur) {
        if (cur->ObjData.name && Elem->name && strcmp(cur->ObjData.name, Elem->name) == 0) {
            if (prev == NULL) {
                *HeadElem = cur->PNext;
                if (*HeadElem == NULL) *TailElem = NULL;
            } else {
                prev->PNext = cur->PNext;
                if (cur->PNext == NULL) *TailElem = prev;
            }
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->PNext;
    }
}

void UpdateObjInList(const Mesh* ObjD, Pt HeadElem) {
    Pt cur = HeadElem;
    while (cur) {
        if (cur->ObjData.name && ObjD->name && strcmp(cur->ObjData.name, ObjD->name) == 0) {
            cur->ObjData = *ObjD;
            return;
        }
        cur = cur->PNext;
    }
}

Mesh GetObjByName(Pt HeadElem, const char* Name) {
    Pt cur = HeadElem;
    while (cur) {
        if (cur->ObjData.name && strcmp(cur->ObjData.name, Name) == 0) {
            return cur->ObjData;
        }
        cur = cur->PNext;
    }
    Mesh empty = {0};
    return empty;
}