/* **********************************************************
 * Copyright (c) 2007-2009 VMware, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of VMware, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/* Containers DynamoRIO Extension: Hashtable */

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_ 1

/***************************************************************************
 * HASHTABLE
 */

/**
 * \addtogroup drcontainers Container Data Structures
 */
/*@{*/ /* begin doxygen group */

/** The type of hash key */
typedef enum {
    HASH_INTPTR,        /**< A pointer-sized integer or pointer */
    HASH_STRING,        /**< A case-sensitive string */
    HASH_STRING_NOCASE, /**< A case-insensitive string */
    HASH_CUSTOM,        /**< A custom key (hash and cmp operations must be provided) */
} hash_type_t;

typedef struct _hash_entry_t {
    void *key;
    void *payload;
    struct _hash_entry_t *next;
} hash_entry_t;

typedef struct _hashtable_t {
    hash_entry_t **table;
    hash_type_t hashtype;
    bool str_dup;
    void *lock;
    uint table_bits;
    bool synch;
    void (*free_payload_func)(void*);
    uint (*hash_key_func)(void*);
    bool (*cmp_key_func)(void*, void*);
} hashtable_t;

/* should move back to utils.c once have iterator and alloc_exit
 * doesn't need this macro
 */
#define HASHTABLE_SIZE(num_bits) (1U << (num_bits))

/** Caseless string compare */
bool
stri_eq(const char *s1, const char *s2);

/**
 * The hashtable has parametrized heap and assert routines for flexibility.
 * This routine must be called BEFORE any other hashtable_ routine; else,
 * the defaults will be used.
 */
void
hashtable_global_config(void *(*alloc_func)(size_t), void (*free_func)(void*, size_t),
                        void (*assert_fail_func)(const char *));

/**
 * Initializes a hashtable with the given size, hash type, and whether to
 * duplicate string keys.  All operations are synchronized by default.
 */
void
hashtable_init(hashtable_t *table, uint num_bits, hash_type_t hashtype, bool str_dup);

/**
 * Initializes a hashtable with the given size, hash type, whether to
 * duplicate string keys, whether to synchronize each operation, a callback
 * for freeing each payload, a callback for hashing a key, and a callback
 * for comparing two keys.  Even when \p synch is false, the hashtable's lock
 * is initialized and can be used via hashtable_lock and hashtable_unlock,
 * allowing the caller to extend synchronization beyond just the operation
 * in question, to include accessing a looked-up payload, e.g.
 */
void
hashtable_init_ex(hashtable_t *table, uint num_bits, hash_type_t hashtype,
                  bool str_dup, bool synch, void (*free_payload_func)(void*),
                  uint (*hash_key_func)(void*), bool (*cmp_key_func)(void*, void*));

/** Returns the payload for the given key, or NULL if the key is not found */
void *
hashtable_lookup(hashtable_t *table, void *key);

/** Like hashtable_lookup but does not unlock the hashtable lock */
void *
hashtable_lookup_keep_locked(hashtable_t *table, void *key);

/**
 * Adds a new entry.  Returns false if an entry for \p key already exists.
 * \note Never use NULL as a payload as that is used for a lookup failure.
 */
bool
hashtable_add(hashtable_t *table, void *key, void *payload);

/**
 * Adds a new entry, replacing an existing entry if any.
 * \note Never use NULL as a payload as that is used for a lookup failure.
 */
void *
hashtable_add_replace(hashtable_t *table, void *key, void *payload);

/**
 * Removes the entry for key.  If free_payload_func was specified calls it
 * for the payload being removed.  Returns false if no such entry
 * exists.
 */
bool
hashtable_remove(hashtable_t *table, void *key);

/**
 * Destroys all storage for the table.  If free_payload_func was specified
 * calls it for each payload. 
 */
void
hashtable_delete(hashtable_t *table);

/** Acquires the hashtable lock. */
void
hashtable_lock(hashtable_t *table);

/** Releases the hashtable lock. */
void
hashtable_unlock(hashtable_t *table);

/*@}*/ /* end doxygen group */

#endif /* _HASHTABLE_H_ */