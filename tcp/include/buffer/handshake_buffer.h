#ifndef HANDSHAKE_BUFFER_H
#define HANDSHAKE_BUFFER_H

#include "uthash.h"

uint64_t make_key(uint32_t ip, uint16_t port)
{
    return ((uint64_t)ip << 16) | port;
}

typedef struct
{
    
} handshake_entry_t;

typedef struct
{
    uint64_t key;

    UT_hash_handle hh;
} handshake_hash_entry_t;

handshake_hash_entry_t *handshake_entry_find(handshake_hash_entry_t *head, uint32_t ip, uint16_t port)
{
    uint64_t k = make_key(ip, port);
    handshake_hash_entry_t *e;
    HASH_FIND(hh, head, &k, sizeof(uint64_t), e);
    return e;
}

int handshake_entry_insert(handshake_hash_entry_t *head, uint32_t ip, uint16_t port, int value)
{
    handshake_hash_entry_t *it = handshake_entry_find(head, ip, port);
    if (it == NULL)
    {
        handshake_hash_entry_t *e = (handshake_hash_entry_t *)malloc(sizeof(handshake_hash_entry_t));
        e->key = make_key(ip, port);
        e->value = value;
        HASH_ADD(hh, head, key, sizeof(uint64_t), e);

        return 1;
    }

    it->value = value;
}

int hash_delete(hash_table_t *head, int key)
{
    hash_table_t *it = hash_find(head, key);
    if (it != NULL)
    {
        HASH_DEL(head, it);
        free(it);
        it = NULL;
        return 1;
    }

    return 0;
}

#endif // HANDSHAKE_BUFFER