/*********************************************************************************
* Copyright (C) 2015-2017 Alexey V. Akimov
*
* This file is distributed under the terms of the GNU General Public License
* as published by the Free Software Foundation, either version 2 of
* the License, or (at your option) any later version.
* See the file LICENSE in the root directory of this distribution
* or <http://www.gnu.org/licenses/>.
*
*********************************************************************************/
/**
  \file base_matrix.h
  \brief The file describes the base_matrix class for representing arbitrary size arbitrary-data type-valued matrices    
*/


#ifndef base_matrix_H
#define base_matrix_H

#include <complex>
#include <vector>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>


#include "../io/libio.h"
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>



/// liblibra 
namespace liblibra{

using namespace libio;
using namespace boost::python;
using namespace std;


/// liblinalg namespace
namespace liblinalg{



template <typename T1>
class base_matrix{
/**
  The class representing an arbitrary-sized arbitrary-type valued matrices

  It implements ONLY the methods and operators overloads that do not return anything of the 
  same base class. Those operators/functions will be implemented in the derived classes, to
  avoid confusions with the data types.

  This has been a hard decision, but other approaches seem to be even less suitable:
  1) tempate specialization - would require re-implementing same methods for different data types
  2) inheriting methods that return the templated data class (e.g. base_matrix<double>) would lead to
  a confusion with the derived classes (e.g., MATRIX::base_matrix<double>). One way to make everything
  consistent is to use converters, but that would incur extra consts for copy-constructors etc.
  That is why, we are going to ensure that MATRIX class is not just a typedef base_matrix<double>

  We stil do not want to re-implement common methods, so we'll create free templated functions here
  acting on even more basic datatypes and then, will re-use them in the derived class, yet keeping in mind
  the above considerations
*/

public:
  int n_rows;  ///< The number of rows
  int n_cols;  ///< The number of colomns
  int n_elts;  ///< The number of elements

  T1* M;        ///< The internal storage of the matrix elements



  ///========= Constructors and destructors ===============
  ///< Constructors
  base_matrix(){ 
//    cout<<"In base constructor 1\n";
  n_rows = n_cols = n_elts = 0; M = NULL;} ///< Default constructor

  base_matrix(int n_rows_,int n_cols_){ 
  /** Generates the complex matrix with given number of rows and coloumns */
//    cout<<"In base constructor 2\n";

    n_rows = n_rows_; n_cols = n_cols_; 
    n_elts = n_rows * n_cols;

    M = new T1[n_elts];

    for(int i=0;i<n_elts;i++){  M[i] = (T1)0.0;   }
  }

  ///< Copy constructor
  base_matrix(const base_matrix<T1>& ob){

//    cout<<"In base cctor\n";

    n_rows = ob.n_rows;
    n_cols = ob.n_cols;
    n_elts = ob.n_elts;

    M = new T1[n_elts];
    for(int i=0;i<n_elts;i++){ M[i] = ob.M[i];  }

  }

  ///< Destructor
  ~base_matrix(){ 

//    cout<<"In base destructor\n";
    delete [] M; 
    M = NULL;
    n_rows = n_cols = n_elts = 0;
  } 


  ///========== Getters and setters ====================
  void set(int i, T1 val){ 
  /** Sets the indx's emelent of the M array to the input value  
  if the index i is set to -1, it sets all matrix elements
  to the specified value
  */

    if(i==-1){ 
      for(int j=0;j<n_elts;j++){ M[j] = val; }
    }
    else{   M[i] = val;  }
  } 

  void set(int i, int j, T1 val){ 
  /** Sets the "row","col" matrix emelent of the M array to the input value 
  If row == -1, set all matrix elements in the specified (by index col) 
  column to the provided value 
  If col == -1, set all matrix elements in the specified (by index row) 
  row to the provided value 
  If row == -1 and col == -1, set all matrix elements to the specified value
  */

//  cout<<"In base::set\n";

    if(i==-1 && j==-1){ 
      for(int k=0;k<n_elts;k++){ M[k] = val; }
    }
    else if(i==-1 && j!=-1){
//      cout<<"...Case 1\n";
      for(int k=0;k<n_rows;k++){ 
//        cout<<k<<"  "<<M[k*n_cols + j]<<endl;
        M[k*n_cols + j] = val; 
      }
    }
    else if(i!=-1 && j==-1){
//      cout<<"...Case 2\n";
      for(int k=0;k<n_cols;k++){ 
//        cout<<k<<"  "<<M[i*n_cols + k]<<endl;
        M[i*n_cols + k] = val; 
      }
    }
    else{  M[i*n_cols+j] = val;     }

  }
  
  ///< Returns the matrix element with the index "indx"
  T1 get(int i){ return M[i]; }

  ///< Returns the matrix element accessed by its row and coloumn indices
  T1 get(int i, int j){  return M[i*n_cols+j];  }


 
  ///========= Initialization =====================
  void diag(int dim, T1 x){ 
  /** initialize dim x dim matrix to be x * I, where I - identity matrix
  if the actual dimension of the matrix is larger than the dim parameters, 
  only the first dim diagonal elements will be set to the provided value
  If the matrix is not square, only the minimal diagonal will be set, 
  only first dim elements of it, that is.
  */


    for(int i=0;i<n_elts;i++){  M[i] = (T1)0.0;   }

    // Finding the minimal dimension
    int min_dim = n_rows;
    if(n_cols <= min_dim){ min_dim = n_cols; }
    if(dim < min_dim){ min_dim = dim; }

    for(int i=0; i<min_dim; i++){  M[i*n_cols+i] = x;   }
  }

  ///< Set the matrix to be the diagonal matrix with the diagonal elements set to x
  void diag(T1 x){   diag(n_rows, x);  }

  ///< Set the matrix to be the diagonal matrix with the diagonal elements set to 1
  void identity(){    diag(n_rows, (T1)1.0);  }


  /// Backward-compatibility functions 

  ///< Set all matrix elements to x
  void Init(T1 x){  set(-1, x); }

  void InitSquareMatrix(int dim, T1 x){ 
  /** This function can resize the matrix to be 
  an dim x dim matrix and filled with x
  */

    // Deallocate previous memory
    if(M!=NULL){ delete [] M; }

    n_rows = dim;
    n_cols = dim;
    n_elts = n_rows * n_cols;

    M = new T1[n_elts];
    for(int i=0;i<n_elts;i++){ M[i] = x;  }
 
  }

  ///< Set the matrix to be the diagonal matrix with the diagonal elements set to x
  bool Init_Unit_Matrix(T1 x){ diag(n_rows, x);  return true; }


 

  ///========= Operations =====================
  ///< Increment the "row", "col" matrix element by "x"
  void add(int row,int col, T1 x){ M[row*n_cols + col] += (T1)x; }  
//  void add(int row,int col, int x){ M[row*n_cols + col] += x; }  

  ///< Increment the "row", "col" matrix element by "x"
//  void add(int row,int col, double x){ M[row*n_cols + col] += x; }  

  ///< Scale (multiply)  the "row", "col" matrix element by "x"
  void scale(int row,int col, T1 x){ M[row*n_cols + col] *= (T1)x; }  
//  void scale(int row,int col, int x){ M[row*n_cols + col] *= x; }  

  ///< Scale (multiply) the "row", "col" matrix element by "x"
//  void scale(int row,int col, double x){ M[row*n_cols + col] *= x; }  


  void product(const base_matrix<T1>& B,const base_matrix<T1>& C){
  /** Compute a product of the input matrices and store the
  result in the calling matrix:  A = B * C  , where A is *this
  This function does not allocate new memory, so the memory in the calling matrix
  must be pre-allocated
  If the dimensions of the operands B and C do not match or if the target matrix
  has inconsistent dimensions - produce the error message and exits
  */

    if(B.n_cols!=C.n_rows){ 
      std::cout<<"Matrix multiplication error: Dimensions of operands must match\n";
      std::cout<<"You try to muplitpy a "<<B.n_rows<<" by "<<B.n_cols<<" matrix and a "
               <<C.n_rows<<" by "<<C.n_cols<<" matrix\n";
      std::cout<<"Exiting...\n";
      exit(0);  
    }

    if(n_rows!=B.n_rows){ 
      std::cout<<"The number of rows of the target matrix ("<<n_rows
      <<") doesn't match the number of rows of the first multiplier matrix ("<<B.n_rows
      <<")\n";
      std::cout<<"Exiting...\n";
      exit(0);  
    }
    if(n_cols!=C.n_cols){ 
      std::cout<<"The number of cols of the target matrix ("<<n_cols
      <<") doesn't match the number of cols of the second multiplier matrix ("<<C.n_cols
      <<")\n";
      std::cout<<"Exiting...\n";
      exit(0);  
    }


    for(int i=0;i<n_elts;i++){  M[i] = (T1)0.0;   }

    for(int row=0; row<n_rows; row++){
      for(int col=0; col<n_cols; col++){

        int indx = row*n_cols + col;
        for(int k=0; k<B.n_cols; k++){  M[indx] += B.M[row*B.n_cols + k] * C.M[k*C.n_cols + col];    }

      }// for col
    }// for row

  }// product


  void dot_product(const base_matrix<T1>& ob1,const base_matrix<T1>& ob2){
  /** Direct product of two matrices - element-wise multiplication
  Dimensions of ob1 and ob2 must be equal - that is both the number of rows
  and the number of columns in the two matrices must match.
  */

    if(ob1.n_cols!=ob2.n_cols){
      std::cout<<"Error in direct CMATRIX multiplication: Number of columns of multiplying matrices must be equal\n";
      std::cout<<"Exiting...\n";
      exit(0);     
    }
  
    if(ob1.n_rows!=ob2.n_rows){
      std::cout<<"Error in direct CMATRIX multiplication: Number of rows of multiplying matrices must be equal\n";
      std::cout<<"Exiting...\n";
      exit(0);     
    }
  
    if(n_cols!=ob2.n_cols){
      std::cout<<"Error in direct CMATRIX multiplication: Number of columns of the multiplying CMATRIX and target CMATRIX must be equal\n";
      std::cout<<"Exiting...\n";
      exit(0);     
    }
  
    if(n_rows!=ob2.n_rows){
      std::cout<<"Error in direct CMATRIX multiplication: Number of rows of the multiplying CMATRIX and target CMATRIX must be equal\n";
      std::cout<<"Exiting...\n";
      exit(0);     
    }
  
  
    for(int row=0;row<n_rows;row++){
      for(int col=0;col<n_cols;col++){
        M[row*n_cols+col] = ob1.M[row*n_cols+col] * ob2.M[row*n_cols+col];
      }
    }

  }

  ///================ Matrix operations ======================

  ///< Transpose the caller matrix
  void Transpose(){
    
    T1 *Temp; Temp = new T1[n_elts];

    for(int row=0; row<n_rows; row++){
      for(int col=0; col<n_cols; col++){
        Temp[col*n_rows+row] = M[row*n_cols+col];
      }
    }

    for(int k=0; k<n_elts; k++){  M[k] = Temp[k];  }

    delete [] Temp;

    // Swap the numbers
    int n = n_cols;    
    n_cols = n_rows;
    n_rows = n;

  }



  void swap_cols(int I, int J){ ///< Swaps two columns

    if(I>=n_cols){
      cout<<"Error in base_matrix<T>::swap_cols\n";
      cout<<"The index of one column ("<<I<<") is larger than the number of columns ("<<n_cols<<")\n";
      exit(0);
    }
    if(J>=n_cols){
      cout<<"Error in base_matrix<T>::swap_cols\n";
      cout<<"The index of one column ("<<J<<") is larger than the number of columns ("<<n_cols<<")\n";
      exit(0);
    }

    for(int row=0;row<n_rows;row++){ 
      T1 tmp = M[row*n_cols+I];
      M[row*n_cols+I] = M[row*n_cols+J];
      M[row*n_cols+J] = tmp;
    }
  }

  void swap_rows(int I, int J){ ///< Swaps two rows

    if(I>=n_rows){
      cout<<"Error in base_matrix<T>::swap_rows\n";
      cout<<"The index of one row ("<<I<<") is larger than the number of rows ("<<n_rows<<")\n";
      exit(0);
    }
    if(J>=n_rows){
      cout<<"Error in base_matrix<T>::swap_rows\n";
      cout<<"The index of one row ("<<J<<") is larger than the number of row ("<<n_rows<<")\n";
      exit(0);
    }

    for(int col=0;col<n_cols;col++){ 
      T1 tmp = M[I*n_cols+col];
      M[I*n_cols+col] = M[J*n_cols+col];
      M[J*n_cols+col] = tmp;
    }
  }


  void RightRotation(int i,int j, T1 sine, T1 cosine){
    T1 a_row_i,a_row_j, A_row_i,A_row_j;
    int k_i,k_j;

    for(int row=0;row<n_rows;row++){

      k_i = row * n_cols+i;
      k_j = row * n_cols+j;

      a_row_i = M[k_i];
      a_row_j = M[k_j];

      A_row_i = a_row_i*cosine + a_row_j*sine;
      A_row_j =-a_row_i*sine   + a_row_j*cosine;

      M[k_i] = A_row_i;
      M[k_j] = A_row_j;
    }
  }

  void LeftRotation(int i,int j, T1 sine, T1 cosine){
    T1 a_i_col,a_j_col, A_i_col,A_j_col;
    int k_i,k_j;

    for(int col=0;col<n_cols;col++){

      k_i = i * n_cols + col;
      k_j = j * n_cols + col;

      a_i_col = M[k_i];
      a_j_col = M[k_j];

      A_i_col = a_i_col*cosine + a_j_col*sine;
      A_j_col =-a_i_col*sine   + a_j_col*cosine;

      M[k_i] = A_i_col;
      M[k_j] = A_j_col;
    }
  }





  ///================ Matrix properties =====================

  T1 tr(){          ///< Compute the trace of the matrix
    T1 res = 0.0;
    int min_dim = n_cols;
    if(n_rows < min_dim){ min_dim = n_rows; }
    for(int i=0;i<min_dim; i++){   res += M[i*n_cols+i];   }
    return res;
  }

  T1 sum(){          ///< Compute the sum of all matrix elements
    T1 res = 0.0;
    for(int i=0;i<n_elts; i++){   res += M[i];   }
    return res;
  }





  ///=================== Matrix IO and preparation ===================
  ///< Binary output
  void bin_dump(std::string filename){
    std::ofstream f(filename.c_str(), ios::out|ios::binary);

    if(f.is_open()){
      f.seekp(0);
      f.write((char*)M, sizeof(T1)*n_elts);
      f.close();    
    }
    else{  cout<<"File "<<filename<<" cann't be open\n"; exit(0); }
  }
 
  ///< Read the matrix from a binary file
  void bin_load(std::string filename){
    std::ifstream f(filename.c_str(), ios::in|ios::binary);

    if(f.is_open()){
      f.seekg(0);
      f.read((char *)M, sizeof(T1)*n_elts);
      f.close();   
    }
    else{  cout<<"File "<<filename<<" cann't be open\n"; exit(0); }

  }

  friend ostream& operator<<(ostream &strm, base_matrix<T1> ob){
    strm.setf(ios::showpoint);
    for(int i=0;i<ob.n_rows;i++){
      for(int j=0;j<ob.n_cols;j++){

        strm.precision(8);
        strm.width(10);
        strm<<left;//right;
        strm<<ob.M[i*ob.n_cols+j]<<"  ";

      }// for j
      strm<<endl;
    }// for i
    return strm;
  }


  ///< Print the matrix out in a formatted way
  void show_matrix(){    std::cout<<*this;   } 

  void show_matrix(char * Output_File){
  /** This function prints the matrix elements into a file
  in a text format
  */
    ofstream ob;  ob.open(Output_File);
    if(ob.is_open()){ ob<<*this; ob.close(); }
    else{ cout<<"Error: can't open file\n"; exit(0); }
    
  }

  bool Load_Matrix_From_File(char *FileName){
  /** Read n_elts numbers from a given file. It doesn't really matter how
  the input values are shaped in the input file. The pre-defined matrix dimensions
  are responsible for the meaning of the input values. The matrix must be
  already created - this function doesn't allocate new memory.
  */

    bool res;
    n_elts = n_rows * n_cols;
    ifstream ob;

    ob.open(FileName);
    if(ob.is_open()){
      for(int k=0;k<n_elts;k++){   ob >> M[k];  }
      ob.close();
      res=true;
    }
    else{ res=false; }

    return res;
  }




  ///============= Operators overload ===============
  /// A good guide: http://stackoverflow.com/questions/4421706/operator-overloading/4421719#4421719


  ///< Assignment = copying (by value)
  void operator=(const base_matrix<T1>& ob){

    if(this == &ob){  return *this;    }
    else{
      n_rows = ob.n_rows;
      n_cols = ob.n_cols;
      n_elts = ob.n_elts;

      memcpy(M, ob.M, sizeof(T1)*n_elts);    
    }
  }

  ///< Assignment of a scalar 
  void operator=(int f){
    for(int i=0;i<n_elts;i++){ M[i] = (T1)f;  }
  }

  void operator=(double f){
    for(int i=0;i<n_elts;i++){ M[i] = (T1)f;  }
  }

  void operator=(complex<double> f){
    for(int i=0;i<n_elts;i++){ M[i] = (T1)f;  }
  }






  ///< Increment by a matrix
  void operator+=(const base_matrix<T1>& ob){
    for(int i=0;i<n_elts;i++) { M[i] += ob.M[i]; }
  }

  ///< Increment by a numeric type
  void operator+=(int f){  
    for(int i=0;i<n_elts;i++) { M[i] += (T1)f; }
  }

  void operator+=(double f){  
    for(int i=0;i<n_elts;i++) { M[i] += (T1)f; }
  }





  ///< Decrement by a matrix type
  void operator-=(const base_matrix<T1>& ob){
    for(int i=0;i<n_elts;i++) { M[i] -= ob.M[i]; }
  }

  ///< Decrement by a numeric type
  void operator-=(int f){  
    for(int i=0;i<n_elts;i++) { M[i] -= (T1)f; }
  }

  void operator-=(double f){  
    for(int i=0;i<n_elts;i++) { M[i] -= (T1)f; }
  }



  ///< Multiplicative scaling scaling by a scalar
  void operator*=(int f){ 
    for(int i=0; i<n_elts; i++){  M[i] *= (T1)f;  }
  }

  void operator*=(double f){ 
    for(int i=0; i<n_elts; i++){  M[i] *= (T1)f;  }
  }




  ///< Divisional scaling by a scalar
  void operator/=(int f){ 
    for(int i=0; i<n_elts; i++){  M[i] = M[i] / (T1)f;  }
  }

  void operator/=(double f){ 
    for(int i=0; i<n_elts; i++){  M[i] = M[i] / (T1)f;  }
  }






  ///< Comparison: true if matrices are not equal to each other
  friend int operator !=(const base_matrix<T1>& m1, const base_matrix<T1>& m2){
    if(m1.n_cols != m2.n_cols){ return 1; } 
    if(m1.n_rows != m2.n_rows){ return 1; } 
    
    for(int i=0;i<m1.n_elts;i++){
      if(m1.M[i]!=m2.M[i]) { return 1; }
    }
    return 0;
  }

  ///< Comparison: true if matrices are equal to each other
  friend int operator ==(const base_matrix<T1>& m1, const base_matrix<T1>& m2){
    return !(m1!=m2);
  }




};



template <typename T1> 
void pop_submatrix(base_matrix<T1>* X, base_matrix<T1>* x, vector<int>& subset){
/**
  Take a smaller sub-matrix from a bigger matrix according to the specified pattern

  Extract the submatrix x from the matrix X according to indices given in <subset>
  Assume that memory for x is already allocated and its dimensions are consistent with the map dimensions:
  subset_size == x->num_of_cols = x->num_of_rows = subset.size()
  X->num_of_cols = X->num_of_rows >= subset_size
*/

  if(X->n_cols!=X->n_rows){ cout<<"Error in pop_submatrix: The source matrix, X, is not square!\nExiting...\n"; exit(0); }
  if(x->n_cols!=x->n_rows){ cout<<"Error in pop_submatrix: The target matrix, x, is not square!\nExiting...\n"; exit(0); }

  int N = X->n_cols;
  int n = x->n_cols;

  if(N<n){ cout<<"Error in pop_submatrix: The size of the source matrix, X, is smaller than that of the target matrix, x!\nExiting...\n"; exit(0); }
  if(n!=subset.size()){
    cout<<"Error in pop_submatrix: the target matrix size ("<<n<<") is not consistent with the stensil size (";
    cout<<subset.size()<<")!\nExiting...\n"; exit(0); 
  }


  int i,j,a,b;

  for(i=0;i<n;i++){
    a = subset[i];
    for(j=0;j<n;j++){      
      b = subset[j];
      x->M[i*n+j] = X->M[a*N+b];
    }// j
  }// i

}// 

template <typename T1> 
void pop_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, vector<int>& subset){
  ///< Take a smaller sub-matrix from a bigger matrix according to the specified pattern
  pop_submatrix(&X, &x, subset); 
}


template <typename T1> 
void pop_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, boost::python::list subset){ 
  ///< Take a smaller sub-matrix from a bigger matrix according to the specified pattern

  // Convert input list to vector
  int sz = boost::python::len(subset);
  vector<int> _subset(sz,0.0);
  for(int i=0;i<sz;i++){ _subset[i] = boost::python::extract<int>(subset[i]);  }

  pop_submatrix(&X, &x, _subset);

}

template <typename T1> 
void pop_submatrix(base_matrix<T1>* X, base_matrix<T1>* x, vector<int>& subset, vector<int>& subset2){
/**
  Take a smaller sub-matrix from a bigger matrix according to the specified pattern

  Extract the submatrix x from the matrix X according to indices given in <subset> (for rows) and <subset2> (for cols)
  Assume that memory for x is already allocated and its dimensions are consistent with the map dimensions:
  subset_size == x->num_of_rows
  subset2_size == x->num_of_cols = 

  X->num_of_rows >= subset_size
  X->num_of_cols >= subset2_size
*/

  if(X->n_cols < x->n_cols){
    cout<<"Error in pop_submatrix: The # of cols of the source matrix, X, is smaller than that of the target matrix, x!\nExiting...\n"; 
    exit(0); 
  }
  if(X->n_rows < x->n_rows){
    cout<<"Error in pop_submatrix: The # of rows of the source matrix, X, is smaller than that of the target matrix, x!\nExiting...\n"; 
    exit(0); 
  }
  if(x->n_rows != subset.size()){
    cout<<"Error in pop_submatrix: # of rows in the target matrix ("<<x->n_rows<<") is not consistent with the stensil size (";
    cout<<subset.size()<<")!\nExiting...\n"; exit(0); 
  }
  if(x->n_cols != subset2.size()){
    cout<<"Error in pop_submatrix: # of cols in the target matrix ("<<x->n_cols<<") is not consistent with the stensil size (";
    cout<<subset2.size()<<")!\nExiting...\n"; exit(0); 
  }

  int i,j,a,b;

  for(i=0;i<x->n_rows;i++){
    a = subset[i];
    for(j=0;j<x->n_cols;j++){      
      b = subset2[j];
      x->M[i*x->n_cols+j] = X->M[a*X->n_cols+b];
    }// j
  }// i

}// 


template <typename T1>
void pop_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, vector<int>& subset, vector<int>& subset2){  
  ///< Take a smaller sub-matrix from a bigger matrix according to the specified pattern
  pop_submatrix(&X, &x, subset, subset2); 
}

template <typename T1> 
void pop_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, boost::python::list subset, boost::python::list subset2){
  ///< Take a smaller sub-matrix from a bigger matrix according to the specified pattern

  // Convert input list to vector
  int sz = boost::python::len(subset);
  vector<int> _subset(sz,0.0);
  for(int i=0;i<sz;i++){ _subset[i] = boost::python::extract<int>(subset[i]);  }

  sz = boost::python::len(subset2);
  vector<int> _subset2(sz,0.0);
  for(int i=0;i<sz;i++){ _subset2[i] = boost::python::extract<int>(subset2[i]);  }

  pop_submatrix(&X, &x, _subset, _subset2);

}



template <typename T1>
void push_submatrix(base_matrix<T1>* X, base_matrix<T1>* x, vector<int>& subset){ 

/**
  Push a smaller matrix into a bigger one according to the specified pattern

  Pushes the smaller submatrix x back to the bigger matrix X, according to indices given in <subset>
  Assume that memory for x is already allocated and its dimensions are consistent with the map dimensions:
  subset_size == x->num_of_cols = x->num_of_rows = subset.size()
  X->num_of_cols = X->num_of_rows >= subset_size
*/

  if(X->n_cols!=X->n_rows){ cout<<"Error in push_submatrix: The target matrix, X, is not square!\nExiting...\n"; exit(0); }
  if(x->n_cols!=x->n_rows){ cout<<"Error in push_submatrix: The source matrix, x, is not square!\nExiting...\n"; exit(0); }

  int N = X->n_cols;
  int n = x->n_cols;

  if(N<n){ cout<<"Error in push_submatrix: The size of the target matrix, X, is smaller than that of the source matrix, x!\nExiting...\n"; exit(0); }
  if(n!=subset.size()){
    cout<<"Error in pop_submatrix: the source matrix size ("<<n<<") is not consistent with the stensil size (";
    cout<<subset.size()<<")!\nExiting...\n"; exit(0); 
  }

  int i,j,a,b;

  for(i=0;i<n;i++){
    a = subset[i];
    for(j=0;j<n;j++){      
      b = subset[j];
      X->M[a*N+b] = x->M[i*n+j];
    }// j
  }// i

}// void push_submatrix(CMATRIX* X,CMATRIX* x,vector<int>& subset)

template <typename T1>
void push_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, vector<int>& subset){
 ///< Push a smaller matrix into a bigger one according to the specified pattern
  push_submatrix(&X, &x, subset); 
}


template <typename T1> 
void push_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, boost::python::list subset){ 
///< Push a smaller matrix into a bigger one according to the specified pattern
 
  // Convert input list to vector
  int sz = boost::python::len(subset);
  vector<int> _subset(sz,0.0);
  for(int i=0;i<sz;i++){ _subset[i] = boost::python::extract<int>(subset[i]);  }

  push_submatrix(&X, &x, _subset);

}


template <typename T1> 
void push_submatrix(base_matrix<T1>* X, base_matrix<T1>* x, vector<int>& subset,vector<int>& subset2){
/**
  Push a smaller matrix into a bigger one according to the specified pattern

  Pushes the smaller submatrix x back to the bigger matrix X, according to indices given in <subset>(for rows) and <subset2> (for cols)
  Assume that memory for x is already allocated and its dimensions are consistent with the map dimensions:
  subset_size == x->num_of_rows
  subset2_size == x->num_of_cols = 

  X->num_of_rows >= subset_size
  X->num_of_cols >= subset2_size
*/

  if(X->n_cols < x->n_cols){
    cout<<"Error in push_submatrix: The # of cols of the target matrix, X, is smaller than that of the source matrix, x!\nExiting...\n"; 
    exit(0); 
  }
  if(X->n_rows < x->n_rows){
    cout<<"Error in push_submatrix: The # of rows of the target matrix, X, is smaller than that of the source matrix, x!\nExiting...\n"; 
    exit(0); 
  }
  if(x->n_rows != subset.size()){
    cout<<"Error in push_submatrix: # of rows in the source matrix ("<<x->n_rows<<") is not consistent with the stensil size (";
    cout<<subset.size()<<")!\nExiting...\n"; exit(0); 
  }
  if(x->n_cols != subset2.size()){
    cout<<"Error in push_submatrix: # of cols in the source matrix ("<<x->n_cols<<") is not consistent with the stensil size (";
    cout<<subset2.size()<<")!\nExiting...\n"; exit(0); 
  }



  int i,j,a,b;

  for(i=0;i<x->n_rows;i++){
    a = subset[i];
    for(j=0;j<x->n_cols;j++){      
      b = subset2[j];
      X->M[a*X->n_cols+b] = x->M[i*x->n_cols+j];
    }// j
  }// i

}// void push_submatrix(CMATRIX* X,CMATRIX* x,vector<int>& subset,vector<int>& subset2)


template <typename T1> 
void push_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, vector<int>& subset,vector<int>& subset2){
  ///< Push a smaller matrix into a bigger one according to the specified pattern
  push_submatrix(&X, &x, subset, subset2); 
}




template <typename T1> 
void push_submatrix(base_matrix<T1>& X, base_matrix<T1>& x, boost::python::list subset,boost::python::list subset2){
  ///< Push a smaller matrix into a bigger one according to the specified pattern

  // Convert input list to vector
  int sz = boost::python::len(subset);
  vector<int> _subset(sz,0.0);
  for(int i=0;i<sz;i++){ _subset[i] = boost::python::extract<int>(subset[i]);  }

  sz = boost::python::len(subset2);
  vector<int> _subset2(sz,0.0);
  for(int i=0;i<sz;i++){ _subset2[i] = boost::python::extract<int>(subset2[i]);  }

  push_submatrix(&X, &x, _subset, _subset2);

}



template <typename T1> 
void set_value(int& is_defined, base_matrix<T1>& value,boost::python::object obj, std::string attrName){

  int has_attr=0;
  has_attr = (int)hasattr(obj,attrName);
  if(has_attr){
    value = extract<base_matrix<T1> >(obj.attr(attrName.c_str()));
   is_defined = 1;
  }
}



// ----------- Save --------------
template <typename T1> 
void save(boost::property_tree::ptree& pt,std::string path, base_matrix<T1>& vt){
  for(int i=0;i<vt.n_elts;i++){
    stringstream ss(stringstream::in | stringstream::out);
    std::string rt; ss<<i; ss>>rt;
    pt.put(path+"."+rt,vt.M[i]);
  }
}

template <typename T1> 
void save(boost::property_tree::ptree& pt,std::string path, char path_separator, base_matrix<T1>& vt){
  for(int i=0;i<vt.n_elts;i++){
    stringstream ss(stringstream::in | stringstream::out);
    std::string rt; ss<<i; ss>>rt;
    pt.put(boost::property_tree::ptree::path_type(path, path_separator),vt.M[i]);
  }
}

template <typename T1> 
void save(boost::property_tree::ptree& pt,std::string path,vector<base_matrix<T1> >& vt){
  int sz = vt.size();
  for(int i=0;i<sz;i++){
    stringstream ss(stringstream::in | stringstream::out);
    std::string rt; ss<<i; ss>>rt;
    save(pt,path+"."+rt,vt[i]);
  }
}

template <typename T1> 
void save(boost::property_tree::ptree& pt,std::string path, char path_separator, vector<base_matrix<T1> >& vt){
  int sz = vt.size();
  for(int i=0;i<sz;i++){
    stringstream ss(stringstream::in | stringstream::out);
    std::string rt; ss<<i; ss>>rt;
    save(pt,path+std::string(1, path_separator)+rt, path_separator,vt[i]);
  }
}


// ----------- Load --------------
template <typename T1> 
void load(boost::property_tree::ptree& pt,std::string path, base_matrix<T1>& vt, int& status){
  std::cout<<"Sorry: the load function for generic matrix object is not defined yet\n"; exit(0); 
}

template <typename T1> 
void load(boost::property_tree::ptree& pt,std::string path, char path_separator, base_matrix<T1>& vt, int& status){
  std::cout<<"Sorry: the load function for generic matrix object is not defined yet\n"; exit(0); 
}

template <typename T1> 
void load(boost::property_tree::ptree& pt,std::string path,vector<base_matrix<T1> >& vt,int& status){
  std::cout<<"Sorry: the load function for vector of generic matrices is not defined yet\n"; exit(0); 
}

template <typename T1> 
void load(boost::property_tree::ptree& pt,std::string path, char path_separator,vector<base_matrix<T1> >& vt,int& status){
  std::cout<<"Sorry: the load function for vector of generic matrices is not defined yet\n"; exit(0); 
}




}//namespace liblinalg
}// liblibra

#endif // CMATRIX_H
