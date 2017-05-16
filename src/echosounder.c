/* vim: set expandtab shiftwidth=2 softtabstop=2 tw=70: */
//
// FIXME: this code should be altered to handle dual- and split-beam data.
// In single-beam data, the procedure is to work in 2-byte chunks (as
// is done here) but for the other cases, it should work in 4-byte
// chunks.

// REFERENCES:
//   [1] "DT4 Data File Format Specification" [July, 2010] DT4_format_2010.pdf

//#define DEBUG 1
#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>
    
// 1. Routines to maintain static storage to avoid possibly thousands
// of allocations of identical size, for a fixed purpose.
// FIXME: see if OK on multicore; if not, use R method (if that's ok).
static unsigned char *buffer = NULL; // single-beam uses just first 2 bytes of each 4-byte entry
void biosonics_allocate_storage(int spp, int byte_per_sample)
{
  if (buffer == NULL) {
#ifdef DEBUG
    Rprintf("should allocate space for %d data now\n", spp * byte_per_sample);
#endif
    buffer = (unsigned char*)calloc(spp * byte_per_sample, sizeof(unsigned char));
    if (buffer == NULL) {
      error("cannot allocate space for temporary buffer, of length %d bytes", spp * byte_per_sample);
    }
#ifdef DEBUG
    Rprintf("... allocation was OK; address is %lx\n", buffer);
#endif
  }
}

void biosonics_free_storage()
{
#ifdef DEBUG
    Rprintf("freeing up 'buffer' storage at address %lx\n", buffer);
#endif
  if (buffer != NULL)
    free(buffer);
  buffer = NULL;
}

// 2. run-length expansion
void rle(unsigned char *samp, int ns, int spp, int byte_per_sample)
{
  // This code is patterned on [1 p36-37] for runlength expansion
  // of data stored in 4-byte chunks.  The main difference is 
  // that the present function uses bytewise operations, which
  // means that it should work the same on big-endian
  // and little-endian computers.  After this function returns,
  // the global-storage 'buffer' contains runlength expanded
  // data taken from the 'samp', and the length of that buffer
  // is 4*spp bytes.
#ifdef DEBUG
  Rprintf("rle(0x%02x%02x%02x%02x ..., ns=%d, spp=%d, byte_per_sample=%d)\n",
      samp[0], samp[1], samp[2], samp[3], ns, spp, byte_per_sample);
#endif
  int i = 0, k = 0;
  unsigned char b1, b2, b3, b4;
  int NS = ns * byte_per_sample;
  int SPP = spp * byte_per_sample;
  while (i < NS) {
    //Rprintf("i=%d k=%d\n", i, k);
    b1 = samp[i++];
    b2 = samp[i++];
    if (byte_per_sample == 4) {
      b3 = samp[i++];
      b4 = samp[i++];
    }
    if (b2 == 0xFF) {
      int n = (int)b1 + 2;
#ifdef DEBUG
      Rprintf("zero-fill %d samples at i=%d k=%d\n", n, i, k);
#endif
      while (n > 0) {
        if (k < SPP) {
          buffer[k++] = 0x00;
          buffer[k++] = 0x00;
          if (byte_per_sample == 4) {
            buffer[k++] = 0x00;
            buffer[k++] = 0x00;
          }
          --n;
        } else { 
          break; // prevent overfill (probably will never happen)
        }
      }
    } else {
      if (k < SPP) {
        buffer[k++] = b1;
        buffer[k++] = b2;
        if (byte_per_sample == 4) {
          buffer[k++] = b3;
          buffer[k++] = b4;
        }
      } else {
        break;
      }
    }
  }
#ifdef DEBUG
  Rprintf("zero-fill at end for %d elements\n", (SPP-k)/4);
#endif
  while (k < SPP) {
    buffer[k++] = 0x00;
    buffer[k++] = 0x00;
    if (byte_per_sample == 4) {
      buffer[k++] = 0x00;
      buffer[k++] = 0x00;
    }
  }
}


// 3. subsecond time for Biosonic echosounder
// [1 p19]
void biosonics_ss(unsigned char *byte, double *out)
{
  if (!(0x80 & *byte))
    *out = 0.0;
  else
    *out = (float)((int)(0x7F & *byte)) / 100;
}

// 4. decode biosonics two-byte floating format
double biosonic_float(unsigned char byte1, unsigned char byte2)
{
    unsigned int assembled_bytes = ((short)byte2 << 8) | ((short)byte1); // little endian
    unsigned int mantissa = (assembled_bytes & 0x0FFF); // rightmost 12 bits (again, little endian)
    int exponent = (assembled_bytes & 0xF000) >> 12; // leftmost 4 bits, shifted to RHS
    unsigned long res;
    if (exponent == 0) {
      res = mantissa;
    } else {
      res = (mantissa + 0x1000) << (exponent - 1);
    }
//#ifdef DEBUG
//    Rprintf(" ***  0x%x%x mantissa=%d exponent=%d res=%f\n", byte1, byte2, mantissa, exponent, resp[i]);
//#endif
    return((double)res);
}

SEXP biosonics_ping(SEXP bytes, SEXP Rspp, SEXP Rns, SEXP Rtype)
{
  PROTECT(bytes = AS_RAW(bytes));
  PROTECT(Rspp = AS_NUMERIC(Rspp));
  int spp = (int)floor(0.5 + *REAL(Rspp));
  PROTECT(Rns = AS_NUMERIC(Rns));
  int ns = (int)floor(0.5 + *REAL(Rns));
  PROTECT(Rtype = AS_NUMERIC(Rtype));
  int type = (int)floor(0.5 + *REAL(Rtype));
  //double *typep = REAL(type);
  //int beam = (int)floor(0.5 + *typep);
#ifdef DEBUG
  Rprintf("biosonics_ping() decoded type:%d, spp:%d, ns:%d\n", type, spp, ns);
#endif
  int byte_per_sample = 2;
  if (type == 1 || type == 2) {
    byte_per_sample = 4;
  }
  unsigned char *bytep = RAW(bytes);

  SEXP res;
  PROTECT(res = allocVector(VECSXP, 3));
  SEXP res_names;
  PROTECT(res_names = allocVector(STRSXP, 3));
  SEXP res_a;
  PROTECT(res_a = allocVector(REALSXP, spp));
  SEXP res_b;
  PROTECT(res_b = allocVector(REALSXP, spp));
  SEXP res_c;
  PROTECT(res_c = allocVector(REALSXP, spp));
  // Get static storage; FIXME: is this thread-safe?
  biosonics_allocate_storage(spp, byte_per_sample);
#ifdef DEBUG
  Rprintf("allocVector(REALSXP, %d)\n", spp);
#endif
  double *resap = REAL(res_a);
  double *resbp = REAL(res_b);
  double *rescp = REAL(res_c);
  if (type == 0) { // single-beam
    rle(bytep, ns, spp, 2);
    for (int k = 0; k < spp; k++) {
      resap[k] = biosonic_float(buffer[byte_per_sample * k], buffer[1 + byte_per_sample * k]);
      resbp[k] = 0.0;
      rescp[k] = 0.0;
    }
  } else if (type == 1) { // dual-beam
    rle(bytep, ns, spp, 4);
    for (int k = 0; k < spp; k++) {
      // Quote [1 p37 re dual-beam]: "For an RLE-expanded sample x, the low-order
      // word (ie, (USHORT)(x & 0x0000FFFF)) contains the narrow-beam data. The
      // high-order word (ie, (USHORT)((x & 0xFFFF0000) >> 16)) contains the
      // wide beam data."
      resap[k] = biosonic_float(buffer[    byte_per_sample * k], buffer[1 + byte_per_sample * k]);
      resbp[k] = biosonic_float(buffer[2 + byte_per_sample * k], buffer[3 + byte_per_sample * k]);
      resbp[k] = 0.0;
    }
  } else if (type == 2) { // split-beam
    rle(bytep, ns, spp, 4);
    for (int k = 0; k < spp; k++) {
      // Quote [1 p38 split-beam e.g. 01-Fish.dt4 example]: "the low-order word
      // (ie, (USHORT)(x & 0x0000FFFF)) contains the amplitude data. The
      // high-order byte (ie, (TINY)((x & 0xFF000000) >> 24)) contains the
      // raw X-axis angle data. The other byte
      // (ie, (TINY)((x & 0x00FF0000) >> 16)) contains the raw Y-axis angle data.
      resap[k] = biosonic_float(buffer[byte_per_sample * k], buffer[1 + byte_per_sample * k]);
      resbp[k] = (double)buffer[2 + byte_per_sample * k];
      rescp[k] = (double)buffer[3 + byte_per_sample * k];
    }
  } else {
    error("unknown type, %d", type);
  }
  SET_VECTOR_ELT(res, 0, res_a);
  SET_VECTOR_ELT(res, 1, res_b);
  SET_VECTOR_ELT(res, 2, res_c);
  SET_STRING_ELT(res_names, 0, mkChar("a"));
  SET_STRING_ELT(res_names, 1, mkChar("b"));
  SET_STRING_ELT(res_names, 2, mkChar("c"));
  setAttrib(res, R_NamesSymbol, res_names);
  UNPROTECT(9);
  return(res);
}


