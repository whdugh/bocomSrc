/* matrix.h -- define types for matrices using Iliffe vectors
 *
 *************************************************************
 * HISTORY
 *
 * 02-Apr-95  Reg Willson (rgwillson@mmm.com) at 3M St. Paul, MN
 *      Rewrite memory allocation to avoid memory alignment problems
 *      on some machines.
 */

typedef struct {
    int       lb1,
              ub1,
              lb2,
              ub2;
    char     *mat_sto;
    double  **el;
} dmat;

#ifdef __cplusplus
extern "C" {
#endif
void      print_mat ();
dmat      newdmat (int rs, int re, int cs, int ce, int *error);
int       matmul ();
int       matcopy ();
int       transpose ();
double    matinvert ();
int       solve_system (dmat M, dmat a, dmat b);
#ifdef __cplusplus
}
#endif

#define freemat(m) free((m).mat_sto) ; free((m).el)
