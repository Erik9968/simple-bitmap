/*
 *  Simple Bitmap 2.0 by Erik S. ver exp. 0.63
 *  (2.0 is part of the name and doesn't refer to the actual product version)
 *  
 *  A library designed to be as simple as possible while providing enough functionality to be useful
 *  enough for most basic applications.
 *  
 *  Types currently supported:
 *      -32bit Bitmap
 *      -24bit Bitmap
 *  
 *  Planned types:
 *      -16bit Bitmap
 *      -8bit Bitmap
 *      -PPM
 *      -PGM
 *      -PBM
 *      -(Unlikely)Uncompressed PNG32 and/or 24, 16, 8
 *  
 *  This is the second (technically third) version of my Bitmap library.
 *  It tries to make expanding a simple bitmap easier. The old simple bitmap or simple bitmap 1.0
 *  could only handle 32 bit bitmaps, and it was not possible to expand it to other bit formats
 *  without introducing some serious changes to basically the entire class, it was defined in.
 *  Simple bitmap 2.0 does not have this issue. It defines image types based on a main "image"
 *  class, that other formats inherit from. This makes it possible to define all graphics and
 *  filter functions just once. Any image type only needs to define/overwrite a few standards
 *  functions from the main class and that's it.
 *  
 *  This library is still in an experimental state and you should absolutely expect crashes. 
 *  Creating, editing and saving images is usually pretty safe, but loading ones not necessarily.
 *  
 *  Changelog:
 *  
 *  <= 0.57
 *      -simple bitmap 1.0
 *  
 *  -0.58
 *      -created main class
 *      -converted graphics methods into standalone functions
 *      -temporarily removed blur functions
 *  
 *  -0.59
 *      -updated main class
 *      -created Bitmap32 class
 *  
 *  -0.60
 *      -updated main class
 *      -finished Bitmap32
 *  
 *  -0.61
 *      -created Bitmap24
 *      -added all graphics functions from 1.0
 *  
 *  -0.62
 *      -fixed bug in Bitmap24 save function
 *  
 *  -0.63
 *      -fixed bug in the round_rectangle function (if radius was to small, the corners would not be drawn -> added min radius)
 *  
 */




#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <stack>
#include <cmath>


namespace sbtmp {
    namespace color {
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

        inline Color blackNwhite(Color col){
            uint8_t bw = (get_red(col) + get_green(col) + get_blue(col)) / 3;
            Color out;

            set_alpha(out, get_alpha(col));
            set_red(out, bw);
            set_green(out, bw);
            set_blue(out, bw);

            return out;
        }

        inline Color invert(Color col){
            Color out = col;
            set_red(out, 255 - get_red(col));
            set_green(out, 255 - get_green(col));
            set_blue(out, 255 - get_blue(col));
            return out;
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

    namespace chars {

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

    namespace base{
        class image{
            public:
            virtual bool save(const char *filename){return false;}; //saves the image to a physical file
            virtual bool load(const char *filename){return false;}; //loads image data from a physical file
            virtual void create(uint32_t length, uint32_t height){return;}; //initializes the image
            virtual void set_pixel(int32_t x, int32_t y, color::Color col){return;}; //sets a pixel to a color
            virtual color::Color get_pixel(int32_t x, int32_t y){return 0;}; //returns the pixel of a color
            virtual uint32_t get_width(){return 0;}; //returns the width of the image
            virtual uint32_t get_height(){return 0;}; //returns the height of the image
            virtual size_t get_raw_size(){return 0;}; //returns the total size of the pixel data in bytes
            virtual bool is_initialized(){return false;}; //returns boolean to check if the image has been initialized
            virtual void resize(uint32_t width, uint32_t height){return;}; //resizes the image
            virtual void clear(){return;}; //clears the image (makes it blank)
            virtual void del(){return;}; //frees all image data and resets all attribute (basically uninitialize an image)
            virtual uint8_t *data(){return nullptr;}; //returns the pointer to the actual image data

            //you may add more functions if you wish but these are the functions you must implement!
        };
    }

    namespace formats {
        class Bitmap32 : public base::image{
            public:

            //constructor
            Bitmap32(uint32_t set_width, uint32_t set_height) {
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
            Bitmap32(Bitmap32 &other){
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
            Bitmap32() = default;

            //destructor
            ~Bitmap32(){
                // free data to prevent memory leak
                if(!pixel_data)
                    free(pixel_data);
            }

            //saves the image with given filename
            bool save(const char * filename) override {
                if(!initialized)
                    return false;
                std::ofstream out_image;
                out_image.open(filename, std::ios::binary);

                //write file header
                out_image.write((char*)&ID_f1, 1);
                out_image.write((char*)&ID_f2, 1);
                out_image.write((char*)&total_size_in_bytes, 4);
                out_image.write((char*)&unused, 2);
                out_image.write((char*)&unused, 2);
                out_image.write((char*)&pixel_data_offset, 4);
                out_image.write((char*)&DIB_header_size, 4);
                out_image.write((char*)&btmp_width, 4);
                out_image.write((char*)&btmp_height, 4);
                out_image.write((char*)&color_planes, 2);
                out_image.write((char*)&bits_per_pixel, 2);
                out_image.write((char*)&Bl_RGB, 4);
                out_image.write((char*)&raw_data_size, 4);
                out_image.write((char*)&DPI_hor, 4);
                out_image.write((char*)&DPI_ver, 4);
                out_image.write((char*)&color_palette, 4);
                out_image.write((char*)&imp_colors, 4);
                //missing header data(added since ver exp 0.34)
                out_image.write((char*)&red_channel_bit_mask, 4);
                out_image.write((char*)&green_channel_bit_mask, 4);
                out_image.write((char*)&blue_channel_bit_mask, 4);
                out_image.write((char*)&alpha_channel_bit_mask, 4);
                out_image.write((char*)color_space, 4);
                out_image.write((char*)color_space_endpoints, 36);
                out_image.write((char*)gamma_rgb, 12);

                //much better way to write pixel data(since ver exp 0.27)
                out_image.write((char*)pixel_data, raw_data_size);

                out_image.close();

                return true;
            }

            // Load *.bmp images
            // Might crash when trying to load a file that doesn't conform to *this* standard. Most errors are caught, but still.
            bool load(const char * filename) override {
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
                in_image.read((char*)&bm_header, 2);
                if(bm_header[0] != 'B' || bm_header[1] != 'M')
                    return false;
                
                //check if file size is correct
                in_image.read((char*)&size_bytes, 4);
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
                in_image.read((char*)&offset, 4);
                //std::cout << offset << '\n';
                in_image.ignore(4); // ignore dib header size
                in_image.read((char*)&btmp_width, 4);
                //std::cout << btmp_width << '\n';
                in_image.read((char*)&btmp_height, 4);
                //std::cout << btmp_height << '\n';
                in_image.ignore(2); //ignore color planes
                in_image.read((char*)&b_per_pix, 2);
                //std::cout << b_per_pix << '\n';
                if(b_per_pix != 32)
                    return false;
                in_image.ignore(4); //ignore Bl_RGB

                //check if raw data size is correct and if not correct it
                in_image.read((char*)&raw_data_size, 4);
                if(raw_data_size != total_size_in_bytes - offset)
                    raw_data_size = total_size_in_bytes - offset;

                //jump to image data location
                in_image.seekg(offset);

                //allocate memory and load the image data
                pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));
                in_image.read((char*)&*pixel_data, raw_data_size);

                in_image.close();

                initialized = true;

                return true;
            }

            //create function (recommended way to init images)
            void create(uint32_t set_width, uint32_t set_height) override {
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

            //set pixel at coords x, y to rgb value
            void set_pixel(int32_t x, int32_t y, color::Color col) override{
                if (!initialized || x > btmp_width - 1 || y > btmp_height - 1 || x < 0 || y < 0)
                    return;
                pixel_data[get_p_index(x, y)+0] = color::get_blue(col);
                pixel_data[get_p_index(x, y)+1] = color::get_green(col);
                pixel_data[get_p_index(x, y)+2] = color::get_red(col);
                pixel_data[get_p_index(x, y)+3] = color::get_alpha(col);
            }

            //get color of pixel at coords x, y
            color::Color get_pixel(int32_t x, int32_t y) override {
                return pixel_data[get_p_index(x, y) + 2] << 8 | pixel_data[get_p_index(x, y) + 1] << 16 | pixel_data[get_p_index(x, y)] << 24 | pixel_data[get_p_index(x, y) + 3];
            }

            //return width of the image
            uint32_t get_width() override {
                return btmp_width;
            }

            //returns height of the image
            uint32_t get_height() override {
                return btmp_height;
            }

            size_t get_raw_size() override {
                return raw_data_size;
            }

            //changes the size of the image
            void resize(uint32_t width, uint32_t height) override {
                if(!initialized)
                    return;
                if(width == btmp_width && height == btmp_height) // args are the same size as image
                    return;
                //if(set_width < btmp_width || set_height < btmp_height) // if args are smaller
                //    return;

                btmp_width = width;
                btmp_height = height;

                total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 4; // recalculate size attribs
                raw_data_size = btmp_height * btmp_width * 4;

                pixel_data = (uint8_t*)realloc(pixel_data, raw_data_size); // realloc bitmap data
            }

            //clears the image
            void clear() override {
                if(!initialized)
                    return;
                memset(pixel_data, 0, raw_data_size);
            }

            //frees the memory of the image and resets all properties
            void del() override {
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

            bool is_initialized() override {
                return initialized;
            }


            private:

            //just two little helper function
            //used to get the array index of any pixel/byte
            size_t get_p_index(uint32_t x_pos, uint32_t y_pos){
                //pixel index
                return ((btmp_height - y_pos - 1) * btmp_width + x_pos) * 4; //y_pos is inverted because of the way the image is stored
            }
            size_t get_r_index(uint32_t x_pos, uint32_t y_pos){
                //raw index
                return (y_pos * btmp_width * 4 + x_pos);
            }

            //BMP header
            const char ID_f1 = 'B', ID_f2 = 'M';
            uint32_t total_size_in_bytes;
            const uint16_t unused = 0;
            const uint32_t pixel_data_offset = 122;

            //DIB header
            const uint32_t DIB_header_size = 108;
            uint32_t btmp_width, btmp_height;
            const uint16_t color_planes = 1;
            const uint16_t bits_per_pixel = 32;
            const uint32_t Bl_RGB = 3;
            uint32_t raw_data_size;
            const uint32_t DPI_hor = 2835, DPI_ver = 2835;
            const uint32_t color_palette = 0;
            const uint32_t imp_colors = 0;
            //additional information needed for 32 bit bitmaps
            const uint32_t red_channel_bit_mask = 0x00ff0000;
            const uint32_t green_channel_bit_mask = 0x0000ff00;
            const uint32_t blue_channel_bit_mask = 0x000000ff;
            const uint32_t alpha_channel_bit_mask = 0xff000000;
            const char color_space[4] = {' ', 'n', 'i', 'W'};
            const uint8_t color_space_endpoints[36] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            const uint32_t gamma_rgb[12] = {0};

            uint8_t * pixel_data = nullptr;
            bool initialized = false;
        };

        class Bitmap24 : public base::image{
            public:

            //constructor
            Bitmap24(uint32_t set_width, uint32_t set_height) {
                if(initialized)
                    return;

                btmp_width = set_width;
                btmp_height = set_height;

                total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 3;
                raw_data_size = btmp_height * btmp_width * 3;

                //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
                pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));

                initialized = true;
            }

            //copy constructor
            //creates a perfect copy of the original image
            //it also copies emtpy images by first freeing all data
            Bitmap24(Bitmap24 &other){
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
            Bitmap24() = default;

            //destructor
            ~Bitmap24(){
                // free data to prevent memory leak
                if(!pixel_data)
                    free(pixel_data);
            }

            //saves the image with given filename
            bool save(const char * filename) override {
                if(!initialized)
                    return false;
                std::ofstream out_image;
                out_image.open(filename, std::ios::binary);

                //write file header
                out_image.write((char*)&ID_f1, 1);
                out_image.write((char*)&ID_f2, 1);
                out_image.write((char*)&total_size_in_bytes, 4);
                out_image.write((char*)&unused, 2);
                out_image.write((char*)&unused, 2);
                out_image.write((char*)&pixel_data_offset, 4);
                out_image.write((char*)&DIB_header_size, 4);
                out_image.write((char*)&btmp_width, 4);
                out_image.write((char*)&btmp_height, 4);
                out_image.write((char*)&color_planes, 2);
                out_image.write((char*)&bits_per_pixel, 2);
                out_image.write((char*)&Bl_RGB, 4);
                out_image.write((char*)&raw_data_size, 4);
                out_image.write((char*)&DPI_hor, 4);
                out_image.write((char*)&DPI_ver, 4);
                out_image.write((char*)&color_palette, 4);
                out_image.write((char*)&imp_colors, 4);

                //pad(x) = (4 - (x mod 4)) mod 4
                //the extra mod at the end prevents a padding size of 4
                //I don't know how to explain, just test it with a calculator
                uint8_t padding_size = (4 - (btmp_width % 4)) % 4;
                char pad_val = 0;

                //much better way to write pixel data(since ver exp 0.27)
                //out_image.write((char*)pixel_data, raw_data_size);
                for(size_t y = 0; y < btmp_height; y++){
                    for(size_t x = 0; x < btmp_width * 3; x++){
                        out_image.write((char*)&pixel_data[get_r_index(x, y)], 1);
                    }
                    for(uint8_t i = 0; i < padding_size; i++){
                        out_image.write(&pad_val, 1);
                    }
                }

                out_image.close();

                return true;
            }

            // Load *.bmp images
            // Might crash when trying to load a file that doesn't conform to *this* standard. Most errors are caught, but still.
            bool load(const char * filename) override {
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
                in_image.read((char*)&bm_header, 2);
                if(bm_header[0] != 'B' || bm_header[1] != 'M')
                    return false;
                
                //check if file size is correct
                in_image.read((char*)&size_bytes, 4);
                total_size_in_bytes = size_bytes; //unnecessary step (may be removed)
                in_image.seekg(0, std::ios::end); //jump to end of file
                if(size_bytes != in_image.tellg())
                    return false;
                in_image.seekg(6); //return to original location

                //read and check image parameters
                in_image.ignore(4); //2 * unused(uint16_t) = 4 bytes
                in_image.read((char*)&offset, 4);
                in_image.ignore(4); // ignore dib header size
                in_image.read((char*)&btmp_width, 4);
                in_image.read((char*)&btmp_height, 4);
                in_image.ignore(2); //ignore color planes
                in_image.read((char*)&b_per_pix, 2);
                if(b_per_pix != 24)
                    return false;
                in_image.ignore(4); //ignore Bl_RGB
                in_image.read((char*)&raw_data_size, 4);

                //jump to image data location
                in_image.seekg(offset);

                //allocate memory and load the image data
                pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));

                //pad(x) = (4 - (x mod 4)) mod 4
                //the extra mod at the end prevents a padding size of 4
                //I don't know how to explain, just test it with a calculator
                uint8_t padding_size = (4 - (raw_data_size % 4)) % 4;

                //much better way to write pixel data(since ver exp 0.27)
                //out_image.write((char*)pixel_data, raw_data_size);
                for(size_t y = 0; y < btmp_height; y++){
                    for(size_t x = 0; x < btmp_width * 3; x++){
                        in_image.read((char*)&pixel_data[get_r_index(x, y)], 1);
                    }
                    in_image.ignore(padding_size);
                }

                in_image.close();

                initialized = true;

                return true;
            }

            //create function (recommended way to init images)
            void create(uint32_t set_width, uint32_t set_height) override {
                if(initialized)
                    return;

                btmp_width = set_width;
                btmp_height = set_height;

                total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 3;
                raw_data_size = btmp_height * btmp_width * 3;

                //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
                pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));

                initialized = true;
            }

            //set pixel at coords x, y to rgb value
            void set_pixel(int32_t x, int32_t y, color::Color col) override{
                if (!initialized || x > btmp_width - 1 || y > btmp_height - 1 || x < 0 || y < 0)
                    return;
                pixel_data[get_p_index(x, y)+0] = color::get_blue(col);
                pixel_data[get_p_index(x, y)+1] = color::get_green(col);
                pixel_data[get_p_index(x, y)+2] = color::get_red(col);
            }

            //get color of pixel at coords x, y
            color::Color get_pixel(int32_t x, int32_t y) override {
                return pixel_data[get_p_index(x, y) + 2] << 8 | pixel_data[get_p_index(x, y) + 1] << 16 | pixel_data[get_p_index(x, y)] << 24 | 0;
            }

            //return width of the image
            uint32_t get_width() override {
                return btmp_width;
            }

            //returns height of the image
            uint32_t get_height() override {
                return btmp_height;
            }

            size_t get_raw_size() override {
                return raw_data_size;
            }

            //changes the size of the image
            void resize(uint32_t width, uint32_t height) override {
                if(!initialized)
                    return;
                if(width == btmp_width && height == btmp_height) // args are the same size as image
                    return;
                //if(set_width < btmp_width || set_height < btmp_height) // if args are smaller
                //    return;

                btmp_width = width;
                btmp_height = height;

                total_size_in_bytes = pixel_data_offset + btmp_height * btmp_width * 3; // recalculate size attribs
                raw_data_size = btmp_height * btmp_width * 3;

                pixel_data = (uint8_t*)realloc(pixel_data, raw_data_size); // realloc bitmap data
            }

            //clears the image
            void clear() override {
                if(!initialized)
                    return;
                memset(pixel_data, 0, raw_data_size);
            }

            //frees the memory of the image and resets all properties
            void del() override {
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

            bool is_initialized() override {
                return initialized;
            }

            private:

            //just two little helper function
            //used to get the array index of any pixel/byte
            size_t get_p_index(uint32_t x_pos, uint32_t y_pos){
                //pixel index
                return ((btmp_height - y_pos - 1) * btmp_width + x_pos) * 3; //y_pos is inverted because of the way the image is stored
            }
            size_t get_r_index(uint32_t x_pos, uint32_t y_pos){
                //raw index
                return (y_pos * btmp_width * 3 + x_pos);
            }

            //BMP header
            const char ID_f1 = 'B', ID_f2 = 'M';
            uint32_t total_size_in_bytes;
            const uint16_t unused = 0;
            const uint32_t pixel_data_offset = 54;

            //DIB header
            const uint32_t DIB_header_size = 40;
            uint32_t btmp_width, btmp_height;
            const uint16_t color_planes = 1;
            const uint16_t bits_per_pixel = 24;
            const uint32_t Bl_RGB = 0;
            uint32_t raw_data_size;
            const uint32_t DPI_hor = 2835, DPI_ver = 2835;
            const uint32_t color_palette = 0;
            const uint32_t imp_colors = 0;

            uint8_t * pixel_data = nullptr;
            bool initialized = false;
        };
    }

    namespace graphics{

        //draws a line between two points
        inline void line(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, color::Color col){
            if(!img.is_initialized())
                return;
            int32_t ax = x2 - x1, ay = y2 - y1;
            ax = (ax < 0) ? -ax : ax;
            ay = (ay < 0) ? ay : -ay;
            int32_t dx = ax, sx = x1 < x2 ? 1 : -1;
            int32_t dy = ay, sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy, e2;

            while (true) {
                img.set_pixel(x1, y1, col);
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

        //sets every pixel on the image to one color
        inline void fill(base::image &img, color::Color col){
            if(!img.is_initialized())
                return;
            for(uint32_t y = 0; y < img.get_height(); y++){
                for(uint32_t x = 0; x < img.get_width(); x++){
                    img.set_pixel(x, y, col);
                }
            }
        }

        //sets an area of pixels, with the same color, to another color
        //like the bucket in paint if that makes sense
        inline void floodfill(base::image &img, int32_t x, int32_t y, color::Color col){
            if(!img.is_initialized() || x < 0 || y < 0 || x >= img.get_width() || y >= img.get_height())
                return;

            color::Color old_col = img.get_pixel(x, y);

            std::stack<uint32_t> pixels_to_fill; // create a stack to store the pixel coordinates
            pixels_to_fill.push(x); // push current pixel coords into stack
            pixels_to_fill.push(y);

            uint32_t x_buf = 0, y_buf = 0; // declare and initialize position buffers

            while(pixels_to_fill.size() > 0 && pixels_to_fill.size() < 100000000){ // while the stack isn't empty and doesn't have 100mil or more elements
                // read pixel coords into buffers
                y_buf = pixels_to_fill.top();
                pixels_to_fill.pop();
                x_buf = pixels_to_fill.top();
                pixels_to_fill.pop();

                if(img.get_pixel(x_buf, y_buf) == old_col){ // check if buffer has the original color

                    //if yes then replace with new color
                    img.set_pixel(x_buf, y_buf, col);

                    //push neighboring pixels into the stack but check if they are out of bounds
                    if(x_buf > 0){
                        pixels_to_fill.push(x_buf - 1);
                        pixels_to_fill.push(y_buf);
                    }
                    if(x_buf < img.get_width() - 1){
                        pixels_to_fill.push(x_buf + 1);
                        pixels_to_fill.push(y_buf);
                    }
                    if(y_buf > 0){
                        pixels_to_fill.push(x_buf);
                        pixels_to_fill.push(y_buf - 1);
                    }
                    if(y_buf < img.get_height() - 1){
                        pixels_to_fill.push(x_buf);
                        pixels_to_fill.push(y_buf + 1);
                    }
                }
            }
        }

        //draws a rectangle of given size
        inline void rectangle(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, color::Color col){
            if(!img.is_initialized() || x1 > x2 || y1 > y2)
                return;
            for(uint32_t i = y1; i <= y2; i++){
                for(uint32_t j = x1; j <= x2; j++){
                    img.set_pixel(j, i, col);
                }
            }
        }

        //draws a border of given size
        inline void border(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t thickness, color::Color col){
            if(!img.is_initialized() || x1 > x2 || y1 > y2)
                return;

            //vertical lines
            for(uint32_t j = y1; j <= y2; j++){
                //left
                for(int32_t i = x1; i <= x1 + thickness; i++){
                    img.set_pixel(i, j, col);
                }
                //right
                for(int32_t i = x2 - thickness; i <= x2; i++){
                    img.set_pixel(i, j, col);
                }
            }
            //horizontal lines
            for(int32_t i = x1 + thickness; i <= x2 - thickness; i++){
                //upper
                for(uint32_t j = y1; j <= y1 + thickness; j++){
                    img.set_pixel(i, j, col);
                }
                //lower
                for(uint32_t j = y2 - thickness; j <= y2; j++){
                    img.set_pixel(i, j, col);
                }
            }
        }

        //draws a circle of given size
        inline void circle(base::image &img, int32_t x_pos, int32_t y_pos, int32_t radius, color::Color col){
            if(!img.is_initialized() || radius < 1)
                return;
            for(int32_t i = -radius; i < radius; i++){
                for(int32_t j = -radius; j < radius; j++){
                    if(i * i + j * j <= radius * radius){
                        img.set_pixel(x_pos + i, y_pos + j, col);
                    }
                }
            }
        }

        //draws an ellipse of given size
        inline void ellipse(base::image &img, int32_t x_pos, int32_t y_pos, int32_t radius, float x_mult, float y_mult, color::Color col){
            if(!img.is_initialized() || radius < 1)
                return;
            float x_mult_inv = 1 / x_mult;
            float y_mult_inv = 1 / y_mult;
            for(int32_t i = -radius * x_mult; i < radius * x_mult; i++){
                for(int32_t j = -radius * y_mult; j < radius * y_mult; j++){
                    if(x_mult_inv * i * i + y_mult_inv * j * j <= radius * radius){
                        img.set_pixel(x_pos + i, y_pos + j, col);
                    }
                }
            }
        }

        //draw a sector of a circle, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        inline void circle_sector(base::image &img, int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, color::Color col){
            if(!img.is_initialized() || radius < 1)
                return;
            //calculate constant to convert degrees into radians
            const double pi_over_180 = 3.14159265 / 180.0;
            img.set_pixel(x_pos, y_pos, col);
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
                    img.set_pixel(std::sin(angle_iter * pi_over_180) * radius_iter + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter + y_pos, col);
                }
            }
        }

        //draw a sector of an ellipse, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        inline void ellipse_sector(base::image &img, int32_t x_pos, int32_t y_pos, uint32_t radius, float start_angle, float end_angle, float x_mult, float y_mult, color::Color col){
            if(!img.is_initialized() || radius < 1)
                return;
            //calculate constant to convert degrees into radians
            const double pi_over_180 = 3.14159265 / 180.0;
            img.set_pixel(x_pos, y_pos, col);
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
                    img.set_pixel(std::sin(angle_iter * pi_over_180) * radius_iter * x_mult + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter * y_mult + y_pos, col);
                }
            }
        }

        //draws a ring of given size
        inline void ring(base::image &img, int32_t x_pos, int32_t y_pos, int32_t out_radius, int32_t in_radius, color::Color col){
            if(!img.is_initialized() || out_radius < 1 || in_radius < 0 || out_radius < in_radius)
                return;
            for(int32_t i = -out_radius; i < out_radius; i++){
                for(int32_t j = -out_radius; j < out_radius; j++){
                    if(i * i + j * j <= out_radius * out_radius && i * i + j * j >= in_radius * in_radius){
                        img.set_pixel(x_pos + i, y_pos + j, col);
                    }
                }
            }
        }

        //draw a sector of a ring, for example a quarter or an eighth
        //both angle parameters are in degrees not radians
        inline void ring_sector(base::image &img, int32_t x_pos, int32_t y_pos, uint32_t out_radius, uint32_t in_radius, float start_angle, float end_angle, color::Color col){
            if(!img.is_initialized() || in_radius < 1 || out_radius <= in_radius)
                return;
            //calculate constant to convert degrees into radians
            const double pi_over_180 = 3.14159265 / 180.0;
            //iterate over all radii starting from 1 to and including the final radius
            for(uint32_t radius_iter = in_radius; radius_iter <= out_radius; radius_iter++){
                //calculate the angle difference and the length of the sectors outer diameter
                double angle_diff = std::abs(start_angle - end_angle);
                double pixel_arc_len = 3.14159265 * radius_iter * radius_iter * (360.0 / angle_diff);
                //use that to estimate a rough step size for the angle iterator
                double angle_step = angle_diff / pixel_arc_len;

                //then iterate over from the starting angle to the end angle
                //The point is calculated like this: P = (sin(a) * r | cos(a) * r)
                for(double angle_iter = start_angle; angle_iter <= end_angle; angle_iter += angle_step){
                    //cos is negated because in the image up means smaller y while in the coordinate system up means larger y
                    img.set_pixel(std::sin(angle_iter * pi_over_180) * radius_iter + x_pos, -std::cos(angle_iter * pi_over_180) * radius_iter + y_pos, col);
                }
            }
        }

        //draws a rounded rectangle of given size
        inline void round_rectangle(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t radius, color::Color col){
            if(!img.is_initialized() || x1 > x2 || y1 > y2)
                return;
            uint32_t max_radius = std::min(std::abs(x1 - x2), std::abs(y1 - y2) / 2);
            if(radius > max_radius)
                radius = max_radius;

            //draw the four corners fist
            circle_sector(img, x1 + radius, y1 + radius, radius, 270, 360, col);
            circle_sector(img, x2 - radius, y1 + radius, radius, 0, 90, col);
            circle_sector(img, x1 + radius, y2 - radius, radius, 180, 270, col);
            circle_sector(img, x2 - radius, y2 - radius, radius, 90, 180, col);

            //then fill the rest
            rectangle(img, x1, y1 + radius, x2, y2 - radius, col);
            rectangle(img, x1 + radius, y1, x2 - radius, y1 + radius, col);
            rectangle(img, x1 + radius, y2 - radius, x2 - radius, y2, col);
        }

        //draws a rounded border of given size
        inline void round_border(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint16_t thickness ,uint32_t radius, color::Color col){
            if(!img.is_initialized() || x1 > x2 || y1 > y2)
                return;
            uint32_t max_radius = std::min(std::abs(x1 - x2), std::abs(y1 - y2) / 2);
            uint32_t min_radius = thickness + 1;
            if(radius < min_radius)
                radius = min_radius;
            if(radius > max_radius)
                radius = max_radius;

            //draw the four corners fist
            ring_sector(img, x1 + radius, y1 + radius, radius, radius - thickness, 270, 360, col);
            ring_sector(img, x2 - radius, y1 + radius, radius, radius - thickness, 0, 90, col);
            ring_sector(img, x1 + radius, y2 - radius, radius, radius - thickness, 180, 270, col);
            ring_sector(img, x2 - radius, y2 - radius, radius, radius - thickness, 90, 180, col);

            //then fill the rest
            rectangle(img, x1, y1 + radius, x1 + thickness, y2 - radius, col);
            rectangle(img, x2 - thickness, y1 + radius, x2, y2 - radius, col);
            rectangle(img, x1 + radius, y1, x2 - radius, y1 + thickness, col);
            rectangle(img, x1 + radius, y2 - thickness, x2 - radius, y2, col);
        }

        //draws a filled triangle
        inline void triangle(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color::Color col){
            if(!img.is_initialized())
                return;
            int32_t ax = x2 - x1, ay = y2 - y1;
            ax = (ax < 0) ? -ax : ax;
            ay = (ay < 0) ? ay : -ay;
            int32_t dx = ax, sx = x1 < x2 ? 1 : -1;
            int32_t dy = ay, sy = y1 < y2 ? 1 : -1;
            int32_t err = dx + dy, e2;

            line(img, x1, y1, x3, y3, col);

            while (true) {
                if (x1 == x2 && y1 == y2) break;
                e2 = 2 * err;
                if (e2 > dy){
                    err += dy;
                    x1 += sx;
                    line(img, x1, y1, x3, y3, col);
                }
                if (e2 < dx){
                    err += dx;
                    y1 += sy;
                    line(img, x1, y1, x3, y3, col);
                }
            }
        }

        //draw triangle border by connecting three points with lines
        inline void triangle_border(base::image &img, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, color::Color col){
            if(!img.is_initialized())
                return;
            line(img, x1, y1, x2, y2, col);
            line(img, x2, y2, x3, y3, col);
            line(img, x3, y3, x1, y1, col);
        }

        //draws a char from namespace chars
        //character bitmap
        inline void draw_char(base::image &img, int32_t x_pos, int32_t y_pos, uint16_t size, const chars::Charbtmp chr, color::Color col){
            if(!img.is_initialized())
                return;
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 5; j++){
                    if((chr[i] << (j + 3)) & 0b10000000) // offset of 3 because the pixel is stored in the last 5 bits of the char and not the first 5
                        rectangle(img, x_pos + j * size, y_pos + i * size, x_pos + (j + 1) * size - 1, y_pos + (i + 1) * size - 1, col);
                }
            }
        }

        //draws a string
        inline void draw_string(base::image &img, int32_t x_pos, int32_t y_pos, uint16_t size, const char *str, color::Color col){
            if(!img.is_initialized())
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
                draw_char(img, x_pos + i * size * 6, y_pos, size, chars::asciitocbtmp(*str), col);
                i++; str++;
            }
        }

        inline void encode_str(base::image &img, const char *str){
            if(!img.is_initialized())
                return;

            //get string length and include NULL char
            size_t str_len = std::strlen(str) + 1;
            size_t data_index = 0;

            //escape bit
            bool esc_bit = true;

            while(esc_bit){

                uint8_t bit_mask = 0b10000000;
                bool str_bit;

                //this loop iterates over every bit of the current char of the string
                for(uint8_t i = 0; i < 8 && esc_bit; i++){
                    //get bit
                    str_bit = *str & bit_mask;

                    //encode bit
                    img.data()[data_index] = ((str_bit) ? (img.data()[data_index] | 0b00000001) : (img.data()[data_index] & 0b11111110));

                    //shift mask
                    bit_mask >>= 1;

                    //increase index
                    data_index++;

                    //if end of array or end of string is reached, exit all loops by setting the escape bit
                    if(data_index >= img.get_raw_size() || data_index >= str_len * 8)
                        esc_bit = false;
                }

                //go to the next char in the string
                str++;
            }
        }

        inline const char *decode_str(base::image &img){
            if(!img.is_initialized())
                return nullptr;
            
            //calculate and allocate the maximum possible string size
            //every byte of the raw pixel data array stores one single bit of the encoded string
            //So every image can encode a string 1/8th it's size
            size_t str_len = img.get_raw_size() / 8;
            char *str_buffer = (char*)calloc(str_len, sizeof(char));

            //string pointer iterator
            char *str_ptr = str_buffer;

            //we start at index 0
            size_t data_index = 0;

            //escape bit
            bool esc_bit = true;

            while(esc_bit){
                //iterate over every bit of a char
                for(uint8_t i = 0; i < 8 && esc_bit; i++){
                    //extract char from the least significant bit of the pixel data
                    *str_ptr |= ((0b00000001 & img.data()[data_index]) << (7 - i));

                    //go to next array index
                    data_index++;

                    //exit the loops if the end of the pixel data array is reached
                    if(data_index >= img.get_raw_size())
                        esc_bit = false;
                }

                //if the last extracted char is NULL then end the decoding process
                if(*str_ptr == '\0'){
                    //calculate actual stringsize
                    str_len = str_ptr - str_buffer;
                    esc_bit = false;
                }

                //go to the next char
                str_ptr++;
            }

            //reallocate the string buffer so that it isn't larger than it has to be
            char *out = (char*)realloc((void*)str_buffer, str_len);

            if(out == nullptr){
                free(str_buffer);
                return nullptr;
            }

            //finally return the decoded string
            return out;
        }
    }

    namespace filters {

        //converts the image to black and white in the specified area
        inline void convert_bw(base::image &img, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
            if(!img.is_initialized() || x1 > x2 || y1 > y2 || x2 > img.get_width() || y2 > img.get_height())
                return;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    img.set_pixel(i, j, color::blackNwhite(img.get_pixel(i, j)));
                }
            }
        }

        //inverts the rgb values of the image in the specified area
        inline void color_invert(base::image &img, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
            if(!img.is_initialized() || x1 > x2 || y1 > y2 || x2 > img.get_width() - 1 || y2 > img.get_height() - 1)
                return;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    img.set_pixel(i, j, color::invert(img.get_pixel(i, j)));
                }
            }
        }

        //flips the image horizontally
        inline void flip_horizontal(base::image &img){
            if(!img.is_initialized())
                return;
            color::Color buffer;
            for(uint32_t i = 0; i < img.get_width(); i++){
                for(uint32_t j = 0; j < img.get_height() / 2; j++){
                    //swapping colors around
                    buffer = img.get_pixel(i, img.get_height() - j - 1);
                    img.set_pixel(i, img.get_height() - j - 1, img.get_pixel(i, j));
                    img.set_pixel(i, j, buffer);
                }
            }
        }

        //flips the image vertically
        inline void flip_vertical(base::image &img){
            if(!img.is_initialized())
                return;
            color::Color buffer;
            for(uint32_t i = 0; i < img.get_width() / 2; i++){
                for(uint32_t j = 0; j < img.get_height(); j++){
                    //swapping colors around
                    buffer = img.get_pixel(img.get_width() - i - 1, j);
                    img.set_pixel(img.get_width() - i - 1, j, img.get_pixel(i, j));
                    img.set_pixel(i, j, buffer);
                }
            }
        }
    }
}