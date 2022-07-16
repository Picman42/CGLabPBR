#pragma once

#include <map>
#include <string>
#include <vector>

#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>

#include <fstream>
#include <sstream>

typedef float real_t;

typedef struct {
    std::string name;

    std::vector<int> intValues;
    std::vector<float> floatValues;
    std::vector<std::string> stringValues;
} tag_t;

// Index struct to support different indices for vtx/normal/texcoord.
// -1 means not used.
typedef struct {
    int vertex_index;
    int normal_index;
    int texcoord_index;
} index_t;

typedef struct {
    std::vector<index_t> indices;
    std::vector<unsigned char> num_face_vertices;  // The number of vertices per
                                                   // face. 3 = polygon, 4 = quad,
                                                   // ... Up to 255.
    std::vector<int> material_ids;                 // per-face material ID
    std::vector<tag_t> tags;                       // SubD tag
} mesh_t;

typedef struct {
    std::string name;
    mesh_t mesh;
} shape_t;

// Vertex attributes
typedef struct {
    std::vector<float> vertices;   // 'v'
    std::vector<float> normals;    // 'vn'
    std::vector<float> texcoords;  // 'vt'
} attrib_t;

struct vertex_index {
    int v_idx, vt_idx, vn_idx;
    vertex_index() : v_idx(-1), vt_idx(-1), vn_idx(-1) {}
    explicit vertex_index(int idx) : v_idx(idx), vt_idx(idx), vn_idx(idx) {}
    vertex_index(int vidx, int vtidx, int vnidx)
        : v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};

struct obj_shape {
    std::vector<float> v;
    std::vector<float> vn;
    std::vector<float> vt;
};

static std::istream& safeGetline(std::istream& is, std::string& t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if (sb->sgetc() == '\n') sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if (t.empty()) is.setstate(std::ios::eofbit);
            return is;
        default:
            t += static_cast<char>(c);
        }
    }
}

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) \
  (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>(10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

static bool tryParseDouble(const char* s, const char* s_end, double* result) {
    if (s >= s_end) {
        return false;
    }

    double mantissa = 0.0;
    // This exponent is base 2 rather than 10.
    // However the exponent we parse is supposed to be one of ten,
    // thus we must take care to convert the exponent/and or the
    // mantissa to a * 2^E, where a is the mantissa and E is the
    // exponent.
    // To get the final double we will use ldexp, it requires the
    // exponent to be in base 2.
    int exponent = 0;

    // NOTE: THESE MUST BE DECLARED HERE SINCE WE ARE NOT ALLOWED
    // TO JUMP OVER DEFINITIONS.
    char sign = '+';
    char exp_sign = '+';
    char const* curr = s;

    // How many characters were read in a loop.
    int read = 0;
    // Tells whether a loop terminated due to reaching s_end.
    bool end_not_reached = false;

    /*
            BEGIN PARSING.
    */

    // Find out what sign we've got.
    if (*curr == '+' || *curr == '-') {
        sign = *curr;
        curr++;
    }
    else if (IS_DIGIT(*curr)) { /* Pass through. */
    }
    else {
        goto fail;
    }

    // Read the integer part.
    end_not_reached = (curr != s_end);
    while (end_not_reached && IS_DIGIT(*curr)) {
        mantissa *= 10;
        mantissa += static_cast<int>(*curr - 0x30);
        curr++;
        read++;
        end_not_reached = (curr != s_end);
    }

    // We must make sure we actually got something.
    if (read == 0) goto fail;
    // We allow numbers of form "#", "###" etc.
    if (!end_not_reached) goto assemble;

    // Read the decimal part.
    if (*curr == '.') {
        curr++;
        read = 1;
        end_not_reached = (curr != s_end);
        while (end_not_reached && IS_DIGIT(*curr)) {
            static const double pow_lut[] = {
                1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
            };
            const int lut_entries = sizeof pow_lut / sizeof pow_lut[0];

            // NOTE: Don't use powf here, it will absolutely murder precision.
            mantissa += static_cast<int>(*curr - 0x30) *
                (read < lut_entries ? pow_lut[read] : std::pow(10.0, -read));
            read++;
            curr++;
            end_not_reached = (curr != s_end);
        }
    }
    else if (*curr == 'e' || *curr == 'E') {
    }
    else {
        goto assemble;
    }

    if (!end_not_reached) goto assemble;

    // Read the exponent part.
    if (*curr == 'e' || *curr == 'E') {
        curr++;
        // Figure out if a sign is present and if it is.
        end_not_reached = (curr != s_end);
        if (end_not_reached && (*curr == '+' || *curr == '-')) {
            exp_sign = *curr;
            curr++;
        }
        else if (IS_DIGIT(*curr)) { /* Pass through. */
        }
        else {
            // Empty E is not allowed.
            goto fail;
        }

        read = 0;
        end_not_reached = (curr != s_end);
        while (end_not_reached && IS_DIGIT(*curr)) {
            exponent *= 10;
            exponent += static_cast<int>(*curr - 0x30);
            curr++;
            read++;
            end_not_reached = (curr != s_end);
        }
        exponent *= (exp_sign == '+' ? 1 : -1);
        if (read == 0) goto fail;
    }

assemble:
    *result =
        (sign == '+' ? 1 : -1) *
        (exponent ? std::ldexp(mantissa * std::pow(5.0, exponent), exponent) : mantissa);
    return true;
fail:
    return false;
}

static inline real_t parseReal(const char** token, double default_value = 0.0) {
    (*token) += strspn((*token), " \t");
    const char* end = (*token) + strcspn((*token), " \t\r");
    double val = default_value;
    tryParseDouble((*token), end, &val);
    real_t f = static_cast<real_t>(val);
    (*token) = end;
    return f;
}

static inline void parseReal2(real_t* x, real_t* y, const char** token,
    const double default_x = 0.0,
    const double default_y = 0.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
}

static inline void parseReal3(real_t* x, real_t* y, real_t* z, const char** token,
    const double default_x = 0.0,
    const double default_y = 0.0,
    const double default_z = 0.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);
}

static inline void parseV(real_t* x, real_t* y, real_t* z, real_t* w,
    const char** token, const double default_x = 0.0,
    const double default_y = 0.0,
    const double default_z = 0.0,
    const double default_w = 1.0) {
    (*x) = parseReal(token, default_x);
    (*y) = parseReal(token, default_y);
    (*z) = parseReal(token, default_z);
    (*w) = parseReal(token, default_w);
}

static inline bool parseOnOff(const char** token, bool default_value = true) {
    (*token) += strspn((*token), " \t");
    const char* end = (*token) + strcspn((*token), " \t\r");

    bool ret = default_value;
    if ((0 == strncmp((*token), "on", 2))) {
        ret = true;
    }
    else if ((0 == strncmp((*token), "off", 3))) {
        ret = false;
    }

    (*token) = end;
    return ret;
}

static inline std::string parseString(const char** token) {
    std::string s;
    (*token) += strspn((*token), " \t");
    size_t e = strcspn((*token), " \t\r");
    s = std::string((*token), &(*token)[e]);
    (*token) += e;
    return s;
}

static inline int parseInt(const char** token) {
    (*token) += strspn((*token), " \t");
    int i = atoi((*token));
    (*token) += strcspn((*token), " \t\r");
    return i;
}

static inline int fixIndex(int idx, int n) {
    if (idx > 0) return idx - 1;
    if (idx == 0) return 0;
    return n + idx;  // negative value = relative
}

static vertex_index parseTriple(const char** token, int vsize, int vnsize,
    int vtsize) {
    vertex_index vi(-1);

    vi.v_idx = fixIndex(atoi((*token)), vsize);
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return vi;
    }
    (*token)++;

    // i//k
    if ((*token)[0] == '/') {
        (*token)++;
        vi.vn_idx = fixIndex(atoi((*token)), vnsize);
        (*token) += strcspn((*token), "/ \t\r");
        return vi;
    }

    // i/j/k or i/j
    vi.vt_idx = fixIndex(atoi((*token)), vtsize);
    (*token) += strcspn((*token), "/ \t\r");
    if ((*token)[0] != '/') {
        return vi;
    }

    // i/j/k
    (*token)++;  // skip '/'
    vi.vn_idx = fixIndex(atoi((*token)), vnsize);
    (*token) += strcspn((*token), "/ \t\r");
    return vi;
}