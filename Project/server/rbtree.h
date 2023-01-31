#ifndef JSW_RBTREE_H
#define JSW_RBTREE_H

#ifdef __cplusplus
#include <cstddef>

using std::size_t;

extern "C" {
#else
#include <stddef.h>
#endif

/* Opaque types */
typedef struct rbtree rbtree_t;
typedef struct rbtrav rbtrav_t;

/* User-defined item handling */
typedef int   (*cmp_f) ( const void *p1, const void *p2 );
typedef void *(*dup_f) ( void *p );
typedef void  (*rel_f) ( void *p );

/* Red Black tree functions */
rbtree_t *rbnew ( cmp_f cmp, dup_f dup, rel_f rel );
void          rbdelete ( rbtree_t *tree );
void         *rbfind ( rbtree_t *tree, void *data );
int           rbinsert ( rbtree_t *tree, void *data );
int           rberase ( rbtree_t *tree, void *data );
size_t        rbsize ( rbtree_t *tree );

/* Traversal functions */
rbtrav_t *rbtnew ( void );
void          rbtdelete ( rbtrav_t *trav );
void         *rbtfirst ( rbtrav_t *trav, rbtree_t *tree );
void         *rbtlast ( rbtrav_t *trav, rbtree_t *tree );
void         *rbtnext ( rbtrav_t *trav );
void         *rbtprev ( rbtrav_t *trav );

#ifdef __cplusplus
}
#endif

#endif
