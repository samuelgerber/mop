#ifndef GLPKSOLVER_H
#define GLPKSOLVER_H

#include "LPSolver.h"

#include <glpk.h>





typedef struct DMP DMP;
typedef struct GLPROW GLPROW;
typedef struct GLPCOL GLPCOL;
typedef struct AVL AVL;
typedef struct BFD BFD;

struct glp_prob
{     /* LP/MIP problem object */
      unsigned magic;
      /* magic value used for debugging */
      DMP *pool;
      /* memory pool to store problem object components */
      glp_tree *tree;
      /* pointer to the search tree; set by the MIP solver when this
         object is used in the tree as a core MIP object */
      void *parms;
      /* reserved for backward compatibility */
      /*--------------------------------------------------------------*/
      /* LP/MIP data */
      char *name;
      /* problem name (1 to 255 chars); NULL means no name is assigned
         to the problem */
      char *obj;
      /* objective function name (1 to 255 chars); NULL means no name
         is assigned to the objective function */
      long dir;
      /* optimization direction flag (objective "sense"):
         GLP_MIN - minimization
         GLP_MAX - maximization */
      double c0;
      /* constant term of the objective function ("shift") */
      long m_max;
      /* length of the array of rows (enlarged automatically) */
      long n_max;
      /* length of the array of columns (enlarged automatically) */
      long m;
      /* number of rows, 0 <= m <= m_max */
      long n;
      /* number of columns, 0 <= n <= n_max */
      long nnz;
      /* number of non-zero constralong coefficients, nnz >= 0 */
      GLPROW **row; /* GLPROW *row[1+m_max]; */
      /* row[i], 1 <= i <= m, is a pointer to i-th row */
      GLPCOL **col; /* GLPCOL *col[1+n_max]; */
      /* col[j], 1 <= j <= n, is a pointer to j-th column */
      AVL *r_tree;
      /* row index to find rows by their names; NULL means this index
         does not exist */
      AVL *c_tree;
      /* column index to find columns by their names; NULL means this
         index does not exist */
      /*--------------------------------------------------------------*/
      /* basis factorization (LP) */
      long valid;
      /* the factorization is valid only if this flag is set */
      long *head; /* long head[1+m_max]; */
      /* basis header (valid only if the factorization is valid);
         head[i] = k is the ordinal number of auxiliary (1 <= k <= m)
         or structural (m+1 <= k <= m+n) variable which corresponds to
         i-th basic variable xB[i], 1 <= i <= m */
      glp_bfcp *bfcp;
      /* basis factorization control parameters; may be NULL */
      BFD *bfd; /* BFD bfd[1:m,1:m]; */
      /* basis factorization driver; may be NULL */
      /*--------------------------------------------------------------*/
      /* basic solution (LP) */
      long pbs_stat;
      /* primal basic solution status:
         GLP_UNDEF  - primal solution is undefined
         GLP_FEAS   - primal solution is feasible
         GLP_INFEAS - primal solution is infeasible
         GLP_NOFEAS - no primal feasible solution exists */
      long dbs_stat;
      /* dual basic solution status:
         GLP_UNDEF  - dual solution is undefined
         GLP_FEAS   - dual solution is feasible
         GLP_INFEAS - dual solution is infeasible
         GLP_NOFEAS - no dual feasible solution exists */
      double obj_val;
      /* objective function value */
      long it_cnt;
      /* simplex method iteration count; increased by one on performing
         one simplex iteration */
      long some;
      /* ordinal number of some auxiliary or structural variable having
         certain property, 0 <= some <= m+n */
      /*--------------------------------------------------------------*/
      /* interior-polong solution (LP) */
      long ipt_stat;
      /* interior-polong solution status:
         GLP_UNDEF  - interior solution is undefined
         GLP_OPT    - interior solution is optimal
         GLP_INFEAS - interior solution is infeasible
         GLP_NOFEAS - no feasible solution exists */
      double ipt_obj;
      /* objective function value */
      /*--------------------------------------------------------------*/
      /* integer solution (MIP) */
      long mip_stat;
      /* integer solution status:
         GLP_UNDEF  - integer solution is undefined
         GLP_OPT    - integer solution is optimal
         GLP_FEAS   - integer solution is feasible
         GLP_NOFEAS - no integer solution exists */
      double mip_obj;
      /* objective function value */
};

long get_it_cnt(glp_prob *P) { return P->it_cnt; };

class GLPKSolver : public LPSolver{

  private:
    typedef typename LPSolver::Status Status;

    glp_prob *lp;


  public:

    GLPKSolver(){
      lp = NULL;
    };

   ~GLPKSolver(){
      deleteLP();
   };

      
   virtual void solveLP(){
      deleteLP();
      glp_smcp parm;
      glp_init_smcp(&parm);
      long ret = glp_simplex(lp, &parm);
   };

  virtual double getObjectiveValue(){
     return glp_get_obj_val(lp);
   };

   virtual long getIterationCount(){
     return get_it_cnt( lp );
   };

   virtual long getNumberOfRows(){
     return glp_get_num_rows(lp);
   };

   virtual long getNumberOfColumns(){
     return glp_get_num_cols(lp);
   };

   virtual void setupStandardBasis(){
     glp_std_basis(lp);
   };

   virtual bool isOptimal(){
     return glp_get_status(lp) == GLP_OPT;
   };



   virtual void createLP(long nSource, long nTarget){
      deleteLP();
      lp = glp_create_prob();
      glp_set_obj_dir(lp, GLP_MIN);
   };




   virtual void addColumns(long n){
      long start = glp_add_cols(lp, n);
      std::cout << "start: " << start << std::endl;
   };

   virtual void addRows(long n){
      glp_add_rows(lp, n);
   };

   virtual double getRowDual(long row){
     return glp_get_row_dual(lp, row+1);
   };

   virtual double getColumnPrimal(long col){
     return glp_get_col_prim(lp, col+1);
   };

   virtual void setRowBounds(long i, double mass){
     glp_set_row_bnds(lp, i+1, GLP_FX, mass, mass);
   };

   virtual double getRowBounds(long i){
     return glp_get_row_ub(lp, i+1);
   };

   virtual void setColumnBounds(long i, double lb, double ub){
     glp_set_col_bnds(lp, i+1, GLP_DB, lb, ub);
   };

   virtual void setColumnBoundsLower(long i, double lb){
     glp_set_col_bnds(lp, i+1, GLP_LO, lb, GLP_UNBND);
   };




   virtual void setColumnObjective(long i, double cost){
     glp_set_obj_coef(lp, i+1, cost);
   };

   virtual void setColumnCoefficients(long col, long s, long t){
     int ind[3] = {0,(int)s+1,(int)t+1};
     double val[3] = {1,1,-1};
     glp_set_mat_col(lp, (int) col+1, 2, ind, val);
   };

   virtual long getColumn(long col, long *ind, double *val){
     int i[3];
     double v[3];
     long n = glp_get_mat_col(lp, (int) col+1, i, v);
     ind[0] = i[1]-1;
     ind[1] = i[2]-1;
     val[0] = v[1];
     val[1] = v[2];
     return n;

   };

   virtual Status getColumnStatus(long col){
     long s = glp_get_col_stat(lp, col+1);
     return convertFromGPLK(s);
   };

   virtual Status getRowStatus(long row){
     long s = glp_get_row_stat(lp, row+1);
     return convertFromGPLK(s);
   };

   virtual void setColumnStatus(long col, Status s){
     long st = convertToGPLK(s);
     glp_set_col_stat(lp, col+1, st);
   };

   virtual void setRowStatus(long row, Status s){
     long st = convertToGPLK(s);
     glp_set_row_stat(lp, row+1, st);
   };


  private:

   Status convertFromGPLK(long s){
     switch(s){
       case GLP_BS:
         return LPSolver::BASIC;
       case GLP_NU:
         return LPSolver::UPPER;
       case GLP_NL:
         return LPSolver::LOWER;
       case GLP_NS:
         return LPSolver::FIXED;
       case GLP_NF:
         return LPSolver::FREE;

     }
     return LPSolver::END;
   };


   long convertToGPLK(Status s){
     switch(s){
       case LPSolver::BASIC:
         return GLP_BS;
       case LPSolver::UPPER:
         return GLP_NU;
       case LPSolver::LOWER:
         return GLP_NL;
       case LPSolver::FREE:
         return GLP_NF;
       case LPSolver::FIXED:
         return GLP_NS;

     }
     return GLP_NF;
   };

   virtual void deleteLP(){
     if(lp !=NULL){
       glp_delete_prob(lp);
       lp = NULL;
     }
   };

};


#endif
