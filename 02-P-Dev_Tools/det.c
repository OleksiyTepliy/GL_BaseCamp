
void fill_sub_matrix(int n, int m[][n], int temp[][n - 1], int pos){
    
    int ti = 0, tj = 0;    // sub matrix idexes  
    for(int i = 1; i < n; i++, ti++){     // ignore 1st row in original matrix, i = 1
        tj = 0;    
        for(int j = 0; j < n; j++){
            if(j != pos){                  // ignore pos coulumn
                temp[ti][tj++] = m[i][j];
            }
        }
    }
}

int det(int n, int m[][n]){
    //base cases
    if(n == 1){
        return m[0][0];
    }
    if(n == 2){
        return m[0][0] * m[1][1] - m[0][1] * m[1][0];   // determinant of 2 x 2 matrix
    }
    int DET = 0;
    int temp[n - 1][n - 1]; // sub matrix storage
    int sign = 1;
    for(int i = 0; i < n; i++, sign = -sign){
        fill_sub_matrix(n, m, temp, i);
        DET += sign * m[0][i] * det(n - 1, temp);
    }
    return DET;
}

