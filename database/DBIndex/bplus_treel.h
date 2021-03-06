#ifndef bplus_treeL_H
#define bplus_treeL_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <utility>
using std::vector;
using std::pair;
using std::cout;
using std::endl;
#include "bplus_node.h"
#include "predefined.h"
#include "comparealgo.h"
#include <fcntl.h>
#include <unistd.h>
#ifndef PARA
#ifdef __linux
#define PARA O_RDWR
#elif __APPLE__
#define PARA O_RDWR
#else
#define PARA O_RDWR|O_BINARY
#endif
#endif

/* offsets */
#define OFFSET_META 0
#define OFFSET_BLOCK OFFSET_META + headL_t::getSize()
#define SIZE_NO_CHILDREN leaf_nodeL_t::getSize() - CHILDREN_NUM * sizeof(recordL_t)

/* head information of B+ tree */
struct headL_t{
    int order;             /* `order` of B+ tree */
    int value_size;        /* size of value */
    int key_size;          /* size of key */
    int internal_nodeL_num; /* how many internal nodes */
    int leaf_nodeL_num;     /* how many leafs */
    int height;            /* height of tree (exclude leafs) */
    int slot;               /* where to store new block */
    int root_offset;        /* where is the root of internal nodes */
    int leaf_offset;        /* where is the first leaf */
    int last_leaf_offset;

    headL_t() {order = value_size = key_size = internal_nodeL_num = leaf_nodeL_num = height = slot =
               root_offset = leaf_offset = last_leaf_offset = 0;}
    static int getSize() {return sizeof(headL_t);}
};

/* internal nodes' indexL segment */

/* the encapulated B+ tree */
class bplus_treeL {
   public:
    bplus_treeL(const char *path, bool force_empty = false,
               bool multi_value = false, CompareAlgo *cmp = NULL, int keySize = 20);

    ~bplus_treeL() {
        close_file();
    }

    /* abstract operations */
    int search(const indexL_key &key, indexL_value *value) const;
    int search_range(const indexL_key &left, const indexL_key &right, vector<pair<int, int>> *result) const;
    void search_greater_equal(const indexL_key &key, vector<pair<int, int>> *result);
    void search_less_equal(const indexL_key &key, vector<pair<int, int>> *result);
    void search_greater(const indexL_key &key, vector<pair<int, int>> *result);
    void search_less(const indexL_key &key, vector<pair<int, int>> *result);
    int search_and_remove_multi(const indexL_key &key, int pagenum, int pageposition);
    int remove(const indexL_key &key);
    int insert(const indexL_key &key, indexL_value value);
    int update(const indexL_key &key, indexL_value value);
    int search_all(vector<pair<int, int>> *result);

    void set_MultiValue(bool multi_value) { this->multi_value = multi_value; }
    headL_t get_head() const { return head; }
    static CompareAlgo *global_cmp;

   private:
    char path[512];
    headL_t head;
    bool multi_value;
    CompareAlgo *cmp;

    /* init empty tree */
    void init_from_empty(int keySize);

    /* find indexL */
    int search_indexL(const indexL_key &key) const;
    int search_indexL_l(const indexL_key &key) const;

    /* find leaf */
    int search_leaf_l(int indexL, const indexL_key &key) const;
    int search_leaf_l(const indexL_key &key) const {
        return search_leaf_l(search_indexL_l(key), key);
    }
    int search_leaf(int indexL, const indexL_key &key) const;
    int search_leaf(const indexL_key &key) const {
        return search_leaf(search_indexL(key), key);
    }

    /* remove internal node */
    void remove_from_indexL(int offset, internal_nodeL_t &node,
                           const indexL_key &key);

    void remove_from_indexL_multi(int parent_off, int off, internal_nodeL_t &parent, const indexL_key &key);

    /* insert into leaf without split */
    void insert_recordL_no_split(leaf_nodeL_t *leaf, const indexL_key &key,
                                const indexL_value &value);

    /* add key to the internal node */
    void insert_indexL_keyo_indexL(int offset, const indexL_key &key, int value,
                             int after);
    void insert_indexL_keyo_indexL_no_split(internal_nodeL_t &node, const indexL_key &key,
                                      int value, int prev_off);
    void update_parent_node(int parent, int offset, const indexL_key &key);

    /* change children's parent */
    void reset_indexL_children_parent(indexL_t *begin, indexL_t *end,
                                     int parent);

    template <class T>
    void node_create(int offset, T *node, T *next);

    template <class T>
    void node_remove(T *prev, T *node);

    /* multi-level file open/close */
    mutable FILE * fp = 0;
    mutable int fp_desc;
    mutable int fp_level;
    void open_file(const char *mode = "rb+") const {
        // `rb+` will make sure we can write everywhere without truncating
        // file
        if (fp_level == 0) {
            fp = fopen(path, mode);
            fclose(fp);
        }
        fp_desc = open(path, PARA);
        ++fp_level;
    }

    void close_file() const {
//        if (fp_level == 1) fclose(fp);
        close(fp_desc);
        --fp_level;
    }

    /* alloc from disk */
    int alloc(int size) {
        int slot = head.slot;
        head.slot += size;
        return slot;
    }

    int alloc(leaf_nodeL_t *leaf) {
        leaf->n = 0;
        head.leaf_nodeL_num++;
        head.last_leaf_offset = head.slot;
        return alloc(leaf_nodeL_t::getSize());
    }

    int alloc(internal_nodeL_t *node) {
        node->n = 1;
        head.internal_nodeL_num++;
        return alloc(internal_nodeL_t::getSize());
    }

    void unalloc(leaf_nodeL_t *leaf, int offset) { --head.leaf_nodeL_num; }

    void unalloc(internal_nodeL_t *node, int offset) {
        --head.internal_nodeL_num;
    }

    /* read block from disk */
    int block_read(void *block, int offset, int size) const {
        lseek(fp_desc, offset, SEEK_SET);
        read(fp_desc, block, size);

        return 0;
    }

    template <class T>
    int block_read(T *block, int offset) const {
        return block_read(block, offset, T::getSize());
    }

    /* write block to disk */
    int block_write(void *block, int offset, int size) const {
        lseek(fp_desc, offset, SEEK_SET);
        write(fp_desc, block, size);
        return 0;
    }

    template <class T>
    int block_write(T *block, int offset) const {
        return block_write(block, offset, T::getSize());
    }
};

#endif // bplus_treeL_H
