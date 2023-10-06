/* very simple bitmap library version exp 0.56
 * by Erik S.
 * 
 * This library is an improved version of my original bitmap library.
 * The original library used vectors to store the bitmap data. (It was too slow)
 * 
 * This library can load 32bit bitmap image files since ver exp 0.35
 * The function is currently very picky because it expects a very basic bmp and dib header
 * This means that any extra information like embedded icc color profiles or other stuff will
 * be ignored or more likely cause the load function to crash
 * No support for compressed bitmaps
 * 
 * I used the following resources to help me with this library:
 * https://en.wikipedia.org/wiki/BMP_file_format
 * https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * 
 * Expect bugs, crashes, memory corruption, leaks and all of the bad stuff
 * 
 * 
 * Oh and my English sucks...
 * 
 * 
 * Changelog:
 * 
 * <0.27:
 * - old library
 * 
 * 0.27:
 * - switched from vector to malloc
 * 
 * 
 * 0.28:
 * - No functions crash if coordinates are out of bounds.
 * - removed (some) old commented out code.
 * - added save function that accepts a string as filename (only if std::string is included).
 * - renamed "createimage" to "create" (may change it back again in the future)
 * 
 * 0.29:
 * - fixed bug that made the image upside down. (How did this even stay unnoticed for so long?)
 * 
 * 0.30:
 * - No real update, but added namespace sbtmp.
 * 
 * 0.31:
 * - Broken functions are only active if sbtmp_experimental is defined.
 * 
 * 0.32:
 * - Invert function fixed (renamed to rgb_invert)
 * - convert_bw function added
 * 
 * 0.33:
 * - Functions flip_vertical and flip_horizontal are swapped
 * - Fixed flip_vertical and flip_horizontal
 * 
 * 0.34:
 * - Fixed header bug that caused transparent parts to be not transparent
 * 
 * 0.35:
 * - Added color datatype (no real use yet)
 * - Added image load function
 * - Added floodfill function
 *
 * 0.36:
 * - Circle function is fixed (not all pixels of a circle have to be inside the image)
 *      -Area of circles can be outside of an image and be only partly visible
 * - Changed color datatype from class to struct
 * - Colors can now be passed to a function directly
 *      -some_func(sbtmp::color(sbtmp_blue)); //this works now
 * 
 * 0.37
 * - Line function fixed (FINALLY)
 * - Triangle function rewritten and fixed
 * 
 * 0.38
 * - some random fixes and improvements
 * 
 * 0.39 (is this even an update?)
 * - permanently removed floodfill function
 *      - causes stack overflow when the part that should be filled is too big
 *      - iterative version will not be developed
 *      - consider floodfill as dead
 * 
 * 0.40
 * - removed color class and replaced it with a typedef
 * 
 * 0.41
 * - added destructor to prevent memory leak
 * 
 * 0.42
 * - added a blur function
 * 
 * 0.43
 * - added/fixed floodfill function
 * 
 * 0.44
 * - improved the blur function
 * 
 * 0.45
 * - (almost) all function don't use/accept a alpha value anymore
 *      - use *fn*_a version of the function to use alpha
 * - floodfill now accepts rgba/rgb values instead of just the "Color" type
 * - crossed the 1000 lines mark. Yay
 * 
 * 0.46
 * - added ring drawing function
 * - renamed a few functions
 * 
 * 0.47
 * - added a copy constructor
 * 
 * 0.48
 * - added simple char/string drawing function (only capital letters)
 * 
 * 0.49
 * - added all 52 letters
 * 
 * 0.50
 * - added all standard ascii characters
 * - draw_char can draw a char-bitmap or a char now
 * 
 * 0.51
 * - fixed blur functions by introducing a buffer
 * - fixed a double free bug in the destructor
 * 
 * 0.52
 * - fixed major bug: Image had to be a perfect square else sbtmp would crash
 * 
 * 0.53
 * - more bug fixes
 * - shapes can now be placed on the image (even outside)
 * - added border drawing function
 *
 * 0.54 (more changed no one asked for...)
 * - added steganography encoding and decoding to hide text messages in an image
 *
 * 0.55
 * - renamed triangle function to triangle_border function
 * - added a real triangle function
 *
 * 0.56
 * - added ellipse drawing function
 * - added circle sector drawing function
 * - added ellipse sector drawing function
 * 
 * TODO:
 * - improve triangle function (maybe copy from rsbtmp?) (Yes sbtmp exists for Rust. Still WIP and very early though. Has more features than this C++ version though)
 * - add thickness parameter to triangle_border function
 * - fix coord limits in rect|border and others
 * - add more shapes (round rectangle, round rectangle border)
 * - add more filters and resize functions (nearest neighbour | bilinear | bicubic)
 */

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stack>
#include <cstdint>
#include <cmath>
#include <stdint.h>

namespace sbtmp{

    namespace color{

        typedef uint32_t Color;
        
        constexpr Color clear       = 0x00000000;
        constexpr Color black       = 0x000000ff;
        constexpr Color white       = 0xffffffff;
        constexpr Color red         = 0x0000ffff;
        constexpr Color dark_red    = 0x000080ff;
        constexpr Color green       = 0x00ff00ff;
        constexpr Color dark_green  = 0x008000ff;
        constexpr Color blue        = 0xff0000ff;
        constexpr Color dark_blue   = 0x800000ff;
        constexpr Color purple      = 0xff00ffff;
        constexpr Color dark_purple = 0x800080ff;
        constexpr Color yellow      = 0x00ffffff;
        constexpr Color orange      = 0x00A5ffff;
        constexpr Color cyan        = 0xffff00ff;


        inline Color set_col(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            return (uint32_t)blue << 24 | (uint32_t)green << 16 | (uint32_t)red << 8 | (uint32_t)alpha;
        }

        inline void set_col(Color &col, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            col = (uint32_t)blue << 24 | (uint32_t)green << 16 | (uint32_t)red << 8 | (uint32_t)alpha;
        }

        inline void set_red(Color &col, uint8_t red){
            col |= (uint32_t)red << 8;
        }

        inline void set_green(Color &col, uint8_t green){
            col |= (uint32_t)green << 16;
        }

        inline void set_blue(Color &col, uint8_t blue){
            col |= (uint32_t)blue << 24;
        }

        inline void set_alpha(Color &col, uint32_t alpha){
            col |= (uint32_t)alpha;
        }

        inline uint8_t get_red(Color col){
            return col >> 8;
        }

        inline uint8_t get_green(Color col){
            return col >> 16;
        }

        inline uint8_t get_blue(Color col){
            return col >> 24;
        }

        inline uint8_t get_alpha(Color col){
            return col;
        }

        inline Color col_avg(Color col1, Color col2){
            short red = 0, green = 0, blue = 0, alpha = 0;
            red = (get_red(col1) + get_red(col2)) / 2;
            green = (get_green(col1) + get_green(col2)) / 2;
            blue = (get_blue(col1) + get_blue(col2)) / 2;
            alpha = (get_alpha(col1) + get_alpha(col2)) / 2;
            return set_col(red, green, blue, alpha);
        }

    }

    namespace chars{

        typedef char Charbtmp[8];

        constexpr Charbtmp A = {0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
        constexpr Charbtmp B = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x11, 0x1E};
        constexpr Charbtmp C = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x10, 0x11, 0x0E};
        constexpr Charbtmp D = {0x1C, 0x12, 0x11, 0x11, 0x11, 0x11, 0x12, 0x1C};
        constexpr Charbtmp E = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10, 0x1F};
        constexpr Charbtmp F = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10, 0x10};
        constexpr Charbtmp G = {0x0E, 0x11, 0x11, 0x10, 0x12, 0x11, 0x11, 0x0E};
        constexpr Charbtmp H = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x11};
        constexpr Charbtmp I = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
        constexpr Charbtmp J = {0x1F, 0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E};
        constexpr Charbtmp K = {0x11, 0x12, 0x14, 0x1C, 0x14, 0x12, 0x12, 0x11};
        constexpr Charbtmp L = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
        constexpr Charbtmp M = {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11, 0x11};
        constexpr Charbtmp N = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11, 0x11};
        constexpr Charbtmp O = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp P = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10, 0x10};
        constexpr Charbtmp Q = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x15, 0x13, 0x0F};
        constexpr Charbtmp R = {0x1E, 0x11, 0x11, 0x1E, 0x18, 0x14, 0x12, 0x11};
        constexpr Charbtmp S = {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x01, 0x11, 0x0E};
        constexpr Charbtmp T = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
        constexpr Charbtmp U = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp V = {0x11, 0x11, 0x11, 0x0A, 0x0A, 0x0A, 0x04, 0x04};
        constexpr Charbtmp W = {0x11, 0x11, 0x11, 0x11, 0x11, 0x15, 0x1B, 0x11};
        constexpr Charbtmp X = {0x11, 0x0A, 0x0A, 0x04, 0x04, 0x0A, 0x0A, 0x11};
        constexpr Charbtmp Y = {0x11, 0x0A, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04};
        constexpr Charbtmp Z = {0x1F, 0x01, 0x02, 0x04, 0x04, 0x08, 0x10, 0x1F};

        constexpr Charbtmp a = {0x00, 0x00, 0x00, 0x0F, 0x11, 0x11, 0x13, 0x0F};
        constexpr Charbtmp b = {0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp c = {0x00, 0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F};
        constexpr Charbtmp d = {0x01, 0x01, 0x01, 0x0F, 0x11, 0x11, 0x11, 0x0F};
        constexpr Charbtmp e = {0x00, 0x00, 0x00, 0x0E, 0x11, 0x1E, 0x10, 0x0E};
        constexpr Charbtmp f = {0x03, 0x04, 0x04, 0x0E, 0x04, 0x04, 0x04, 0x04};
        constexpr Charbtmp g = {0x00, 0x00, 0x00, 0x0E, 0x11, 0x0F, 0x01, 0x0E};
        constexpr Charbtmp h = {0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x11};
        constexpr Charbtmp i = {0x00, 0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x04};
        constexpr Charbtmp j = {0x00, 0x00, 0x02, 0x00, 0x02, 0x02, 0x0A, 0x04};
        constexpr Charbtmp k = {0x10, 0x10, 0x10, 0x11, 0x16, 0x18, 0x16, 0x11};
        constexpr Charbtmp l = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x02};
        constexpr Charbtmp m = {0x00, 0x00, 0x00, 0x0A, 0x15, 0x15, 0x11, 0x11};
        constexpr Charbtmp n = {0x00, 0x00, 0x00, 0x1F, 0x09, 0x09, 0x09, 0x09};
        constexpr Charbtmp o = {0x00, 0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp p = {0x00, 0x00, 0x00, 0x0E, 0x11, 0x1E, 0x10, 0x10};
        constexpr Charbtmp q = {0x00, 0x00, 0x00, 0x0E, 0x11, 0x0F, 0x01, 0x01};
        constexpr Charbtmp r = {0x00, 0x00, 0x00, 0x0A, 0x0C, 0x08, 0x08, 0x08};
        constexpr Charbtmp s = {0x00, 0x00, 0x00, 0x0E, 0x10, 0x0E, 0x01, 0x0E};
        constexpr Charbtmp t = {0x04, 0x04, 0x04, 0x0E, 0x04, 0x04, 0x04, 0x02};
        constexpr Charbtmp u = {0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp v = {0x00, 0x00, 0x00, 0x11, 0x11, 0x0A, 0x0A, 0x04};
        constexpr Charbtmp w = {0x00, 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A};
        constexpr Charbtmp x = {0x00, 0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11};
        constexpr Charbtmp y = {0x00, 0x00, 0x00, 0x0A, 0x04, 0x04, 0x04, 0x04};
        constexpr Charbtmp z = {0x00, 0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F};

        constexpr Charbtmp one = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
        constexpr Charbtmp two = {0x0E, 0x11, 0x01, 0x03, 0x0C, 0x10, 0x10, 0x1F};
        constexpr Charbtmp three = {0x0E, 0x11, 0x01, 0x0E, 0x01, 0x01, 0x11, 0x0E};
        constexpr Charbtmp four = {0x08, 0x08, 0x12, 0x12, 0x1F, 0x02, 0x02, 0x02};
        constexpr Charbtmp five = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
        constexpr Charbtmp six = {0x0E, 0x11, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp seven = {0x1F, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08};
        constexpr Charbtmp eight = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x11, 0x0E};
        constexpr Charbtmp nine = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x11, 0x0E};
        constexpr Charbtmp zero = {0x0E, 0x13, 0x13, 0x15, 0x15, 0x19, 0x19, 0x0E};

        constexpr Charbtmp exclamation_mark = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04};
        constexpr Charbtmp double_quote = {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        constexpr Charbtmp hash = {0x0A, 0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x0A};
        constexpr Charbtmp dollar = {0x00, 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04};
        constexpr Charbtmp percent = {0x19, 0x1A, 0x02, 0x04, 0x04, 0x08, 0x0B, 0x13};
        constexpr Charbtmp ampersand = {0x0E, 0x11, 0x11, 0x12, 0x0C, 0x15, 0x12, 0x0D};
        constexpr Charbtmp single_quote = {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        constexpr Charbtmp left_round_bracket = {0x02, 0x04, 0x08, 0x08, 0x08, 0x08, 0x04, 0x02};
        constexpr Charbtmp right_round_bracket = {0x08, 0x04, 0x02, 0x02, 0x02, 0x02, 0x04, 0x08};
        constexpr Charbtmp asterisk = {0x00, 0x00, 0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15};
        constexpr Charbtmp plus = {0x00, 0x00, 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04};
        constexpr Charbtmp comma = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08};
        constexpr Charbtmp minus = {0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00};
        constexpr Charbtmp dot = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
        constexpr Charbtmp slash = {0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10};
        constexpr Charbtmp colon = {0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04};
        constexpr Charbtmp semicolon = {0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x04, 0x08};
        constexpr Charbtmp left_angle_bracket = {0x01, 0x02, 0x04, 0x08, 0x08, 0x04, 0x02, 0x01};
        constexpr Charbtmp equal = {0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00};
        constexpr Charbtmp right_angle_bracket = {0x10, 0x08, 0x04, 0x02, 0x02, 0x04, 0x08, 0x10};
        constexpr Charbtmp question_mark = {0x0E, 0x01, 0x0E, 0x10, 0x10, 0x0E, 0x00, 0x04};
        constexpr Charbtmp at = {0x0E, 0x15, 0x1B, 0x1B, 0x1D, 0x16, 0x10, 0x0E};
        constexpr Charbtmp left_square_bracket = { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E};
        constexpr Charbtmp backslash = {0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01};
        constexpr Charbtmp right_square_bracket = {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E};
        constexpr Charbtmp caret = {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
        constexpr Charbtmp underscore = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F};
        constexpr Charbtmp backtick = {0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        constexpr Charbtmp left_curly_bracket = {0x02, 0x04, 0x04, 0x18, 0x18, 0x04, 0x04, 0x02};
        constexpr Charbtmp pipe = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
        constexpr Charbtmp right_curly_bracket = {0x08, 0x04, 0x04, 0x03, 0x03, 0x04, 0x04, 0x08};
        constexpr Charbtmp tilde = {0x00, 0x00, 0x00, 0x00, 0x08, 0x15, 0x02, 0x00};

        //convert any char to the corresponding predefined char-bitmap
        //this is probably inefficient AF but I don't really care,
        //so here you have a massive switch case statement to make your eyes bleed
        inline const char *asciitocbtmp(const char ascii){
            switch(ascii){
                case 'a': return a; break;
                case 'b': return b; break;
                case 'c': return c; break;
                case 'd': return d; break;
                case 'e': return e; break;
                case 'f': return f; break;
                case 'g': return g; break;
                case 'h': return h; break;
                case 'i': return i; break;
                case 'j': return j; break;
                case 'k': return k; break;
                case 'l': return l; break;
                case 'm': return m; break;
                case 'n': return n; break;
                case 'o': return o; break;
                case 'p': return p; break;
                case 'q': return q; break;
                case 'r': return r; break;
                case 's': return s; break;
                case 't': return t; break;
                case 'u': return u; break;
                case 'v': return v; break;
                case 'w': return w; break;
                case 'x': return x; break;
                case 'y': return y; break;
                case 'z': return z; break;

                case 'A': return A; break;
                case 'B': return B; break;
                case 'C': return C; break;
                case 'D': return D; break;
                case 'E': return E; break;
                case 'F': return F; break;
                case 'G': return G; break;
                case 'H': return H; break;
                case 'I': return I; break;
                case 'J': return J; break;
                case 'K': return K; break;
                case 'L': return L; break;
                case 'M': return M; break;
                case 'N': return N; break;
                case 'O': return O; break;
                case 'P': return P; break;
                case 'Q': return Q; break;
                case 'R': return R; break;
                case 'S': return S; break;
                case 'T': return T; break;
                case 'U': return U; break;
                case 'V': return V; break;
                case 'W': return W; break;
                case 'X': return X; break;
                case 'Y': return Y; break;
                case 'Z': return Z; break;

                case '1': return one; break;
                case '2': return two; break;
                case '3': return three; break;
                case '4': return four; break;
                case '5': return five; break;
                case '6': return six; break;
                case '7': return seven; break;
                case '8': return eight; break;
                case '9': return nine; break;
                case '0': return zero; break;

                case '!': return exclamation_mark; break;
                case '\"': return double_quote; break;
                case '#': return hash; break;
                case '$': return dollar; break;
                case '%': return percent; break;
                case '&': return ampersand; break;
                case '\'': return single_quote; break;
                case '(': return left_round_bracket; break;
                case ')': return right_round_bracket; break;
                case '*': return asterisk; break;
                case '+': return plus; break;
                case ',': return comma; break;
                case '-': return minus; break;
                case '.': return dot; break;
                case '/': return slash; break;
                case ':': return colon; break;
                case ';': return semicolon; break;
                case '<': return left_angle_bracket; break;
                case '=': return equal; break;
                case '>': return right_angle_bracket; break;
                case '?': return question_mark; break;
                case '@': return at; break;
                case '[': return left_square_bracket; break;
                case '\\': return backslash; break;
                case ']': return right_square_bracket; break;
                case '^': return caret; break;
                case '_': return underscore; break;
                case '`': return backtick; break;
                case '{': return left_curly_bracket; break;
                case '|': return pipe; break;
                case '}': return right_curly_bracket; break;
                case '~': return tilde; break;

                default: return question_mark; break;
            }
        }
    }

    class Bitmap{
        public:

        //constructor
        Bitmap(uint32_t set_width, uint32_t set_height) {
            if(initialized)
                return;

            btmp_width = set_width;
            btmp_height = set_height;

            total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 4;
            raw_data_size = btmp_height * btmp_width * 4;

            //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
            pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));

            initialized = true;
        }

        //copy constructor
        //creates a perfect copy of the original image
        //it also copies emtpy images by first freeing all data
        Bitmap(Bitmap &other){
            initialized = other.initialized;
            total_size_in_bytes = other.total_size_in_bytes;
            btmp_width = other.btmp_width;
            btmp_height = other.btmp_height;
            raw_data_size = other.raw_data_size;
            if(pixel_data)
                free(pixel_data);
            if(other.pixel_data){
                pixel_data = (uint8_t*)malloc(raw_data_size);
                for(uint32_t i = 0; i < raw_data_size; i++){
                    pixel_data[i] = other.pixel_data[i];
                }
            }
        }

        //allows not using the constructor
        Bitmap() = default;

        //destructor
        ~Bitmap(){
            // free data to prevent memory leak
            if(!pixel_data)
                free(pixel_data);
        }

        //create function (recommended way to init images)
        void create(uint32_t set_width, uint32_t set_height){
            if(initialized)
                return;

            btmp_width = set_width;
            btmp_height = set_height;

            total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 4;
            raw_data_size = btmp_height * btmp_width * 4;

            //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
            pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));

            initialized = true;
        }

        //return width of the image
        uint32_t get_width(){
            return btmp_width;
        }

        //returns height of the image
        uint32_t get_height(){
            return btmp_height;
        }

        //returns the entire file size including header 
        uint32_t get_total_size(){
            return total_size_in_bytes;
        }

        //returns the raw-image-size
        uint32_t get_size(){
            return raw_data_size;
        }

        //set pixel at coords x_pos, y_pos to rgba value
        void set_pixel_a(int32_t x_pos, int32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] = blue;
            pixel_data[get_index(x_pos, y_pos)+1] = green;
            pixel_data[get_index(x_pos, y_pos)+2] = red;
            pixel_data[get_index(x_pos, y_pos)+3] = alpha;
        }

        //set pixel at coords x_pos, y_pos to rgb value
        //no alpha
        void set_pixel(int32_t x_pos, int32_t y_pos, uint8_t red, uint8_t green, uint8_t blue){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] = blue;
            pixel_data[get_index(x_pos, y_pos)+1] = green;
            pixel_data[get_index(x_pos, y_pos)+2] = red;
            pixel_data[get_index(x_pos, y_pos)+3] = 255;
        }

        //set pixel at coords x_pos, y_pos to rgba value
        void set_pixel_a(int32_t x_pos, int32_t y_pos, color::Color val){
            set_pixel_a(x_pos, y_pos, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //set pixel at coords x_pos, y_pos to rgb value
        //no alpha
        void set_pixel(int32_t x_pos, int32_t y_pos, color::Color val){
            set_pixel(x_pos, y_pos, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //change the transparency of a pixel
        void set_alpha(int32_t x_pos, int32_t y_pos, uint8_t alpha){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+3] = alpha;
        }

        //draws line between two points
        void line_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(!initialized)
                return;
            int32_t ax = x2 - x1, ay = y2 - y1;
            ax = (ax < 0) ? -ax : ax;
            ay = (ay < 0) ? ay : -ay;
            int32_t dx = ax, sx = x1 < x2 ? 1 : -1;
            int32_t dy = ay, sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy, e2;

            while (true) {
                set_pixel_a(x1, y1, red, green, blue, alpha);
                if (x1 == x2 && y1 == y2) break;
                e2 = 2 * err;
                if (e2 > dy){
                    err += dy;
                    x1 += sx;
                }
                if (e2 < dx){
                    err += dx;
                    y1 += sy;
                }
            }
        }

        //draws line between two points
        //no alpha
        void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t red, uint8_t green, uint8_t blue){
            if(!initialized)
                return;
            int32_t ax = x2 - x1, ay = y2 - y1;
            ax = (ax < 0) ? -ax : ax;
            ay = (ay < 0) ? ay : -ay;
            int32_t dx = ax, sx = x1 < x2 ? 1 : -1;
            int32_t dy = ay, sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy, e2;

            while (true) {
                set_pixel(x1, y1, red, green, blue);
                if (x1 == x2 && y1 == y2) break;
                e2 = 2 * err;
                if (e2 > dy){
                    err += dy;
                    x1 += sx;
                }
                if (e2 < dx){
                    err += dx;
                    y1 += sy;
                }
            }
        }

        //draws line between two points
        void line_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color::Color val){
            line_a(x1, y1, x2, y2, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws line between two points
        //no alpha
        void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color::Color val){
            line(x1, y1, x2, y2, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //sets every pixel of the image to rgba value
        void fill_a(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(!initialized)
                return;
            for(uint32_t i = 0; i < btmp_height; i++){
                for(uint32_t j = 0; j < btmp_width; j++){
                    set_pixel_a(j, i, red, green, blue, alpha);
                }
            }
        }

        //sets every pixel of the image to rgb value
        //no alpha
        void fill(uint8_t red, uint8_t green, uint8_t blue){
            if(!initialized)
                return;
            for(uint32_t i = 0; i < btmp_height; i++){
                for(uint32_t j = 0; j < btmp_width; j++){
                    set_pixel(j, i, red, green, blue);
                }
            }
        }

        //sets every pixel of the image to rgba value
        void fill_a(color::Color val){
            fill_a(color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //sets every pixel of the image to rgb value
        //no alpha
        void fill(color::Color val){
            fill(color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //sets an area of pixels, with the same color, to another color
        //like the bucket in paint if that makes sense
        void floodfill_a(uint32_t x_pos, uint32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(!initialized)
                return;

            uint8_t  // first we get the color value of the specified pixel
                old_red = get_pixel(x_pos, y_pos, 2),
                old_green = get_pixel(x_pos, y_pos, 1),
                old_blue = get_pixel(x_pos, y_pos, 0),
                old_alpha = get_pixel(x_pos, y_pos, 3);

            std::stack<uint32_t> pixels_to_fill; // create a stack to store the pixel coordinates
            pixels_to_fill.push(x_pos); // push current pixel coords into stack
            pixels_to_fill.push(y_pos);

            uint32_t x_buf = 0, y_buf = 0; // declare and initialize position buffers

            while(pixels_to_fill.size() > 0 && pixels_to_fill.size() < 100000000){ // while the stack isn't empty and doesnt have 100mil or more elements
                // read pixel coords into buffers
                x_buf = pixels_to_fill.top();
                pixels_to_fill.pop();
                y_buf = pixels_to_fill.top();
                pixels_to_fill.pop();

                if(get_pixel(x_buf, y_buf, 2) == old_red &&
                   get_pixel(x_buf, y_buf, 1) == old_green &&
                   get_pixel(x_buf, y_buf, 0) == old_blue &&
                   get_pixel(x_buf, y_buf, 3) == old_alpha){ // check if buffer has the original color

                    //if yes then replace with new color
                    set_pixel_a(x_buf, y_buf, red, green, blue, alpha);

                    //push neighboring pixels into the stack but check if they are out of bounds
                    if(x_buf > 0){
                        pixels_to_fill.push(x_buf - 1);
                        pixels_to_fill.push(y_buf);
                    }
                    if(x_buf < btmp_width - 1){
                        pixels_to_fill.push(x_buf + 1);
                        pixels_to_fill.push(y_buf);
                    }
                    if(y_buf > 0){
                        pixels_to_fill.push(x_buf);
                        pixels_to_fill.push(y_buf - 1);
                    }
                    if(y_buf < btmp_height - 1){
                        pixels_to_fill.push(x_buf);
                        pixels_to_fill.push(y_buf + 1);
                    }
                }
            }
        }

        //sets an area of pixels, with the same color, to another color
        //like the bucket in paint if that makes sense
        //no alpha value
        void floodfill(uint32_t x_pos, uint32_t y_pos, uint8_t red, uint8_t green, uint8_t blue){
            if(!initialized)
                return;

            uint8_t  // first we get the color value of the specified pixel
                old_red = get_pixel(x_pos, y_pos, 2),
                old_green = get_pixel(x_pos, y_pos, 1),
                old_blue = get_pixel(x_pos, y_pos, 0);

            std::stack<uint32_t> pixels_to_fill; // create a stack to store the pixel coordinates
            pixels_to_fill.push(x_pos); // push current pixel coords into stack
            pixels_to_fill.push(y_pos);

            uint32_t x_buf = 0, y_buf = 0; // declare and initialize position buffers

            while(pixels_to_fill.size() > 0 && pixels_to_fill.size() < 100000000){ // while the stack isn't empty and doesnt have 100mil or more elements
                // read pixel coords into buffers
                x_buf = pixels_to_fill.top();
                pixels_to_fill.pop();
                y_buf = pixels_to_fill.top();
                pixels_to_fill.pop();

                if(get_pixel(x_buf, y_buf, 2) == old_red &&
                   get_pixel(x_buf, y_buf, 1) == old_green &&
                   get_pixel(x_buf, y_buf, 0) == old_blue){ // check if buffer has the original color

                    //if yes then replace with new color
                    set_pixel(x_buf, y_buf, red, green, blue);

                    //push neighboring pixels into the stack but check if they are out of bounds
                    if(x_buf > 0){
                        pixels_to_fill.push(x_buf - 1);
                        pixels_to_fill.push(y_buf);
                    }
                    if(x_buf < btmp_width - 1){
                        pixels_to_fill.push(x_buf + 1);
                        pixels_to_fill.push(y_buf);
                    }
                    if(y_buf > 0){
                        pixels_to_fill.push(x_buf);
                        pixels_to_fill.push(y_buf - 1);
                    }
                    if(y_buf < btmp_height - 1){
                        pixels_to_fill.push(x_buf);
                        pixels_to_fill.push(y_buf + 1);
                    }
                }
            }
        }

        //sets an area of pixels, with the same color, to another color
        //like the bucket in paint if that makes sense
        void floodfill_a(uint32_t x_pos, uint32_t y_pos, color::Color col){
            floodfill_a(x_pos, y_pos, color::get_red(col), color::get_green(col), color::get_blue(col), color::get_alpha(col));
        }

        //sets an area of pixels, with the same color, to another color
        //like the bucket in paint if that makes sense
        //no alpha value
        void floodfill(uint32_t x_pos, uint32_t y_pos, color::Color col){
            floodfill(x_pos, y_pos, color::get_red(col), color::get_green(col), color::get_blue(col));
        }

        //draws a rectangle of given size
        void rectangle_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x1 > x2 || y1 > y2 || !initialized)
                return;
            for(uint32_t i = x1; i <= x2; i++){
                for(uint32_t j = y1; j <= y2; j++){
                    set_pixel_a(i, j, red, green, blue, alpha);
                }
            }
        }

        //draws a rectangle of given size
        //no alpha
        void rectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t red, uint8_t green, uint8_t blue){
            if(x1 > x2 || y1 > y2 || !initialized)
                return;
            for(uint32_t i = x1; i <= x2; i++){
                for(uint32_t j = y1; j <= y2; j++){
                    set_pixel(i, j, red, green, blue);
                }
            }
        }

        //draws a rectangle of given size
        void rectangle_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color::Color val){
            rectangle_a(x1, y1, x2, y2, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a rectangle of given size
        void rectangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color::Color val){
            rectangle(x1, y1, x2, y2, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws a border of given size
        void border_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t thickness, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x1 > x2 || y1 > y2 || !initialized)
                return;

            //vertical lines
            for(uint32_t j = y1; j <= y2; j++){
                //left
                for(int32_t i = x1; i <= x1 + thickness; i++){
                    set_pixel_a(i, j, red, green, blue, alpha);
                }
                //right
                for(int32_t i = x2 - thickness; i <= x2; i++){
                    set_pixel_a(i, j, red, green, blue, alpha);
                }
            }
            //horizontal lines
            for(int32_t i = x1 + thickness; i <= x2 - thickness; i++){
                //upper
                for(uint32_t j = y1; j <= y1 + thickness; j++){
                    set_pixel_a(i, j, red, green, blue, alpha);
                }
                //lower
                for(uint32_t j = y2 - thickness; j <= y2; j++){
                    set_pixel_a(i, j, red, green, blue, alpha);
                }
            }
        }

        //draws a border of given size
        //no alpha
        void border(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t thickness, uint8_t red, uint8_t green, uint8_t blue){
            if(x1 > x2 || y1 > y2 || !initialized)
                return;

            //vertical lines
            for(uint32_t j = y1; j <= y2; j++){
                //left
                for(int32_t i = x1; i <= x1 + thickness; i++){
                    set_pixel(i, j, red, green, blue);
                }
                //right
                for(int32_t i = x2 - thickness; i <= x2; i++){
                    set_pixel(i, j, red, green, blue);
                }
            }
            //horizontal lines
            for(int32_t i = x1 + thickness; i <= x2 - thickness; i++){
                //upper
                for(uint32_t j = y1; j <= y1 + thickness; j++){
                    set_pixel(i, j, red, green, blue);
                }
                //lower
                for(uint32_t j = y2 - thickness; j <= y2; j++){
                    set_pixel(i, j, red, green, blue);
                }
            }
        }

        //draws a border of given size
        void border_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t thickness, color::Color val){
            border_a(x1, y1, x2, y2, thickness, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a border of given size
        //no alpha
        void border(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t thickness, color::Color val){
            border(x1, y1, x2, y2, thickness, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws a circle of given size
        void circle_a(int32_t x_pos, int32_t y_pos, int32_t radius, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(radius < 1 || !initialized)
                return;
            for(int32_t i = -radius; i < radius; i++){
                for(int32_t j = -radius; j < radius; j++){
                    if(i * i + j * j <= radius * radius){
                        set_pixel_a(x_pos + i, y_pos + j, red, green, blue, alpha);
                    }
                }
            }
        }

        //draws a circle of given size
        //no alpha
        void circle(int32_t x_pos, int32_t y_pos, int32_t radius, uint8_t red, uint8_t green, uint8_t blue){
            if(radius < 1 || !initialized)
                return;
            for(int32_t i = -radius; i < radius; i++){
                for(int32_t j = -radius; j < radius; j++){
                    if(i * i + j * j <= radius * radius){
                        set_pixel(x_pos + i, y_pos + j, red, green, blue);
                    }
                }
            }
        }

        //draws a circle of given size
        void circle_a(int32_t x_pos, int32_t y_pos, int32_t radius, color::Color val){
            circle_a(x_pos, y_pos, radius, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a circle of given size
        //no alpha
        void circle(int32_t x_pos, int32_t y_pos, int32_t radius, color::Color val){
            circle(x_pos, y_pos, radius, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws an ellipse of given size
        void ellipse_a(int32_t x_pos, int32_t y_pos, int32_t radius, float x_mult, float y_mult, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(radius < 1 || !initialized)
                return;
            float x_mult_inv = 1 / x_mult;
            float y_mult_inv = 1 / y_mult;
            for(int32_t i = -radius; i < radius * x_mult; i++){
                for(int32_t j = -radius; j < radius * y_mult; j++){
                    if(x_mult_inv * i * i + y_mult_inv * j * j <= radius * radius){
                        set_pixel_a(x_pos + i, y_pos + j, red, green, blue, alpha);
                    }
                }
            }
        }

        //draws an ellipse of given size
        //no alpha
        void ellipse(int32_t x_pos, int32_t y_pos, int32_t radius, float x_mult, float y_mult, uint8_t red, uint8_t green, uint8_t blue){
            if(radius < 1 || !initialized)
                return;
            float x_mult_inv = 1 / x_mult;
            float y_mult_inv = 1 / y_mult;
            for(int32_t i = -radius * x_mult; i < radius * x_mult; i++){
                for(int32_t j = -radius * y_mult; j < radius * y_mult; j++){
                    if(x_mult_inv * i * i + y_mult_inv * j * j <= radius * radius){
                        set_pixel(x_pos + i, y_pos + j, red, green, blue);
                    }
                }
            }
        }

        //draws an ellipse of given size
        void ellipse_a(int32_t x_pos, int32_t y_pos, int32_t radius, float x_mult, float y_mult, color::Color val){
            ellipse_a(x_pos, y_pos, radius, x_mult, y_mult, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws an ellipse of given size
        //no alpha
        void ellipse(int32_t x_pos, int32_t y_pos, int32_t radius, float x_mult, float y_mult, color::Color val){
            ellipse(x_pos, y_pos, radius, x_mult, y_mult, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draw a sector of a circle, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        void circle_sector_a(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(radius < 1 || !initialized)
                return;
            //calculate constant to convert degrees into radians
            const double pi_over_180 = 3.14159265 / 180.0;
            set_pixel_a(x_pos, y_pos, red, green, blue, alpha);
            //iterate over all radii starting from 1 to and including the final radius
            for(uint32_t radius_iter = 1; radius_iter <= radius; radius_iter++){
                //calculate the angle difference and the length of the sectors outer diameter
                double angle_diff = std::abs(start_angle - end_angle);
                double pixel_arc_len = 3.14159265 * radius_iter * radius_iter * (360.0 / angle_diff);
                //use that to estimate a rough step size for the angle iterator
                double angle_step = angle_diff / pixel_arc_len;

                //then iterate over from the starting angle to the end angle
                //The point is calculated like this: P = (sin(a) * r | cos(a) * r)
                for(double angle_iter = start_angle; angle_iter <= end_angle; angle_iter += angle_step){
                    //cos is negated because in the image up means smaller y while in the coordinate system up means larger y
                    set_pixel_a(std::sin(angle_iter * pi_over_180) * radius_iter + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter + y_pos, red, green, blue, alpha);
                }
            }
        }

        //draw a sector of a circle, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        //no alpha
        void circle_sector(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, uint8_t red, uint8_t green, uint8_t blue){
            if(radius < 1 || !initialized)
                return;
            const double pi_over_180 = 3.14159265 / 180.0;
            set_pixel(x_pos, y_pos, red, green, blue);
            for(uint32_t radius_iter = 1; radius_iter <= radius; radius_iter++){
                double angle_diff = std::abs(start_angle - end_angle);
                double pixel_arc_len = 3.14159265 * radius_iter * radius_iter * (360.0 / angle_diff);
                double angle_step = angle_diff / pixel_arc_len;

                for(double angle_iter = start_angle; angle_iter <= end_angle; angle_iter += angle_step){
                    set_pixel(std::sin(angle_iter * pi_over_180) * radius_iter + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter + y_pos, red, green, blue);
                }
            }
        }

        //draw a sector of a circle, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        void circle_sector_a(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, color::Color val){
            circle_sector_a(x_pos, y_pos, radius, start_angle, end_angle, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draw a sector of a circle, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        //no alpha
        void circle_sector(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, color::Color val){
            circle_sector(x_pos, y_pos, radius, start_angle, end_angle, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draw a sector of an ellipse, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        void ellipse_sector_a(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, float x_mult, float y_mult, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(radius < 1 || !initialized)
                return;
            //calculate constant to convert degrees into radians
            const double pi_over_180 = 3.14159265 / 180.0;
            set_pixel_a(x_pos, y_pos, red, green, blue, alpha);
            //iterate over all radii starting from 1 to and including the final radius
            for(uint32_t radius_iter = 1; radius_iter <= radius; radius_iter++){
                //calculate the angle difference and the length of the sectors outer diameter
                double angle_diff = std::abs(start_angle - end_angle);
                double pixel_arc_len = 3.14159265 * radius_iter * radius_iter * (360.0 / angle_diff);
                //use that to estimate a rough step size for the angle iterator
                double angle_step = angle_diff / pixel_arc_len;

                //then iterate over from the starting angle to the end angle
                //The point is calculated like this: P = (sin(a) * r | cos(a) * r)
                for(double angle_iter = start_angle; angle_iter <= end_angle; angle_iter += angle_step){
                    //cos is negated because in the image up means smaller y while in the coordinate system up means larger y
                    set_pixel_a(std::sin(angle_iter * pi_over_180) * radius_iter * x_mult + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter * y_mult + y_pos, red, green, blue, alpha);
                }
            }
        }

        //draw a sector of an ellipse, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        //no alpha
        void ellipse_sector(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, float x_mult, float y_mult, uint8_t red, uint8_t green, uint8_t blue){
            if(radius < 1 || !initialized)
                return;
            const double pi_over_180 = 3.14159265 / 180.0;
            set_pixel(x_pos, y_pos, red, green, blue);
            //iterate over all radii from 1 to and including the final radius
            //by stretching the ellipse we simply make the radius larger and squish the circle down
            for(uint32_t radius_iter = 1; radius_iter <= radius * x_mult * y_mult; radius_iter++){
                double angle_diff = std::abs(start_angle - end_angle);
                double pixel_arc_len = 3.14159265 * radius_iter * radius_iter * (360.0 / angle_diff);
                double angle_step = angle_diff / pixel_arc_len;

                for(double angle_iter = start_angle; angle_iter <= end_angle; angle_iter += angle_step){
                    //divide the coordinates by the perpendicular coordinate stretch parameter to form the correct shape
                    set_pixel(std::sin(angle_iter * pi_over_180) * radius_iter / y_mult + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter / x_mult + y_pos, red, green, blue);
                }
            }
        }

        //draw a sector of an ellipse, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        void ellipse_sector_a(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, float x_mult, float y_mult, color::Color val){
            ellipse_sector_a(x_pos, y_pos, radius, start_angle, end_angle, x_mult, y_mult, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draw a sector of an ellipse, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        //no alpha
        void ellipse_sector(int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, float x_mult, float y_mult, color::Color val){
            ellipse_sector(x_pos, y_pos, radius, start_angle, end_angle, x_mult, y_mult, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws a ring of given size
        void ring_a(int32_t x_pos, int32_t y_pos, int32_t out_radius, int32_t in_radius, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(out_radius < 1 || in_radius < 0 || out_radius < in_radius || !initialized)
                return;
            for(int32_t i = -out_radius; i < out_radius; i++){
                for(int32_t j = -out_radius; j < out_radius; j++){
                    if(i * i + j * j <= out_radius * out_radius && i * i + j * j >= in_radius * in_radius){
                        set_pixel_a(x_pos + i, y_pos + j, red, green, blue, alpha);
                    }
                }
            }
        }

        //draws a ring of given size
        //no alpha
        void ring(int32_t x_pos, int32_t y_pos, int32_t out_radius, int32_t in_radius, uint8_t red, uint8_t green, uint8_t blue){
            if(out_radius < 1 || in_radius < 0 || out_radius < in_radius || !initialized)
                return;
            for(int32_t i = -out_radius; i < out_radius; i++){
                for(int32_t j = -out_radius; j < out_radius; j++){
                    if(i * i + j * j <= out_radius * out_radius && i * i + j * j >= in_radius * in_radius){
                        set_pixel(x_pos + i, y_pos + j, red, green, blue);
                    }
                }
            }
        }

        //draws a ring of given size
        void ring_a(int32_t x_pos, int32_t y_pos, int32_t out_radius, int32_t in_radius, color::Color val){
            ring_a(x_pos, y_pos, out_radius, in_radius, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a ring of given size
        //no alpha
        void ring(int32_t x_pos, int32_t y_pos, int32_t out_radius, int32_t in_radius, color::Color val){
            ring(x_pos, y_pos, out_radius, in_radius, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draw triangle
        void triangle_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(!initialized)
                return;
            int32_t ax = x2 - x1, ay = y2 - y1;
            ax = (ax < 0) ? -ax : ax;
            ay = (ay < 0) ? ay : -ay;
            int32_t dx = ax, sx = x1 < x2 ? 1 : -1;
            int32_t dy = ay, sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy, e2;

            line_a(x1, y1, x3, y3, red, green, blue, alpha);

            while (true) {
                if (x1 == x2 && y1 == y2) break;
                e2 = 2 * err;
                if (e2 > dy){
                    err += dy;
                    x1 += sx;
                    line_a(x1, y1, x3, y3, red, green, blue, alpha);
                }
                if (e2 < dx){
                    err += dx;
                    y1 += sy;
                    line_a(x1, y1, x3, y3, red, green, blue, alpha);
                }
            }
        }

        //draw triangle
        //no alpha
        void triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t red, uint8_t green, uint8_t blue){
            if(!initialized)
                return;
            int32_t ax = x2 - x1, ay = y2 - y1;
            ax = (ax < 0) ? -ax : ax;
            ay = (ay < 0) ? ay : -ay;
            int32_t dx = ax, sx = x1 < x2 ? 1 : -1;
            int32_t dy = ay, sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy, e2;

            line(x1, y1, x3, y3, red, green, blue);

            while (true) {
                if (x1 == x2 && y1 == y2) break;
                e2 = 2 * err;
                if (e2 > dy){
                    err += dy;
                    x1 += sx;
                    line(x1, y1, x3, y3, red, green, blue);
                }
                if (e2 < dx){
                    err += dx;
                    y1 += sy;
                    line(x1, y1, x3, y3, red, green, blue);
                }
            }
        }

        //draw triangle
        void triangle_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color::Color val){
            triangle_a(x1, y1, x2, y2, x3, y3, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draw triangle
        //no alpha
        void triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color::Color val){
            triangle(x1, y1, x2, y2, x3, y3, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draw triangle border by connecting three points with lines
        void triangle_border_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            line_a(x1, y1, x2, y2, red, green, blue, alpha);
            line_a(x2, y2, x3, y3, red, green, blue, alpha);
            line_a(x3, y3, x1, y1, red, green, blue, alpha);
        }

        //draw triangle border by connecting three points with lines
        //no alpha
        void triangle_border(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t red, uint8_t green, uint8_t blue){
            line(x1, y1, x2, y2, red, green, blue);
            line(x2, y2, x3, y3, red, green, blue);
            line(x3, y3, x1, y1, red, green, blue);
        }

        //draw triangle border by connecting three points with lines
        void triangle_border_a(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color::Color val){
            triangle_border_a(x1, y1, x2, y2, x3, y3, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draw triangle border by connecting three points with lines
        //no alpha
        void triangle_border(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color::Color val){
            triangle_border(x1, y1, x2, y2, x3, y3, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws a char from namespace chars
        //character bitmap
        void draw_char_a(int32_t x_pos, int32_t y_pos, uint16_t size, const chars::Charbtmp chr, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(!initialized)
                return;
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 5; j++){
                    if((chr[i] << (j + 3)) & 0b10000000) // offset of 3 because the pixel is stored in the last 5 bits of the char and not the first 5
                        rectangle_a(x_pos + j * size, y_pos + i * size, x_pos + (j + 1) * size - 1, y_pos + (i + 1) * size - 1, red, green, blue, alpha);
                }
            }
        }

        //draws a char from namespace chars
        //character bitmap
        //no alpha
        void draw_char(int32_t x_pos, int32_t y_pos, uint16_t size, const chars::Charbtmp chr, uint8_t red, uint8_t green, uint8_t blue){
            if(!initialized)
                return;
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 5; j++){
                    if((chr[i] << (j + 3)) & 0b10000000) // offset of 3 because the pixel is stored in the last 5 bits of the char and not the first 5
                        rectangle(x_pos + j * size, y_pos + i * size, x_pos + (j + 1) * size - 1, y_pos + (i + 1) * size - 1, red, green, blue);
                }
            }
        }

        //draws a char from namespace chars
        //character bitmap
        void draw_char_a(int32_t x_pos, int32_t y_pos, uint16_t size, const chars::Charbtmp chr, sbtmp::color::Color val){
            draw_char_a(x_pos, y_pos, size, chr, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a char from namespace chars
        //character bitmap
        //no alpha
        void draw_char(int32_t x_pos, int32_t y_pos, uint16_t size, const chars::Charbtmp chr, sbtmp::color::Color val){
            draw_char(x_pos, y_pos, size, chr, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws a char from namespace chars
        //char
        void draw_char_a(int32_t x_pos, int32_t y_pos, uint16_t size, char chr, sbtmp::color::Color val){
            draw_char_a(x_pos, y_pos, size, chars::asciitocbtmp(chr), color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a char from namespace chars
        //char
        //no alpha
        void draw_char(int32_t x_pos, int32_t y_pos, uint16_t size, char chr, sbtmp::color::Color val){
            draw_char(x_pos, y_pos, size, chars::asciitocbtmp(chr), color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //draws a string
        void draw_string_a(int32_t x_pos, int32_t y_pos, uint16_t size, const char *str, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(!initialized)
                return;
            int i = 0;
            while(*str){
                if(*str == '\n'){
                    i = 0;
                    y_pos += 9 * size;
                    str++;
                    continue;
                }
                if(*str == ' '){i++; str++; continue;}
                draw_char_a(x_pos + i * size * 6, y_pos, size, chars::asciitocbtmp(*str), red, green, blue, alpha);
                i++; str++;
            }
        }

        //draws a string
        //no alpha
        void draw_string(int32_t x_pos, int32_t y_pos, uint16_t size, const char *str, uint8_t red, uint8_t green, uint8_t blue){
            if(!initialized)
                return;
            int i = 0;
            while(*str){
                if(*str == '\n'){
                    i = 0;
                    y_pos += 9 * size;
                    str++;
                    continue;
                }
                if(*str == ' '){i++; str++; continue;}
                draw_char(x_pos + i * size * 6, y_pos, size, chars::asciitocbtmp(*str), red, green, blue);
                i++; str++;
            }
        }

        //draws a string
        void draw_string_a(int32_t x_pos, int32_t y_pos, uint16_t size, const char *str, color::Color val){
            draw_string_a(x_pos, y_pos, size, str, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //draws a string
        //no alpha
        void draw_string(int32_t x_pos, int32_t y_pos, uint16_t size, const char *str, color::Color val){
            draw_string(x_pos, y_pos, size, str, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //hide a secret text message inside the image using very simple steganography
        //the message can only have half as many characters as the image has pixels
        //because each pixel will contain 4 bits of the string -> 2 pixels = 1 char
        void encode_str(const char *str){
            if(!initialized)
                return;
            
            //get string length and include the NULL char
            uint32_t str_len = std::strlen(str) + 1;
            uint32_t data_index = 0;
            
            //escape condition
            bool esc_cond = true;

            while(esc_cond){

                uint8_t bit_mask = 0b10000000;
                bool str_bit;

                //this loop iterates over every bit of a char
                for(uint8_t i = 0; i < 8 && esc_cond; i++){
                    //get bit
                    str_bit = *str & bit_mask;

                    //encode bit
                    pixel_data[data_index] = ((str_bit) ? (pixel_data[data_index] | 0b00000001) : (pixel_data[data_index] & 0b11111110));

                    //shift mask
                    bit_mask >>= 1;

                    //increase index
                    data_index++;

                    //if end of array or end of string is reached, exit all loops by setting the condition to false
                    if(data_index >= raw_data_size || data_index >= str_len * 8)
                        esc_cond = false;
                }
                //go to the next char in the string
                str++;
            }
        }

        //read a hidden message from the image using steganography
        //string will automatically be cut of at any NULL character,
        //but this function can't detect the presence of a hidden message.
        //This means if there is nothing to decode then it will return garbage data
        const char *decode_str(){
            if(!initialized)
                return nullptr;
                
            //calculate and allocate the maximum possible string size
            //number of pixels / 2 -> 4 subpixels = 4 bits
            uint32_t str_len = btmp_height * btmp_width / 2;
            char *str_buffer = (char*)calloc(str_len, sizeof(char));

            //just a buffer pointing to one char of the string buffer
            char *str_ptr = (char*)str_buffer;

            //we start at index 0
            uint32_t data_index = 0;

            //escape condition
            bool esc_cond = true;

            while(esc_cond){
                //set char to NULL
                *str_ptr = '\0';
                
                //iterate over every bit of a char
                for(uint8_t i = 0; i < 8 && esc_cond; i++){
                    //extract char from least significant bit of the pixel data
                    *str_ptr |= ((0b00000001 & pixel_data[data_index]) << (7 - i));
                    
                    //go to next array index
                    data_index++;

                    //exit the loops if the end of the pixel data array is reached
                    if(data_index >= raw_data_size)
                        esc_cond = false;
                }
                //if the last extracted char is NULL then end the decoding process
                if(*str_ptr == '\0'){
                    //calculate actual stringsize
                    str_len = str_ptr - str_buffer;
                    esc_cond = false;
                }
                //go to the next char
                str_ptr++;
            }

            //reallocate the string buffer so that it isn't larger than it has to be
            str_buffer = (char*)realloc((void*)str_buffer, str_len);

            //finally return the decoded string
            return str_buffer;
        }

        //converts the image to black and white in the specified area
        void convert_bw(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
            if(x1 > btmp_width || y1 > btmp_height || x2 > btmp_width || y2 > btmp_height || !initialized)
                return;
            char bw_color;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    bw_color = (pixel_data[get_index(i, j)+0] + pixel_data[get_index(i, j)+1] + pixel_data[get_index(i, j)+2]) / 3;
                    pixel_data[get_index(i, j)+0] = bw_color;
                    pixel_data[get_index(i, j)+1] = bw_color;
                    pixel_data[get_index(i, j)+2] = bw_color;
                }
            }
        }

        //inverts the rgb values of the image in the specified area
        void rgb_invert(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
            if(x1 > x2 || y1 > y2 || x1 > btmp_width - 1 || y1 > btmp_height - 1 || x2 > btmp_width - 1 || y2 > btmp_height - 1 || !initialized)
                return;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    pixel_data[get_index(i, j)+0] = 255 - pixel_data[get_index(i, j)+0];
                    pixel_data[get_index(i, j)+1] = 255 - pixel_data[get_index(i, j)+1];
                    pixel_data[get_index(i, j)+2] = 255 - pixel_data[get_index(i, j)+2];
                }
            }
            return;
        }

        //inverts the rgba values of the image in the specified area
        void rgba_invert(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
            if(x1 > x2 || y1 > y2 || x1 > btmp_width - 1 || y1 > btmp_height - 1 || x2 > btmp_width - 1 || y2 > btmp_height - 1 || !initialized)
                return;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    pixel_data[get_index(i, j)+0] = 255 - pixel_data[get_index(i, j)+0];
                    pixel_data[get_index(i, j)+1] = 255 - pixel_data[get_index(i, j)+1];
                    pixel_data[get_index(i, j)+2] = 255 - pixel_data[get_index(i, j)+2];
                    pixel_data[get_index(i, j)+3] = 255 - pixel_data[get_index(i, j)+3];
                }
            }
            return;
        }

        //flips the image vertically
        void flip_vertical(){
            if(!initialized)
                return;
            for(uint32_t i = 0; i < btmp_height; i++){
                for(uint32_t j = 0; j < btmp_width / 2; j++){
                    //this is magic
                    std::swap(pixel_data[get_index(i, j)], pixel_data[get_index(i, btmp_width - j - 1)]);
                    std::swap(pixel_data[get_index(i, j)+1], pixel_data[get_index(i, btmp_width - j - 1)+1]);
                    std::swap(pixel_data[get_index(i, j)+2], pixel_data[get_index(i, btmp_width - j - 1)+2]);
                    std::swap(pixel_data[get_index(i, j)+3], pixel_data[get_index(i, btmp_width - j - 1)+3]);
                }
            }
        }

        //flips the image horizontally
        void flip_horizontal(){
            if(!initialized)
                return;
            for(uint32_t i = 0; i < btmp_height / 2; i++){
                for(uint32_t j = 0; j < btmp_width; j++){
                    //this is magic too
                    std::swap(pixel_data[get_index(i, j)], pixel_data[get_index(btmp_height - i - 1, j)]);
                    std::swap(pixel_data[get_index(i, j)+1], pixel_data[get_index(btmp_height - i - 1, j)+1]);
                    std::swap(pixel_data[get_index(i, j)+2], pixel_data[get_index(btmp_height - i - 1, j)+2]);
                    std::swap(pixel_data[get_index(i, j)+3], pixel_data[get_index(btmp_height - i - 1, j)+3]);
                }
            }
        }

        //blur the image
        void blur_a(){
            if(!initialized)
                return;

            Bitmap buffer(btmp_width, btmp_width);
            
            short red_buf, green_buf, blue_buf, alpha_buf;
            //Bitmap buff(width, height); //not needed yet
            for(uint32_t y = 1; y < btmp_height - 1; y++){
                for(uint32_t x = 1; x < btmp_width - 1; x++){
                    //oh boy is this baaaaaaad
                    red_buf   = get_pixel(x + 1, y, 2) + 
                                get_pixel(x + 1, y - 1, 2) +
                                get_pixel(x, y - 1, 2) +
                                get_pixel(x - 1, y - 1, 2) +
                                get_pixel(x - 1, y, 2) +
                                get_pixel(x - 1, y + 1, 2) +
                                get_pixel(x, y + 1, 2) +
                                get_pixel(x + 1, y + 1, 2);
                    green_buf = get_pixel(x + 1, y, 1) +
                                get_pixel(x + 1, y - 1, 1) +
                                get_pixel(x, y - 1, 1) +
                                get_pixel(x - 1, y - 1, 1) +
                                get_pixel(x - 1, y, 1) +
                                get_pixel(x - 1, y + 1, 1) +
                                get_pixel(x, y + 1, 1) +
                                get_pixel(x + 1, y + 1, 1);
                    blue_buf  = get_pixel(x + 1, y, 0) +
                                get_pixel(x + 1, y - 1, 0) +
                                get_pixel(x, y - 1, 0) +
                                get_pixel(x - 1, y - 1, 0) +
                                get_pixel(x - 1, y, 0) +
                                get_pixel(x - 1, y + 1, 0) +
                                get_pixel(x, y + 1, 0) +
                                get_pixel(x + 1, y + 1, 0);
                    alpha_buf = get_pixel(x + 1, y, 3) +
                                get_pixel(x + 1, y - 1, 3) +
                                get_pixel(x, y - 1, 3) +
                                get_pixel(x - 1, y - 1, 3) +
                                get_pixel(x - 1, y, 3) +
                                get_pixel(x - 1, y + 1, 3) +
                                get_pixel(x, y + 1, 3) +
                                get_pixel(x + 1, y + 1, 3);

                    buffer.set_pixel_a(x, y, red_buf/8, green_buf/8, blue_buf/8, alpha_buf/8);
                }
            }

            for(uint32_t y = 0; y < btmp_height; y++){
                for(uint32_t x = 0; x < btmp_width; x++){
                    set_pixel_a(x, y, buffer.get_pixel(x, y));
                }
            }
            buffer.del();
        }

        //blur the image
        //no alpha
        void blur(){
            if(!initialized)
                return;
            
            Bitmap buffer(btmp_width, btmp_width);
            
            short red_buf, green_buf, blue_buf;
            //Bitmap buff(width, height); //not needed yet
            for(uint32_t y = 1; y < btmp_height - 1; y++){
                for(uint32_t x = 1; x < btmp_width - 1; x++){
                    //oh boy is this baaaaaaad
                    red_buf   = get_pixel(x + 1, y, 2) + 
                                get_pixel(x + 1, y - 1, 2) +
                                get_pixel(x, y - 1, 2) +
                                get_pixel(x - 1, y - 1, 2) +
                                get_pixel(x - 1, y, 2) +
                                get_pixel(x - 1, y + 1, 2) +
                                get_pixel(x, y + 1, 2) +
                                get_pixel(x + 1, y + 1, 2);
                    green_buf = get_pixel(x + 1, y, 1) +
                                get_pixel(x + 1, y - 1, 1) +
                                get_pixel(x, y - 1, 1) +
                                get_pixel(x - 1, y - 1, 1) +
                                get_pixel(x - 1, y, 1) +
                                get_pixel(x - 1, y + 1, 1) +
                                get_pixel(x, y + 1, 1) +
                                get_pixel(x + 1, y + 1, 1);
                    blue_buf  = get_pixel(x + 1, y, 0) +
                                get_pixel(x + 1, y - 1, 0) +
                                get_pixel(x, y - 1, 0) +
                                get_pixel(x - 1, y - 1, 0) +
                                get_pixel(x - 1, y, 0) +
                                get_pixel(x - 1, y + 1, 0) +
                                get_pixel(x, y + 1, 0) +
                                get_pixel(x + 1, y + 1, 0);

                    buffer.set_pixel(x, y, red_buf/8, green_buf/8, blue_buf/8);
                }
            }

            for(uint32_t y = 0; y < btmp_height; y++){
                for(uint32_t x = 0; x < btmp_width; x++){
                    set_pixel(x, y, buffer.get_pixel(x, y));
                }
            }
            buffer.del();
        }

        #ifdef sbtmp_experimental
        void merge(Bitmap& other){
            if(other.get_width() != btmp_width || other.get_height() != btmp_height){
                return;
            }

            for(uint32_t i = 0; i < btmp_width; i++){
                for(uint32_t j = 0; j < btmp_height; j++){
                    if(other.get_pixel(i, j, 1) != 0 && other.get_pixel(i, j, 2) != 0 && other.get_pixel(i, j, 3) != 0){
                        set_pixel(i, j, other.get_pixel(i, j, 1), other.get_pixel(i, j, 2), other.get_pixel(i, j, 3), other.get_pixel(i, j, 0));
                    }
                }
            }
        }
        #endif

        //adds rgba value to specified pixel
        void addpixel_a(int32_t x_pos, int32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] += blue;
            pixel_data[get_index(x_pos, y_pos)+1] += green;
            pixel_data[get_index(x_pos, y_pos)+2] += red;
            pixel_data[get_index(x_pos, y_pos)+3] += alpha;
        }

        //adds rgb value to specified pixel
        void addpixel(int32_t x_pos, int32_t y_pos, uint8_t red, uint8_t green, uint8_t blue){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] += blue;
            pixel_data[get_index(x_pos, y_pos)+1] += green;
            pixel_data[get_index(x_pos, y_pos)+2] += red;
            pixel_data[get_index(x_pos, y_pos)+3] = 255;
        }

        //adds rgba value to specified pixel
        void addpixel_a(int32_t x_pos, int32_t y_pos, color::Color val){
            addpixel_a(x_pos, x_pos, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //adds rgb value to specified pixel
        void addpixel(int32_t x_pos, int32_t y_pos, color::Color val){
            addpixel(x_pos, x_pos, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //subtracts rgba value from specified pixel
        void subpixel_a(int32_t x_pos, int32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] -= blue;
            pixel_data[get_index(x_pos, y_pos)+1] -= green;
            pixel_data[get_index(x_pos, y_pos)+2] -= red;
            pixel_data[get_index(x_pos, y_pos)+3] -= alpha;
        }

        //subtracts rgb value from specified pixel
        void subpixel(int32_t x_pos, int32_t y_pos, uint8_t red, uint8_t green, uint8_t blue){
            if (x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] -= blue;
            pixel_data[get_index(x_pos, y_pos)+1] -= green;
            pixel_data[get_index(x_pos, y_pos)+2] -= red;
            pixel_data[get_index(x_pos, y_pos)+3] = 255;
        }

        //subtracts rgba value from specified pixel
        void subpixel_a(int32_t x_pos, int32_t y_pos, color::Color val){
            subpixel_a(x_pos, x_pos, color::get_red(val), color::get_green(val), color::get_blue(val), color::get_alpha(val));
        }

        //subtracts rgb value from specified pixel
        void subpixel(uint32_t x_pos, uint32_t y_pos, color::Color val){
            subpixel(x_pos, x_pos, color::get_red(val), color::get_green(val), color::get_blue(val));
        }

        //return the r-g-b-a value of specified pixel (3rd param: 0=b, 1=g, 2=r, 3=a)
        uint8_t get_pixel(int32_t x_pos, int32_t y_pos, uint8_t color_index){
            if (color_index > 3 || x_pos > btmp_width - 1 || y_pos > btmp_height - 1 || x_pos < 0 || y_pos < 0 || !initialized)
                return 0;
            return pixel_data[get_index(x_pos, y_pos) + color_index];
        }

        color::Color get_pixel(int32_t x_pos, int32_t y_pos){
            return get_pixel(x_pos, y_pos, 2) << 8 | get_pixel(x_pos, y_pos, 1) << 16 | get_pixel(x_pos, y_pos, 0) << 24 | get_pixel(x_pos, y_pos, 3);
        }

        //changes the size of the image
        void resize(uint32_t set_width, uint32_t set_height){
            if(!initialized)
                return;
            if(set_width == btmp_width && set_height == btmp_height) // args are the same size as image
                return;
            //if(set_width < btmp_width || set_height < btmp_height) // if args are smaller
            //    return;

            btmp_width = set_width;
            btmp_height = set_height;

            total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 4; // recalculate size attribs
            raw_data_size = btmp_height * btmp_width * 4;

            pixel_data = (uint8_t*)realloc(pixel_data, raw_data_size); // realloc bitmap data
        }

        //clears the image
        void clear(){
            if(!initialized)
                return;
            for(uint64_t i = 0; i < raw_data_size; i++){
                pixel_data[i] = 0;
            }
        }

        //frees the memory of the image and resets all properties
        void del(){
            if(!initialized)
                return;

            //reset all attribs
            btmp_width = 0;
            btmp_height = 0;
            total_size_in_bytes = 0;
            raw_data_size = 0;

            //pixel_data = (uint8_t*)realloc(pixel_data, 0); //what is this shit?
            free(pixel_data); // much better

            // image is not initialized anymore and can be reinitialized
            initialized = false;
        }

        //saves the image with given filename
        bool save(const char * filename){
            if(!initialized)
                return false;
            std::ofstream out_image;
            out_image.open(filename, std::ios::binary);

            //write file header
            out_image.write((char *)&ID_f1, 1);
            out_image.write((char *)&ID_f2, 1);
            out_image.write((char *)&total_size_in_bytes, 4);
            out_image.write((char *)&unused, 2);
            out_image.write((char *)&unused, 2);
            out_image.write((char *)&pixel_data_offset, 4);
            out_image.write((char *)&DIB_header_size, 4);
            out_image.write((char *)&btmp_width, 4);
            out_image.write((char *)&btmp_height, 4);
            out_image.write((char *)&color_planes, 2);
            out_image.write((char *)&bits_per_pixel, 2);
            out_image.write((char *)&Bl_RGB, 4);
            out_image.write((char *)&raw_data_size, 4);
            out_image.write((char *)&DPI_hor, 4);
            out_image.write((char *)&DPI_ver, 4);
            out_image.write((char *)&color_palette, 4);
            out_image.write((char *)&imp_colors, 4);
            //missing header data(added since ver exp 0.34)
            out_image.write((char *)&red_channel_bit_mask, 4);
            out_image.write((char *)&green_channel_bit_mask, 4);
            out_image.write((char *)&blue_channel_bit_mask, 4);
            out_image.write((char *)&alpha_channel_bit_mask, 4);
            out_image.write((char *)color_space, 4);
            out_image.write((char *)color_space_endpoints, 36);
            out_image.write((char *)gamma_rgb, 12);

            //much better way to write pixel data(since ver exp 0.27)
            out_image.write((char *)pixel_data, raw_data_size);

            out_image.close();

            return true;
        }

        // Load *.bmp images
        // Might crash when trying to load a file that doesn't conform to *this* standard. Most errors are caught, but still.
        bool load(const char * filename){
            if(initialized)
                return false;

            std::ifstream in_image;
            in_image.open(filename, std::ios::binary);

            //"BM" header
            char bm_header[2];
            //total size in bytes
            uint32_t size_bytes;
            //pixel data offset
            uint32_t offset;
            //bits per pixel
            uint16_t b_per_pix;

            //read file header

            //check if file starts with "BM"
            in_image.read((char *)&bm_header, 2);
            if(bm_header[0] != 'B' || bm_header[1] != 'M')
                return false;
            
            //check if file size is correct
            in_image.read((char *)&size_bytes, 4);
            total_size_in_bytes = size_bytes; //unnecessary step (may be removed)
            //std::cout << size_bytes << '\n';
            //std::cout << in_image.tellg() << '\n';
            in_image.seekg(0, std::ios::end); //jump to end of file
            //std::cout << in_image.tellg() << '\n';
            if(size_bytes != in_image.tellg())
                return false;
            in_image.seekg(6); //return to original location

            //read and check image parameters
            in_image.ignore(4); //2 * unused(uint16_t) = 4 bytes
            in_image.read((char *)&offset, 4);
            //std::cout << offset << '\n';
            in_image.ignore(4); // ignore dib header size
            in_image.read((char *)&btmp_width, 4);
            //std::cout << btmp_width << '\n';
            in_image.read((char *)&btmp_height, 4);
            //std::cout << btmp_height << '\n';
            in_image.ignore(2); //ignore color planes
            in_image.read((char *)&b_per_pix, 2);
            //std::cout << b_per_pix << '\n';
            if(b_per_pix != 32)
                return false;
            in_image.ignore(4); //ignore Bl_RGB

            //check if raw data size is correct and if not correct it
            in_image.read((char *)&raw_data_size, 4);
            if(raw_data_size != total_size_in_bytes - offset)
                raw_data_size = total_size_in_bytes - offset;

            //jump to image data location
            in_image.seekg(offset);

            //allocate memory and load the image data
            pixel_data = (uint8_t*)calloc (raw_data_size, sizeof(uint8_t));
            in_image.read((char *)&*pixel_data, raw_data_size);

            in_image.close();

            initialized = true;

            return true;
        }

        //returns pointer (maybe? -> Yes it does! 16.01.2022)
        //Be carefull with this function
        //Never free it!!
        uint8_t * ptr(){
            return pixel_data;
        }



        //if std::string is included, the save function can be called with a string instead of a char array
        #if defined(_GLIBCXX_STRING_) //gnu gcc compiler (Linux)
            bool save(std::string filename){
                return save(filename.c_str());
            }
        #elif defined(_STRING_) //msvc / llvm compiler (Windows)
            bool save(std::string filename){
                return save(filename.c_str());
            }
        #endif


        private:
        //BMP header
        const char ID_f1 = 'B', ID_f2 = 'M';
        uint32_t total_size_in_bytes;
        const uint16_t unused = 0;
        //const uint32_t pixel_data_offset = 54;
        const uint32_t pixel_data_offset = 122;

        //DIB header
        //const uint32_t DIB_header_size = 40;
        const uint32_t DIB_header_size = 108;
        uint32_t btmp_width, btmp_height;
        const uint16_t color_planes = 1;
        const uint16_t bits_per_pixel = 32;
        const uint32_t Bl_RGB = 3;
        uint32_t raw_data_size;
        const uint32_t DPI_hor = 2835, DPI_ver = 2835;
        const uint32_t color_palette = 0;
        const uint32_t imp_colors = 0;
        //missing header data(added since ver exp 0.34)
        const uint32_t red_channel_bit_mask = 0x00ff0000;
        const uint32_t green_channel_bit_mask = 0x0000ff00;
        const uint32_t blue_channel_bit_mask = 0x000000ff;
        const uint32_t alpha_channel_bit_mask = 0xff000000;
        const char color_space[4] = {' ', 'n', 'i', 'W'};
        const uint8_t color_space_endpoints[36] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        const uint32_t gamma_rgb[12] = {0};

        //const uint8_t padding = 0;
        //uint8_t padding_size;
        //uint8_t * pixel_data = (uint8_t*)malloc (0);
        uint8_t * pixel_data = nullptr; //is this the same as above? (future me: It is better. The above is total junk)
        bool initialized = false;

        //function used to get the array index of any pixel
        uint64_t get_index(uint32_t x_pos, uint32_t y_pos){
            return ((btmp_height - y_pos - 1) * btmp_width + x_pos) * 4; //y_pos is inverted because of the way the image is stored
        }
    };
}