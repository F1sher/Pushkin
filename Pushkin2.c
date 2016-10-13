#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define ERR_STATUS(s) { free_all_mem(text, ptr_lines, ptr_lines_lock); fprintf(stderr, "ret = %d | ", ret); perror(s); }

#define POEM_FILE   "Tired.txt"


int lines = 0;


int cmp_str(const void *a, const void *b)
{
    const char *s1 = *((const char **)a);
    const char *s2 = *((const char **)b);
    const char *s1_max_ind = s1 + strlen(s1);
    const char *s2_max_ind = s2 + strlen(s2);
    const char *p1 = s1;
    const char *p2 = s2;

    while (1) {
        if ((p1 >= s1_max_ind) || (p2 >= s2_max_ind)) {
            break;
        }
        
        if ((*p1 == '\0') || (*p1 < 'A') || (*p1 > 'z')) {
            p1++;
            
            continue;
        }
        
        if ((*p2 == '\0') || (*p2 < 'A') || (*p2 > 'z')) {
            p2++;
            
            continue;
        }
        
        if (*p1 < *p2) {
            return -1;
        }
        
        if (*p1 > *p2) {
            return 1;
        }
        
        p1++;
        p2++;
    }
    
    return 0;
}


int find_ptr_len(const char *s, char **arr)
{
    int i = 0;
    
    while (s != arr[i]) {
        i++;
    }
    
    if (i == lines-1) {
        return (int)strlen(s);
    }
    else {
        return (int)(arr[i+1]-arr[i]-1);
    }
}


int bsearch_ptr_len(const char *s_key, char **arr)
{
    int start = 0;
    int end = lines-1;
    int mid;
    
    while (start <= end) {
        mid = (start + end) / 2;
        if (s_key == arr[mid])
            if (mid == lines-1) {
                return (int)strlen(s_key);
            }
            else {
                return (int)(arr[mid+1]-arr[mid]-1);
            }
        else if (s_key < arr[mid])
            end = mid - 1;
        else start = mid + 1;
    }
    
    return -1;
}


int cmp_reverse_str(const void *a, const void *b, void *ptr_lines_lock)
{
    const char *s1 = *((const char **)a);
    const char *s2 = *((const char **)b);
    
    int l_s1 = bsearch_ptr_len(s1, (char **)ptr_lines_lock);
    int l_s2 = bsearch_ptr_len(s2, (char **)ptr_lines_lock);
    
    if((l_s1 == -1) || (l_s2 == -1)) {
        errno = EIO;
        
        return 0;
    }
    
    const char *p1 = s1 + l_s1 - 1;
    const char *p2 = s2 + l_s2 - 1;
    
    
    while (1) {
        if((p1 <= s1) || (p2 <= s2)) {
            break;
        }
        
        if ((*p1 == '\0') && (*p2 == '\0')) {
            p1--;
            p2--;
            
            continue;
        }
        
        if ((*p1 == '\0') || (*p1 < 'A') || (*p1 > 'z')) {
            p1--;
            
            continue;
        }
        
        if ((*p2 == '\0') || (*p2 < 'A') || (*p2 > 'z')) {
            p2--;
            
            continue;
        }
        
        if (*p1 < *p2) {
            return -1;
        }
        
        if (*p1 > *p2) {
            return 1;
        }
        
        p1--;
        p2--;
    }
    

    return 0;
}


int write_res(char **ptr_lines, const char *filename, const int lines)
{
    int i = 0, ret = 0, len = 0;
    FILE *fp = fopen(filename, "w+");
    
    if(fp == NULL) {
        return -1;
    }
    
    for (i = 0; i <= lines-1; i++) {
        len = strlen(ptr_lines[i]);
        
        ret = fprintf(fp, "%s\n", ptr_lines[i]);
        if(ret < len) {
            return -2;
        }
    }
    
    ret = fclose(fp);
    
    if(ret != 0) {
        return -3;
    }

    return 0;
}


int find_lines(char *text, const int file_size, char **ptr_lines, const int lines)
{
    int i = 0, j = 0;
    
    ptr_lines[0] = text;
    for (i = 0; i <= file_size-2; i++) {
        if(text[i] == '\n') {
            j++;
            ptr_lines[j] = text + i + 1;
            text[i] = '\0';
        }
    }
    text[i] = '\0';
    
    return 0;
}


int read_poem(const char *poem_name, char **text, char ***ptr_lines, char ***ptr_lines_lock, int *lines)
{
    FILE *fp_poem = NULL;
    int i = 0, file_size = 0, ret = 0;
     
    fp_poem = fopen(poem_name, "r");
    if (fp_poem == NULL) {
        return -1;
    }
    
    while (!feof(fp_poem)) {
        if (fgetc(fp_poem) == '\n') {
            (*lines)++;
        }
    }
    file_size = ftell(fp_poem);
    rewind(fp_poem);
    
    *text = (char *)calloc(file_size, sizeof(char));
    *ptr_lines = (char **)calloc(*lines, sizeof(char *));
    *ptr_lines_lock = (char **)calloc(*lines, sizeof(char *)); 
    if((text == NULL) || (ptr_lines == NULL) || (ptr_lines_lock == NULL)) {
        return -2;
    }
    
    ret = fread(*text, sizeof(char), file_size, fp_poem);
    if (ret < file_size) {
        return -2;
    }
    
    ret = fclose(fp_poem);
    if(ret != 0) {
        return -3;
    }
    
    find_lines(*text, file_size, *ptr_lines, *lines);
    
    for (i = 0; i <= *lines-1; i++) {
        (*ptr_lines_lock)[i] = (*ptr_lines)[i];
    }
    
    return 0;
}

void free_all_mem(char *text, char **ptr_lines, char **ptr_lines_lock)
{
    free(text);
    text = NULL;
    free(ptr_lines);
    ptr_lines = NULL;
    free(ptr_lines_lock);
    ptr_lines_lock = NULL;
}


int main(int argc, char **argv)
{
    char *text = NULL;
    char **ptr_lines = NULL;
    char **ptr_lines_lock = NULL;
    int ret = 0;
    
    ret = read_poem(POEM_FILE, &text, &ptr_lines, &ptr_lines_lock, &lines);
    if (ret != 0) {
        ERR_STATUS("Error in read_poem func")
    
        return -1;
    }
    
    //printf("%s %d\n", text, strlen(text));
    //printf("%p %p\n", text, ptr_lines_lock);
    
    qsort(ptr_lines, lines, sizeof(char **), cmp_str);
    ret = write_res(ptr_lines, "encyclop.txt", lines);
    if (ret != 0) {
        ERR_STATUS("Error in write encyclop.txt")
        
        return -1;
    }

    qsort_r(ptr_lines, lines, sizeof(char **), cmp_reverse_str, ptr_lines_lock);
    ret = write_res(ptr_lines, "rhymes.txt", lines);
    if (ret != 0) {
        ERR_STATUS("Error in write rhymes.txt")
        
        return -1;
    }
   
   /* 
    int i = 0;
    for(i=0; i <= lines-1; i++) {
        printf("%s\n", ptr_lines[i]);
    }
    */
    
    free_all_mem(text, ptr_lines, ptr_lines_lock);

	return 0;
}
