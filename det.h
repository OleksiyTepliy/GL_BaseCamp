// fill sub matrix, ignores 1-st row and one coulumn from original matrix
void fill_sub_matrix(int size, int original_matrix[][size], int sub_matrix[][size - 1], int ignored_position);

// recursive function, calculate determinant of sqared matrix size X size
int det(int size, int matrix[][size]);