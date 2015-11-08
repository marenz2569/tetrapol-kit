#define LOG_PREFIX "frame"
#include <tetrapol/log.h>
#include <tetrapol/tetrapol.h>
#include <tetrapol/frame.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// used when decoding firts part of frame, common to data and voice frames
enum {
    FRAME_DATA_LEN1 = 52,
};

struct frame_decoder_priv_t {
    int band;
    int scr;
    int fr_type;
};

/**
  PAS 0001-2 6.1.5.1
  PAS 0001-2 6.2.5.1
  PAS 0001-2 6.3.4.1

  Scrambling sequence was generated by this python3 script

  s = [1, 1, 1, 1, 1, 1, 1]
  for k in range(len(s), 127):
    s.append(s[k-1] ^ s[k-7])
  for i in range(len(s)):
    print(s[i], end=", ")
    if i % 8 == 7:
      print()
  */
static uint8_t scramb_table[127] = {
    1, 1, 1, 1, 1, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 1, 0, 1,
    1, 1, 0, 1, 0, 0, 1, 0,
    1, 1, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 0, 1, 1, 0,
    1, 0, 1, 1, 0, 1, 1, 0,
    0, 1, 0, 0, 1, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 0, 0,
    1, 0, 1, 0, 1, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 0,
    1, 0, 0, 1, 1, 1, 1, 0,
    0, 0, 1, 0, 1, 0, 0, 0,
    0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0,
};

static void frame_descramble(uint8_t *fr_data_tmp, const uint8_t *fr_data,
        int scr)
{
    if (scr == 0) {
        memcpy(fr_data_tmp, fr_data, FRAME_DATA_LEN);
        return;
    }

    for(int k = 0 ; k < FRAME_DATA_LEN; k++) {
        fr_data_tmp[k] = fr_data[k] ^ scramb_table[(k + scr) % 127];
    }
}

/**
  PAS 0001-2 6.1.4.2
  PAS 0001-2 6.2.4.2

  Audio and data frame differencial precoding index table was generated by the
  following python 3 scipt.

  pre_cod = ( 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40,
             43, 46, 49, 52, 55, 58, 61, 64, 67, 70, 73, 76,
             83, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116,
            119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149 )
  for i in range(152):
      print(1+ (i in pre_cod), end=", ")
      if i % 8 == 7:
          print()
*/
static const int diff_precod_UHF[] = {
    1, 1, 1, 1, 1, 1, 1, 2,
    1, 1, 2, 1, 1, 2, 1, 1,
    2, 1, 1, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 1, 1, 2,
    1, 1, 2, 1, 1, 2, 1, 1,
    2, 1, 1, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 1, 1, 2,
    1, 1, 2, 1, 1, 2, 1, 1,
    2, 1, 1, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 1, 1, 1,
    1, 1, 1, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 1, 1, 2,
    1, 1, 2, 1, 1, 2, 1, 1,
    2, 1, 1, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 1, 1, 2,
    1, 1, 2, 1, 1, 2, 1, 1,
    2, 1, 1, 2, 1, 1, 2, 1,
    1, 2, 1, 1, 2, 1, 1, 2,
    1, 1, 2, 1, 1, 2, 1, 1,
};

static void frame_diff_dec(uint8_t *fr_data)
{
    for (int j = FRAME_DATA_LEN - 1; j > 0; --j) {
        fr_data[j] ^= fr_data[j - diff_precod_UHF[j]];
    }
}

/**
  PAS 0001-2 6.1.3.1
  Generated by following python3 scritp.

p = { 0: 0, 1: 4, 2: 2, 3: 6, 4: 1, 5: 5, 6: 3, 7: 7, }
for j in range(0, 152):
    k = 19 * p[j % 8] + (3 * (j // 8)) % 19
    print(k, end=', ')
    if j % 8 == 7:
        print()
  **/
static const uint8_t interleave_voice_VHF[] = {
    0, 76, 38, 114, 19, 95, 57, 133,
    3, 79, 41, 117, 22, 98, 60, 136,
    6, 82, 44, 120, 25, 101, 63, 139,
    9, 85, 47, 123, 28, 104, 66, 142,
    12, 88, 50, 126, 31, 107, 69, 145,
    15, 91, 53, 129, 34, 110, 72, 148,
    18, 94, 56, 132, 37, 113, 75, 151,
    2, 78, 40, 116, 21, 97, 59, 135,
    5, 81, 43, 119, 24, 100, 62, 138,
    8, 84, 46, 122, 27, 103, 65, 141,
    11, 87, 49, 125, 30, 106, 68, 144,
    14, 90, 52, 128, 33, 109, 71, 147,
    17, 93, 55, 131, 36, 112, 74, 150,
    1, 77, 39, 115, 20, 96, 58, 134,
    4, 80, 42, 118, 23, 99, 61, 137,
    7, 83, 45, 121, 26, 102, 64, 140,
    10, 86, 48, 124, 29, 105, 67, 143,
    13, 89, 51, 127, 32, 108, 70, 146,
    16, 92, 54, 130, 35, 111, 73, 149,
};

// PAS 0001-2 6.1.4.1
static const uint8_t interleave_voice_UHF[] = {
    1, 77, 38, 114, 20, 96, 59, 135,
    3, 79, 41, 117, 23, 99, 62, 138,
    5, 81, 44, 120, 26, 102, 65, 141,
    8, 84, 47, 123, 29, 105, 68, 144,
    11, 87, 50, 126, 32, 108, 71, 147,
    14, 90, 53, 129, 35, 111, 74, 150,
    17, 93, 56, 132, 37, 113, 73, 4,
    0, 76, 40, 119, 19, 95, 58, 137,
    151, 80, 42, 115, 24, 100, 60, 133,
    12, 88, 48, 121, 30, 106, 66, 139,
    18, 91, 51, 124, 28, 104, 67, 146,
    10, 89, 52, 131, 34, 110, 70, 149,
    13, 97, 57, 130, 36, 112, 75, 148,
    6, 82, 39, 116, 16, 92, 55, 134,
    2, 78, 43, 122, 22, 98, 61, 140,
    9, 85, 45, 118, 27, 103, 63, 136,
    15, 83, 46, 125, 25, 101, 64, 143,
    7, 86, 49, 128, 31, 107, 69, 142,
    21, 94, 54, 127, 33, 109, 72, 145,
};

// PAS 0001-2 6.2.3.1
static const uint8_t *interleave_data_VHF = interleave_voice_VHF;

// PAS 0001-2 6.2.4.1
static const uint8_t interleave_data_UHF[] = {
    1, 77, 38, 114, 20, 96, 59, 135,
    3, 79, 41, 117, 23, 99, 62, 138,
    5, 81, 44, 120, 26, 102, 65, 141,
    8, 84, 47, 123, 29, 105, 68, 144,
    11, 87, 50, 126, 32, 108, 71, 147,
    14, 90, 53, 129, 35, 111, 74, 150,
    17, 93, 56, 132, 37, 112, 76, 148,
    2, 88, 40, 115, 19, 97, 58, 133,
    4, 75, 43, 118, 22, 100, 61, 136,
    7, 85, 46, 121, 25, 103, 64, 139,
    10, 82, 49, 124, 28, 106, 67, 142,
    13, 91, 52, 127, 31, 109, 73, 145,
    16, 94, 55, 130, 34, 113, 70, 151,
    0, 80, 39, 116, 21, 95, 57, 134,
    6, 78, 42, 119, 24, 98, 60, 137,
    9, 83, 45, 122, 27, 101, 63, 140,
    12, 86, 48, 125, 30, 104, 66, 143,
    15, 89, 51, 128, 33, 107, 69, 146,
    18, 92, 54, 131, 36, 110, 72, 149,
};

/**
  Deinterleave firts part of frame (common for data and voice frames)
  */
static void frame_deinterleave1(uint8_t *fr_data_deint, const uint8_t *fr_data,
        int band)
{
    const uint8_t *int_table;

    if (band == TETRAPOL_BAND_VHF) {
        int_table = interleave_data_VHF;
    } else {
        int_table = interleave_data_UHF;
    }

    for (int j = 0; j < FRAME_DATA_LEN1; ++j) {
        fr_data_deint[j] = fr_data[int_table[j]];
    }
}

/**
  Deinterleave second part of frame (differs for data and voice frames)
  */
static void frame_deinterleave2(uint8_t *fr_data_deint, const uint8_t *fr_data,
        int band, int fr_type)
{
    const uint8_t *int_table;

    if (band == TETRAPOL_BAND_VHF) {
        if (fr_type == FRAME_TYPE_DATA) {
            int_table = interleave_data_VHF;
        } else {
            int_table = interleave_voice_VHF;
        }
    } else {
        if (fr_type == FRAME_TYPE_DATA) {
            int_table = interleave_data_UHF;
        } else {
            int_table = interleave_voice_UHF;
        }
    }

    for (int j = FRAME_DATA_LEN1; j < FRAME_DATA_LEN; ++j) {
        fr_data_deint[j] = fr_data[int_table[j]];
    }
}

// http://ghsi.de/CRC/index.php?Polynom=10010
static void mk_crc5(uint8_t *res, const uint8_t *input, int input_len)
{
    uint8_t inv;
    memset(res, 0, 5);

    for (int i = 0; i < input_len; ++i)
    {
        inv = input[i] ^ res[0];

        res[0] = res[1];
        res[1] = res[2];
        res[2] = res[3] ^ inv;
        res[3] = res[4];
        res[4] = inv;
    }
}

// http://ghsi.de/CRC/index.php?Polynom=1010
static void mk_crc3(uint8_t *res, const uint8_t *input, int input_len)
{
    uint8_t inv;
    memset(res, 0, 3);

    for (int i = 0; i < input_len; ++i)
    {
        inv = input[i] ^ res[0];

        res[0] = res[1];
        res[1] = res[2] ^ inv;
        res[2] = inv;
    }
    res[0] = res[0] ^ 1;
    res[1] = res[1] ^ 1;
    res[2] = res[2] ^ 1;
}

/**
  PAS 0001-2 6.1.2
  PAS 0001-2 6.2.2
  */
static int decode_data_frame(uint8_t *fr_sol, uint8_t *fr_errs,
        const uint8_t *fr_data, int sol_len)
{
#ifdef FRBIT
#error "Collision in definition of macro FRBIT!"
#endif
#define FRBIT(x, y) fr_data[((x) + (y)) % (2*sol_len)]

    int nerrs = 0;
    for (int i = 0; i < sol_len; ++i) {
        fr_sol[i] = FRBIT(2*i, 2) ^ FRBIT(2*i, 3);
        const uint8_t sol2 = FRBIT(2*i, 5) ^ FRBIT(2*i, 6) ^ FRBIT(2*i, 7);
        // we have 2 solutions, check if they match
        fr_errs[i] = fr_sol[i] ^ sol2;
        nerrs += fr_errs[i];
    }
#undef FRBIT

    return nerrs;
}

static int frame_decode1(uint8_t *fr_sol, uint8_t *fr_errs,
        const uint8_t *fr_data, frame_type_t fr_type)
{
    switch (fr_type) {
        case FRAME_TYPE_AUTO:
        case FRAME_TYPE_DATA:
        case FRAME_TYPE_VOICE:
            // decode 52 bits of frame common to both types
            return decode_data_frame(fr_sol, fr_errs, fr_data, 26);
        break;

        default:
            // TODO
            LOG(ERR, "decoding frame type %d not implemented", fr_type);
            return INT_MAX;
    }
}

static int frame_decode2(uint8_t *fr_sol, uint8_t *fr_errs,
        const uint8_t *fr_data, frame_type_t fr_type)
{
    int nerrs = 0;

    switch (fr_type) {
        case FRAME_TYPE_VOICE:
            memcpy(fr_sol + 26, fr_data + 2*26, 100);
            break;

        case FRAME_TYPE_DATA:
            // decode remaining part of frame
            nerrs += decode_data_frame(fr_sol + 26, fr_errs + 26, fr_data + 2*26, 50);
            if (!nerrs && ( fr_sol[74] || fr_sol[75] )) {
                LOG(WTF, "nonzero padding in frame: %d %d",
                        fr_sol[74], fr_sol[75]);
            }
        break;

        default:
            LOG(ERR, "decoding frame type %d not implemented", fr_type);
            nerrs = INT_MAX;
    }

    return nerrs;
}

static bool frame_check_crc(const uint8_t *fr_data, frame_type_t fr_type)
{
    if (fr_type == FRAME_TYPE_AUTO) {
        fr_type = fr_data[0];
    } else {
        if (fr_type != fr_data[0]) {
            return false;
        }
    }

    if (fr_type == FRAME_TYPE_DATA) {
        uint8_t crc[5];

        mk_crc5(crc, fr_data, 69);
        return !memcmp(fr_data + 69, crc, 5);
    }

    if (fr_type == FRAME_TYPE_VOICE) {
        uint8_t crc[3];

        mk_crc3(crc, fr_data, 23);
        return !memcmp(fr_data + 23, crc, 3);
    }
    return false;
}

frame_decoder_t *frame_decoder_create(int band, int scr, int fr_type)
{
    frame_decoder_t *fd = malloc(sizeof(frame_decoder_t));
    if (!fd) {
        return NULL;
    }

    frame_decoder_reset(fd, band, scr, fr_type);

    return fd;
}

void frame_decoder_destroy(frame_decoder_t *fd)
{
    free(fd);
}

void frame_decoder_reset(frame_decoder_t *fd, int band, int scr, int fr_type)
{
    fd->band = band;
    fd->scr = scr;
    fd->fr_type = fr_type;
}

void frame_decoder_set_scr(frame_decoder_t *fd, int scr)
{
    fd->scr = scr;
}

/**
  Fix some errors in frame. This routine is pretty naive, suboptimal and does
  not use all possible potential of error correction.
  TODO, FIXME ...

  For those who want to improve it (or rather write new one from scratch!).
  Each sigle bit error in fr_data can be found by characteristic pattern
  (syndrome) in fr_errs and fixed by inverting invalid bits in fr_data.
  Trivial syndromes are are 101 and 111, both can be fixed by xoring
  fr_data with pattern 001 (xor last bit of syndrome).

  If more errors occures the resulting syndrome is linear combination of
  two trivial syndromes, the same stay for correction.

Example for 2 bit error:
syndrome    correction
  101       001
    111       001
  10011     00101
  */
int frame_fix_errs(uint8_t *fr_data, uint8_t *fr_errs, int len)
{
    int nerrs = 0;
    for (int i = 0; i < len; ++i) {
        // Fixe some 3 bit error with 5 bit syndromes. There is few cases of
        // those even for single bite error in demodulated frame.
        if (!(fr_errs[i] |
                    (fr_errs[(i + 1) % len]) |
                    (fr_errs[(i + 2) % len]) |
                    (fr_errs[(i + 3) % len]) |
                    (fr_errs[(i + 4) % len] ^ 1) |
                    //
                    (fr_errs[(i + 8) % len] ^ 1) |
                    (fr_errs[(i + 9) % len]) |
                    (fr_errs[(i + 10) % len]) |
                    (fr_errs[(i + 11) % len]) |
                    (fr_errs[(i + 12) % len])
             )) {
            nerrs += 2 + fr_errs[(i + 5) % len] +
                fr_errs[(i + 6) % len] +
                fr_errs[(i + 7) % len];
            fr_data[(i + 6) % len] ^= 1;
            fr_data[(i + 7) % len] ^= fr_errs[(i + 6) % len];
            fr_data[(i + 8) % len] ^= 1;
            fr_errs[(i + 4) % len] = 0;
            fr_errs[(i + 5) % len] = 0;
            fr_errs[(i + 6) % len] = 0;
            fr_errs[(i + 7) % len] = 0;
            fr_errs[(i + 8) % len] = 0;
            i += 6;
            continue;
        }
        // Fix some 2 bit errors with 4 bit syndromes.
        if (!(fr_errs[i] |
                (fr_errs[(i + 1) % len]) |
                (fr_errs[(i + 2) % len]) |
                (fr_errs[(i + 3) % len] ^ 1) |
                //
                (fr_errs[(i + 6) % len] ^ 1) |
                (fr_errs[(i + 7) % len]) |
                (fr_errs[(i + 8) % len]) |
                (fr_errs[(i + 9) % len])
             )) {
            nerrs += 2 + (fr_errs[(i + 4) % len]) + (fr_errs[(i + 5) % len]);
            fr_data[(i + 5) % len] ^= 1;
            fr_data[(i + 6) % len] ^= 1;
            fr_errs[(i + 3) % len] = 0;
            fr_errs[(i + 4) % len] = 0;
            fr_errs[(i + 5) % len] = 0;
            fr_errs[(i + 6) % len] = 0;
            i += 4;
            continue;
        }
        // Fix some 1 bit error with 3 bit syndromes.
        if (!(fr_errs[i] |
                (fr_errs[(i + 1) % len]) |
                (fr_errs[(i + 2) % len] ^ 1) |
                //
                (fr_errs[(i + 4) % len] ^ 1) |
                (fr_errs[(i + 5) % len]) |
                (fr_errs[(i + 6) % len])
             )) {
            nerrs += 2 + (fr_errs[(i + 3) % len]);
            fr_data[(i + 4) % len] ^= 1;
            fr_errs[(i + 2) % len] = 0;
            fr_errs[(i + 3) % len] = 0;
            fr_errs[(i + 4) % len] = 0;
            i += 4;
            continue;
        }
    }

    return nerrs;
}

void frame_decoder_decode(frame_decoder_t *fd, frame_t *fr, const uint8_t *fr_data)
{
    if (fd->fr_type != FRAME_TYPE_AUTO &&
            fd->fr_type != FRAME_TYPE_VOICE &&
            fd->fr_type  != FRAME_TYPE_DATA)
    {
        fr->broken = -2;
        return;
    }

    uint8_t fr_data_tmp[FRAME_DATA_LEN];

    frame_descramble(fr_data_tmp, fr_data, fd->scr);
    if (fd->band == TETRAPOL_BAND_UHF) {
        frame_diff_dec(fr_data_tmp);
    }

    uint8_t fr_data_deint[FRAME_DATA_LEN];
    uint8_t fr_errs[FRAME_DATA_LEN];

    frame_deinterleave1(fr_data_deint, fr_data_tmp, fd->band);
    fr->broken = frame_decode1(fr->blob_, fr_errs, fr_data_deint, fd->fr_type);

    fr->fr_type = (fd->fr_type == FRAME_TYPE_AUTO) ? fr->d : fd->fr_type;

    if (fr->broken) {
        fr->broken -= frame_fix_errs(fr->blob_, fr_errs, 26);
        if (fr->broken > 0) {
            return;
        }
    }

    frame_deinterleave2(fr_data_deint, fr_data_tmp, fd->band, fr->fr_type);
    fr->broken = frame_decode2(fr->blob_, fr_errs, fr_data_deint, fr->fr_type);

    if (fr->fr_type == FRAME_TYPE_VOICE) {
        fr->broken = frame_check_crc(fr->blob_, fr->fr_type) ? 0 : -1;
        return;
    }

    if (fr->broken) {
        fr->broken -= frame_fix_errs(fr->blob_ + 26, fr_errs + 26, 50);
        if (fr->broken > 0) {
            return;
        }
    }

    fr->broken = frame_check_crc(fr->blob_, fr->fr_type) ? 0 : -1;
}

