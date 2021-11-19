//Nathan Sexton

#include "MMHeader.h"

//struct
typedef struct __matrix
{
	int num_rows;
	int num_cols;
	double* elements;

} matrix;

//function declarations
matrix* matrix_malloc(int, int);
void matrix_free(matrix*);
void set_element(matrix*, int, int, double);
double get_element(matrix*, int, int);
void display(matrix*);
matrix* multiply(matrix*, matrix*);


//functions
matrix* matrix_malloc(int num_rows, int num_cols)
{
	//call mem manager malloc on matrix struct
	matrix* m = (matrix*)mem_manager_malloc(sizeof(matrix));
	m->num_rows = num_rows;
	m->num_cols = num_cols;
	//size is the rows multplied by columns
	int s = num_cols*num_rows;
	m->elements = (double*)mem_manager_malloc(s*sizeof(double));
	//fill the array of elements up with 0s
	for(int i = 0; i < s; i++)
	{
		m->elements[i] = 0.0;
	}
	return m;
}

//requires two mallocs--  malloc space for the struct, then malloc space for the array, 
//store the returned address for the array in the struct
void matrix_free(matrix* mat)  //requires two frees
{
	mem_manager_free((void*)mat);
}

void set_element(matrix* mat, int row, int col, double val)
{
	int cols = mat->num_cols;
	mat->elements[(row - 1)*cols + col - 1] = val;
}

//to convert from (row, col) to index: int index = (row - 1)*cols + col - 1;
double get_element(matrix* mat, int row, int col)
{
	int cols = mat->num_cols;
	int index = (row - 1)*cols + col - 1;
	return mat->elements[index];
}

void display(matrix* mat)
{
	int cols = mat->num_cols;
	int rows = mat->num_rows;

	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j < cols; j++)
		{
			double var = mat->elements[(i)*cols + j];
			printf("%f ", var);
		}
		printf("\n\n\n");
	}
}

//left cols has to be the same as right rows for matrix multiplication
matrix* multiply(matrix* left, matrix* right)
{
	int left_rows = left->num_rows;
	int left_cols = left->num_cols;
	int right_rows = right->num_rows;
	int right_cols = right->num_cols;
	matrix* result = matrix_malloc(left_rows, right_cols);
	
	for (int i = 1; i <= left_rows; i++)
	{
		for (int j = 1; j <= right_cols; j++)
		{
			double val = 0;
			for (int k = 1; k <= left_cols; k++)
			{
				double element_left = get_element(left, i, k);
				double element_right = get_element(right, k, j);
				double mul = element_left * element_right;
				val += mul;
			}
			set_element(result, i, j, val);
		}
	}
	
	return result;
}