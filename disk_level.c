//
// Created by Vijay on 23-10-2017.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_level.h"


/*
 * check if the file with given 'fname' already exists; if not
 * creates a file a new file and subsequently calls get_in_order
 * that retrieve nodes from the tree.
 */
int disk_level_push(treemap tree, char *name){

    FILE *fp;
    char *path = "G:\\ClionProjects\\file_test\\";
    char *fname = malloc(strlen(name)+strlen(path)+1);
    strcpy(fname, path);
    strcat(fname, name);

    if ((fp = fopen(fname, "rb")) == NULL) {
        if((fp = fopen(fname, "wb"))) {
            get_in_order(tree->root, tree->size, fp);
            fclose(fp);
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    } else {
        fclose(fp);
        printf("Error: Could not create new file with name '%s'",fname);
        return EXIT_FAILURE;
    }
}

/*
 * writes the key-data pair into the binary file pointed by the file
 * pointer fp.
 */
void handle_push(FILE *fp, char *key, byte_array data, int total_key) {

    static unsigned long int file_size_counter;
    static unsigned int bucket_size_counter;
    static unsigned int key_count;
    unsigned int data_len = 0;
    data_len = (int) data[0];               //retrieve total length of byte-array
    ++key_count;                            //keep track of number of keys written
    static long int p1, p2;
    //check for the fresh file.
    if (file_size_counter == 0) {
        fprintf(fp, "%10s", key);
        p1 = ftell(fp);
        file_size_counter += FILE_HEADER_SIZE;
        fseek(fp, FILE_HEADER_SIZE, SEEK_SET);
    }

    //check for the fresh bucket.
    if (bucket_size_counter == 0) {
        fprintf(fp, "%10s", key);
        p2 = ftell(fp);
        bucket_size_counter += BUCKET_HEADER_SIZE;
        file_size_counter += BUCKET_HEADER_SIZE;
        fseek(fp, (FILE_HEADER_SIZE + BUCKET_HEADER_SIZE), SEEK_SET);
    }

    //in every case write key-data pair to the file.
    fprintf(fp, "%10s", key);
    fwrite(data, data_len, 1, fp);
    file_size_counter += data_len + FILE_KEY_LEN;
    bucket_size_counter += data_len + FILE_KEY_LEN;

    //check for the key that fills the current bucket.
    if (key_count % BUCKET_KEY_LIMIT == 0){
        //goto position of the last key in the bucket header.
        fseek(fp, p2, SEEK_SET);
        //write last key.
        fprintf(fp, "%10s", key);
        //write bucket size.
        fwrite(&bucket_size_counter, BUCKET_SIZE_BYTES, 1, fp);
        //goto the last position in the current file.
        fseek(fp, (bucket_size_counter - FILE_HEADER_SIZE), SEEK_CUR);
        //set bucket size counter to 0.
        bucket_size_counter = 0;
    }

    //check for the incoming last key to the file.
    if (key_count == total_key) {
        //avoid overwrite when 'total_key % BUCKET_KEY_LIMIT == 0'
        if (key_count % BUCKET_KEY_LIMIT != 0) {
            fseek(fp, p2, SEEK_SET);
            fprintf(fp, "%10s", key);
            fwrite(&bucket_size_counter, BUCKET_SIZE_BYTES, 1, fp);
        }
        //goto position of the last key in the file header.
        fseek(fp, p1, SEEK_SET);
        //write last key.
        fprintf(fp, "%10s", key);
        //write number of keys.
        fwrite(&key_count, FILE_NKEY_BYTES, 1, fp);
        //write file size.
        fwrite(&file_size_counter, FILE_SIZE_BYTES, 1, fp);
    }
}

/*
 * traverse the tree in 'inorder' fashion and pass the key-value
 * pair from the nodes to be written to the file pointed by file
 * pointer 'fp'.
 */
void get_in_order(node root, int size, FILE *fp){
    static int i;   //iteration variable to keep track of the recursion stack.

    if (root != NULL) {
        get_in_order(root->left_child, size, fp);
        if(++i>7)
            printf("\nerror: recursion prob\n");
        handle_push(fp, root->key, root->value, size);
        get_in_order(root->right_child, size, fp);
    }
}