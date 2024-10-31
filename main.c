#include <stdbool.h>
#include <stdlib.h>

/**
 * define a new btree:
 *
 * args:
 *      @LINKAGE:       linkage of generated functions
 *      @TYPE:          element type
 *      @DEGREE:        degree of btree nodes
 *      @CMP_FN:        element comparison function
 *      @NAME:          name of generated struct
 *
 * ret:
 *      @success:       generated btree struct and functions
 *      @failure:       does not fail
 */
#define BTREE_DEFINE(LINKAGE, TYPE, DEGREE, CMP_FN, NAME)               \
                                                                        \
/* btree node */                                                        \
struct NAME ## _node {                                                  \
        /* array of pointers to children */                             \
        struct NAME ## _node    *bn_kids[(DEGREE << 1)];                \
        /* elements */                                                  \
        TYPE                    bn_elem[(DEGREE << 1) - 1];             \
        /* is this node a leaf? */                                      \
        bool                    bn_leaf;                                \
        /* number of keys in node */                                    \
        int                     bn_len;                                 \
};                                                                      \
                                                                        \
/* btree */                                                             \
struct NAME {                                                           \
        /* root node of btree */                                        \
        struct NAME ## _node    *b_root;                                \
};                                                                      \
                                                                        \
/**                                                                     \
 * initialize a NAME:                                                   \
 *                                                                      \
 * args:                                                                \
 *      @bp:            pointer to NAME                                 \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       nothing                                         \
 *      @failure:       does not fail                                   \
 */                                                                     \
LINKAGE void                                                            \
NAME ## _init(struct NAME *bp)                                          \
{                                                                       \
        bp->b_root = NULL;                                              \
}                                                                       \
                                                                        \
/**                                                                     \
 * create a new NAME ## _node (for internal use only):                  \
 *                                                                      \
 * args:                                                                \
 *      @leaf:          is this node a leaf?                            \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       pointer to new NAME ## _node                    \
 *      @failure:       NULL and errno set                              \
 */                                                                     \
LINKAGE struct NAME ## _node *                                          \
NAME ## _node_new(bool leaf)                                            \
{                                                                       \
        struct NAME ## _node *np = NULL;                                \
                                                                        \
        np = malloc(sizeof(*np));                                       \
        if (np == NULL)                                                 \
                return NULL;                                            \
                                                                        \
        np->bn_leaf = leaf;                                             \
        np->bn_len = 0;                                                 \
        return np;                                                      \
}                                                                       \
                                                                        \
/**                                                                     \
 * split a NAME ## _node (for internal use only):                       \
 *                                                                      \
 * args:                                                                \
 *      @np:            pointer to NAME ## _node                        \
 *      @kid:           kid to split                                    \
 *      @idx:           index to split on                               \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       0                                               \
 *      @failure:       -1 and errno set                                \
 */                                                                     \
LINKAGE int                                                             \
NAME ## _node_split(struct NAME ## _node *np,                           \
                    struct NAME ## _node *kid,                          \
                    int idx)                                            \
{                                                                       \
        struct NAME ## _node *new = NULL;                               \
        int i = -1;                                                     \
                                                                        \
        new = NAME ## _node_new(kid->bn_leaf);                          \
        if (new == NULL)                                                \
                return -1;                                              \
                                                                        \
        new->bn_len = DEGREE - 1;                                       \
        for (i = 0; i < (DEGREE - 1); i++)                              \
                new->bn_elem[i] = kid->bn_elem[DEGREE + i];             \
        if (!kid->bn_leaf) {                                            \
                for (i = 0; i < DEGREE; i++)                            \
                        new->bn_kids[i] = kid->bn_kids[DEGREE + i];     \
        }                                                               \
        kid->bn_len = DEGREE - 1;                                       \
                                                                        \
        for (i = np->bn_len; i >= idx + 1; i--)                         \
                np->bn_kids[i + 1] = np->bn_kids[i];                    \
        np->bn_kids[idx + 1] = new;                                     \
                                                                        \
        for (i = np->bn_len - 1; i >= idx; i--)                         \
                np->bn_elem[i + 1] = np->bn_elem[i];                    \
        np->bn_elem[idx] = kid->bn_elem[DEGREE - 1];                    \
                                                                        \
        np->bn_len++;                                                   \
        return 0;                                                       \
}                                                                       \
                                                                        \
/**                                                                     \
 * add an element to NAME ## _node (for internal use only):             \
 *                                                                      \
 * args:                                                                \
 *      @np:            pointer to NAME ## _node                        \
 *      @elem:          element to add                                  \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       0                                               \
 *      @failure:       -1 and errno set                                \
 */                                                                     \
LINKAGE int                                                             \
NAME ## _node_add(struct NAME ## _node *np, TYPE elem)                  \
{                                                                       \
        struct NAME ## _node *kid = NULL;                               \
        int i = -1;                                                     \
                                                                        \
        if (np->bn_leaf) {                                              \
                i = np->bn_len - 1;                                     \
                while (i >= 0 && CMP_FN(np->bn_elem[i], elem) > 0) {    \
                        np->bn_elem[i + 1] = np->bn_elem[i];            \
                        i--;                                            \
                }                                                       \
                np->bn_elem[i + 1] = elem;                              \
                np->bn_len++;                                           \
                return 0;                                               \
        }                                                               \
                                                                        \
        i = np->bn_len - 1;                                             \
        while (i >= 0 && CMP_FN(np->bn_elem[i], elem) > 0)              \
                i--;                                                    \
                                                                        \
        if (np->bn_kids[i + 1]->bn_len == ((DEGREE << 1) - 1)) {        \
                kid = np->bn_kids[i + 1];                               \
                if (NAME ## _node_split(np, kid, i + 1) < 0)            \
                        return -1;                                      \
                if (CMP_FN(np->bn_elem[i + 1], elem) < 0)               \
                        i++;                                            \
        }                                                               \
                                                                        \
        NAME ## _node_add(np->bn_kids[i + 1], elem);                    \
        return 0;                                                       \
}                                                                       \
                                                                        \
/**                                                                     \
 * add element to NAME:                                                 \
 *                                                                      \
 * args:                                                                \
 *      @bp:            pointer to NAME                                 \
 *      @elem:          element to add                                  \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       0                                               \
 *      @failure:       -1 and errno set                                \
 */                                                                     \
LINKAGE int                                                             \
NAME ## _add(struct NAME *bp, TYPE elem)                                \
{                                                                       \
        struct NAME ## _node *new = NULL;                               \
        int i = -1;                                                     \
                                                                        \
        if (bp->b_root == NULL) {                                       \
                bp->b_root = NAME ## _node_new(true);                   \
                if (bp->b_root == NULL)                                 \
                        return -1;                                      \
                bp->b_root->bn_elem[0] = elem;                          \
                bp->b_root->bn_len = 1;                                 \
                return 0;                                               \
        }                                                               \
                                                                        \
        if (bp->b_root->bn_len == ((DEGREE << 1) - 1)) {                \
                new = NAME ## _node_new(false);                         \
                if (new == NULL)                                        \
                        return -1;                                      \
                new->bn_kids[0] = bp->b_root;                           \
                if (NAME ## _node_split(new, bp->b_root, 0) < 0) {      \
                        free(new);                                      \
                        return -1;                                      \
                }                                                       \
                i = 0;                                                  \
                if (CMP_FN(new->bn_elem[0], elem) < 0)                  \
                        i++;                                            \
                (void)NAME ## _node_add(new->bn_kids[i], elem);         \
                bp->b_root = new;                                       \
                return 0;                                               \
        }                                                               \
                                                                        \
        return NAME ## _node_add(bp->b_root, elem);                     \
}                                                                       \
                                                                        \
/**                                                                     \
 * iterate through NAME ## _node elements (for internal use only):      \
 *                                                                      \
 * args:                                                                \
 *      @np:            pointer to NAME ## _node                        \
 *      @fn:            pointer to function to run on elements          \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       nothing                                         \
 *      @failure:       does not fail                                   \
 */                                                                     \
LINKAGE void                                                            \
NAME ## _node_for_each(struct NAME ## _node *np, void (*fn)(TYPE))      \
{                                                                       \
        int i = -1;                                                     \
                                                                        \
        if (np == NULL)                                                 \
                return;                                                 \
                                                                        \
        for (i = 0; i < np->bn_len; i++) {                              \
                if (!np->bn_leaf)                                       \
                        NAME ## _node_for_each(np->bn_kids[i], fn);     \
                fn(np->bn_elem[i]);                                     \
        }                                                               \
                                                                        \
        if (!np->bn_leaf)                                               \
                NAME ## _node_for_each(np->bn_kids[i], fn);             \
}                                                                       \
                                                                        \
/**                                                                     \
 * iterate through NAME elements:                                       \
 *                                                                      \
 * args:                                                                \
 *      @bp:            pointer to NAME                                 \
 *      @fn:            pointer to function to run on elements          \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       nothing                                         \
 *      @failure:       does not fail                                   \
 */                                                                     \
LINKAGE void                                                            \
NAME ## _for_each(struct NAME *bp, void (*fn)(TYPE))                    \
{                                                                       \
        NAME ## _node_for_each(bp->b_root, fn);                         \
}                                                                       \
                                                                        \
/**                                                                     \
 * free memory allocated by NAME ## _node (for internal use only):      \
 *                                                                      \
 * args:                                                                \
 *      @np:            pointer to NAME ## _node                        \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       nothing                                         \
 *      @failure:       does not fail                                   \
 */                                                                     \
LINKAGE void                                                            \
NAME ## _node_free(struct NAME ## _node *np)                            \
{                                                                       \
        int i = -1;                                                     \
                                                                        \
        if (np == NULL)                                                 \
                return;                                                 \
                                                                        \
        for (i = 0; i < np->bn_len; i++) {                              \
                if (!np->bn_leaf)                                       \
                        NAME ## _node_free(np->bn_kids[i]);             \
        }                                                               \
                                                                        \
        if (!np->bn_leaf)                                               \
                NAME ## _node_free(np->bn_kids[i]);                     \
                                                                        \
        free(np);                                                       \
}                                                                       \
                                                                        \
/**                                                                     \
 * free memory allocated by NAME:                                       \
 *                                                                      \
 * args:                                                                \
 *      @bp:            pointer to NAME                                 \
 *                                                                      \
 * ret:                                                                 \
 *      @success:       nothing                                         \
 *      @failure:       does not fail                                   \
 */                                                                     \
LINKAGE void                                                            \
NAME ## _free(struct NAME *bp)                                          \
{                                                                       \
        NAME ## _node_free(bp->b_root);                                 \
        bp->b_root = NULL;                                              \
}

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

BTREE_DEFINE(, char *, 8, strcmp, strlist)

static void
printstr(char *s)
{
        /* printf("%s", s); */
        free(s);
}

int
main(int argc, char **argv)
{
        struct strlist args = {0};
        char buf[BUFSIZ] = "";
        char *dup = NULL;

        strlist_init(&args);
        while (fgets(buf, sizeof(buf), stdin) != NULL) {
                dup = strdup(buf);
                if (dup == NULL)
                        err(EX_SOFTWARE, "strdup");
                if (strlist_add(&args, dup) < 0)
                        err(EX_SOFTWARE, "strlist_add");
        }

        strlist_for_each(&args, printstr);
        strlist_free(&args);
}
