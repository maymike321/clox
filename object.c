#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, extraSize, objectType) \
	(type*)allocateObject(sizeof(type) + extraSize, objectType)

static Obj* allocateObject(size_t size, ObjType type) {
	Obj* object = (Obj*)reallocate(NULL, 0, size);
	object->type = type;

	object->next = vm.objects; 
	vm.objects = object;
	return object;
}

ObjFunction* newFunction() {
	ObjFunction* function = ALLOCATE_OBJ(ObjFunction, 0, OBJ_FUNCTION);
	function->arity = 0;
	function->name = NULL;
	initChunk(&function->chunk);
	return function;
}

static ObjString* allocateString(const char* chars, int length, uint32_t hash, bool constant) {
	ObjString* string = ALLOCATE_OBJ(ObjString, length + 1, OBJ_STRING);
	string->length = length;
	string->constant = constant;
	string->hash = hash;
	memcpy(string->chars, chars, length);
	string->chars[length] = '\0';
	tableSet(&vm.strings, string, NIL_VAL);
	return string;
}

static uint32_t hashString(const char* key, int length) {
	uint32_t hash = 2166136261u;
	for (int i = 0; i < length; i++) {
		hash ^= (uint8_t)key[i];
		hash *= 16777619;
	}
	return hash;
}

ObjString* takeString(char* chars, int length) {
	uint32_t hash = hashString(chars, length);
	ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

	if (interned != NULL) {
		FREE_ARRAY(char, chars, length + 1);
		return interned;
	}
	ObjString* string = allocateString(chars, length, hash, false);
	FREE_ARRAY(char, chars, length + 1);
	return string;
}

ObjString* copyString(const char* chars, int length, bool constant) {
	uint32_t hash = hashString(chars, length);
	ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
	
	if (interned != NULL) return interned;
	return allocateString(chars, length, hash, constant);
}

static void printFunction(ObjFunction* function) {
	printf("<fn %s>", function->name->chars);
}

void printObject(Value value) {
	switch (OBJ_TYPE(value)) {
		case OBJ_STRING:
			printf("%s", AS_CSTRING(value));
			break;
		case OBJ_FUNCTION:
			printFunction(AS_FUNCTION(value));
	}
}