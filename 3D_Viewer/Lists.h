#ifndef INC_3D_VIEWER_LISTS_H
#define INC_3D_VIEWER_LISTS_H

#include "mesh.h"   // определяет структуру Mesh и все нужные поля

// Указатель на узел списка
typedef struct TNode* Pt;

// Узел списка
typedef struct TNode {
    Mesh ObjData;     // теперь поле называется ObjData и имеет тип Mesh
    Pt PNext;
} TNode;

// Прототипы функций для работы со списком (объявлены в Lists.c)
void AddElemToList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem);
void UpdateObjInList(const Mesh* ObjD, Pt HeadElem);
void DelElemFromList(Pt* HeadElem, Pt* TailElem, const Mesh* Elem);
Mesh GetObjByName(Pt HeadElem, const char* Name);

#endif