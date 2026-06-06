// Lists.c
#include "Lists.h"
#include <stdlib.h>
#include <string.h>

void AddElemToList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem) {
    Pt NewNode = (Pt)malloc(sizeof(TNode));
    NewNode->ObjData = *Elem;   // поверхностное копирование
    NewNode->PNext = NULL;

    if (*HeadElem == NULL) {
        *HeadElem = NewNode;
    } else {
        (*TailElem)->PNext = NewNode;
    }
    *TailElem = NewNode;
}

void DelElemFromList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem) {
    Pt current = *HeadElem, prev = NULL;
    while (current != NULL) {
        if (current->ObjData.name && Elem->name && strcmp(current->ObjData.name, Elem->name) == 0) {
            if (prev == NULL) {
                *HeadElem = current->PNext;
                if (*HeadElem == NULL) *TailElem = NULL;
            } else {
                prev->PNext = current->PNext;
                if (current->PNext == NULL) *TailElem = prev;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->PNext;
    }
}

void UpdateObjInList(const Mesh* ObjD, Pt HeadElem) {
    Pt cur = HeadElem;
    while (cur != NULL) {
        if (cur->ObjData.name && ObjD->name && strcmp(cur->ObjData.name, ObjD->name) == 0) {
            cur->ObjData = *ObjD;
            return;
        }
        cur = cur->PNext;
    }
}

Mesh GetObjByName(Pt HeadElem, const char* Name) {
    Pt cur = HeadElem;
    while (cur != NULL) {
        if (cur->ObjData.name && strcmp(cur->ObjData.name, Name) == 0) {
            return cur->ObjData;
        }
        cur = cur->PNext;
    }
    Mesh empty = {0};
    return empty;
}