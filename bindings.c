#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "fec.h"
#include "char.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#define NUM_SYMBOLS 20
#define NUM_DATA 8
#define NUM_PARITY (NUM_SYMBOLS - NUM_DATA)

#define BITS_PER_SYMBOL 5
#define GF_POLY_DEFAULT 0x25
#define FCR_DEFAULT 19
#define PRIM_DEFAULT 13
#define PAD_DEFAULT ((1<<BITS_PER_SYMBOL) -1 - NUM_SYMBOLS)

static void * rs;


/*
 *      See example.js for JavaScript usage.
 *
 *    It assumes 20 ARUCO markers of type DICT_4X4_50, i.e., 5 bits each,
 *  to encode a 40 bit identifier with 60 parity bits, i.e., 8 data markers and
 *  12 parity markers.
 *
 *   If we can provide error location, i.e., erasures, we can recover from 12
 * marker failures.
 *
 *   If we don't know where the errors are, we can recover from 6 marker
 *  failures.
 *
 *  We can also recover from combined errors and erasures as long as:
 *              #erasures + 2*#errors <= 12
 *
 *  and reliably detect up to 12 errors.
 */

EMSCRIPTEN_KEEPALIVE
void cleanup() {
    if (rs) {
        free_rs_char(rs);
        rs = 0;
    }
}

static void init(int symbolSize, int gfpoly, int fcr, int prim, int nroots,
                 int pad) {
    if (rs) {
        cleanup();
    }
    // symbolSize = 5, gfpoly=0x25, fcr= first root, prim= primitive element
    // to generate polynomial roots, nroots= #parity bits, pad= padding bytes
    // at the front of block, i.e., 2^5-1-pad= nroots+ ndata

    rs = init_rs_char(symbolSize, gfpoly, fcr, prim, nroots, pad);
}

EMSCRIPTEN_KEEPALIVE
int encode(unsigned char data[], int size) {
    if (size != NUM_SYMBOLS) {
        return EXIT_FAILURE;
    } else {

        // in python (unireedsolomon-1.0) RS.RSCoder(20, 8, 3, 0x25, 1, 5)
        //   so python generator 3 maps to prim 13 in C
        //    and python fcr 1 maps to C fcr 19 ...
        // found by exhaustive search...

        if (!rs) {
            // symbol size = 5, gfpoly=0x25, fcr= first root, prim= primitive
            // element to generate polynomial roots, nroots= #parity bits,
            // pad= padding bytes at the front of block, i.e.,
            //  2^5-1-pad= nroots+ ndata
            init(BITS_PER_SYMBOL, GF_POLY_DEFAULT, FCR_DEFAULT, PRIM_DEFAULT,
                 NUM_PARITY, PAD_DEFAULT);
        }
        encode_rs_char(rs, data, &data[NUM_DATA]);
        return EXIT_SUCCESS;
    }
}

EMSCRIPTEN_KEEPALIVE
double decode(unsigned char data[], int sizeData, unsigned char erasures[],
              int sizeErasures) {

    if (sizeData != NUM_SYMBOLS) {
        return -1.0;
    } else {
        int erasuresInt[sizeErasures];
        for (int i=0; i< sizeErasures; i++) {
            // need to add the padding
            erasuresInt[i] = PAD_DEFAULT + (int) erasures[i];
        }
        if (!rs) {
            init(BITS_PER_SYMBOL, GF_POLY_DEFAULT, FCR_DEFAULT, PRIM_DEFAULT,
                 NUM_PARITY, PAD_DEFAULT);
        }
        int nErrors = decode_rs_char(rs, data, erasuresInt, sizeErasures);
        if (nErrors > 0) {
            printf("Corrected  %d errors\n", nErrors);
        }
        if (nErrors >= 0) {
            uint64_t result = 0;
            for (int i= 0; i<NUM_DATA-1; i++) {
                result += data[i];
                result <<= BITS_PER_SYMBOL;
            }
            result += data[NUM_DATA-1];
            return (double) result;
        } else {
            printf("Cannot recover errors\n");
            return nErrors;
        }
    }
}
