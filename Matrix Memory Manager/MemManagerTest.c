//Nathan Sexton

#include "Matrix.h"
#include "MMHeader.h"

matrix* read_file(char* file_name)
{
    matrix* mat = 0;

    int num_rows = 0;
    int num_cols = 0;

    FILE* f_matrix = fopen(file_name, "r");
    fscanf(f_matrix, "%d", &num_rows);
    fscanf(f_matrix, "%d", &num_cols);
    mat = matrix_malloc(num_rows, num_cols);

    int num_elements = num_rows*num_cols;
    for (int i = 1; i <= num_elements; i++)
    {
        double* elements = mat->elements;
        double element = 0;
        fscanf(f_matrix, "%lf", &element);
        elements[i-1] = element;
        //printf("%.2lf", element);
    }

    fclose(f_matrix);
    return mat;
}
//the following line is 29 in the video
int main(int argc, char** argv)
{
    init_mem(4096);
    //
    void* ptr = mem_manager_malloc(20);

    *((int*) ptr) = 11;
    printf("%p\n", ptr);

    //int* temp = ((int*)ptr) + 1;
    void* temp = ptr + 1;
    printf("%p\n", temp);



    printf("%d\n", *((int*) ptr));
    mem_manager_free(ptr);
    printf("%d\n", *((int*) ptr));

    void* ptr1 = mem_manager_malloc(100);
    printf("%p\n", ptr1);
    void* ptr2 = mem_manager_malloc(100);
    printf("%p\n", ptr2);
    void* ptr3 = mem_manager_malloc(100);
    printf("%p\n", ptr3);
    mem_manager_free(ptr2);

    traverse_free_list();
    //


    traverse_free_list();

    char* mat_1_file_name = "mat_1.txt";
    matrix* mat_1 = read_file(mat_1_file_name);

    display(mat_1);
    traverse_free_list();
    /*
    char* mat_2_file_name = "mat_2.txt";
    matrix* mat_2 = read_file(mat_2_file_name);

    display(mat_2);
    traverse_free_list();

    matrix_free(mat_1);
    traverse_free_list();
*/
return 0;
}