/* vim: set expandtab shiftwidth=2 softtabstop=2 tw=70: */

#include <Rcpp.h>
using namespace Rcpp;

// Cross-reference work:
// 1. update ../src/registerDynamicSymbol.c with an item for this
// 2. main code should use the autogenerated wrapper in ../R/RcppExports.R
//
// [[Rcpp::export]]
NumericMatrix do_ad2cp_ahrs(NumericMatrix v, NumericMatrix ahrs)
{
    if (ahrs.ncol() != 9)
      Rf_error("ncol(ahrs) must be 9, but it is %d", ahrs.ncol());
    unsigned int nrow = v.nrow();
    unsigned int ncol = v.ncol();
    if (ncol != 3)
      Rf_error("ncol(v) must be 3, but it is %d", ncol);
    if (ahrs.nrow() != nrow)
      Rf_error("nrow(v) and nrow(ahrs) must agree, but they are %d and %d", nrow, ahrs.nrow());
    NumericMatrix enu(nrow, ncol);
    for (unsigned int i = 0; i < nrow; i++) {
      // e <- V[,,1]*rep(AHRS[,1], each=nc) + V[,,2]*rep(AHRS[,2], each=nc) + V[,,3]*rep(AHRS[,3], each=nc)
      enu(i, 0) = v(i, 0)*ahrs(i, 0) + v(i, 1)*ahrs(i, 1) + v(i, 2)*ahrs(i, 2);
      enu(i, 1) = v(i, 0)*ahrs(i, 3) + v(i, 1)*ahrs(i, 4) + v(i, 2)*ahrs(i, 5);
      enu(i, 2) = v(i, 0)*ahrs(i, 6) + v(i, 1)*ahrs(i, 7) + v(i, 2)*ahrs(i, 8);
      //if (i < 4)
      //  Rprintf("i=%d ahrs=%12.9f %12.9f %12.9f %12.9f %12.9f %12.9f %12.9f %12.9f %12.9f\n    v=%12.9f %12.9f %12.9f\n",
      //      i, 
      //      ahrs(i,0), ahrs(i,1), ahrs(i,2),
      //      ahrs(i,3), ahrs(i,4), ahrs(i,5),
      //      ahrs(i,6), ahrs(i,7), ahrs(i,8),
      //      v(i,0), v(i,1), v(i,2));
    }
    return(enu);
}
