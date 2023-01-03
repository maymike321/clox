#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->lineCount = 0;
    chunk->capacity = 0;
    chunk->lineCapacity = 0;
    chunk->lines = NULL;
    chunk->code = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;

    if (chunk->lineCount < line) {
        if (chunk->lineCapacity < line) {
            int oldLineCapacity = chunk->lineCapacity;
            int previousOldLineCapacity = oldLineCapacity;
            while (chunk->lineCapacity < line) {
                chunk->lineCapacity = GROW_CAPACITY(previousOldLineCapacity);
                previousOldLineCapacity = chunk->lineCapacity;
            }
            chunk->lines = GROW_ARRAY(int, chunk->lines, oldLineCapacity, chunk->lineCapacity);
        }
        chunk->lines[line - 1] = 1;
        chunk->lineCount = line;
    }
    else {
        chunk->lines[line - 1] = chunk->lines[line - 1] + 1;
    }
}

void writeConstant(Chunk* chunk, Value value, int line) {
    int constant = addConstant(chunk, value);
    if (constant > UINT8_MAX) {
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, constant >> 16 & 255, line);
        writeChunk(chunk, constant >> 8 & 255, line);
        writeChunk(chunk, constant & 255, line);
    }
    else {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, constant, line);
    }
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

int getLine(Chunk* chunk, int offset) {
    int line = 0;
    while (offset >= 0 && line < chunk->lineCount) {
        offset -= chunk->lines[line];
        line++;
    }
    return line;
}