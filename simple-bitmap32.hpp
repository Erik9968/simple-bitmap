/* very simple bitmap library version exp 0.34
 * by Erik S.
 * 
 * This library is an improved version of my original bitmap library.
 * The original library used vectors to store the bitmap data. (It was too slow)
 * 
 * A few functions are not working properly:
 * breseham line drawing algorithm
 * triangle drawing function (uses bresenham line drawing algorithm)
 * 
 * This library, belive it or not, can not load a bitmap file (yet).
 * Yes that's right, it can NOT load a bitmap file.
 * 
 * I used the following resources to help me with this library:
 * https://en.wikipedia.org/wiki/BMP_file_format
 * https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
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
 * - Fixed header bug that coused transparent parts to be not transparent
 * 
 * 
 * TODO:
 * - fix circle function (does not work when part of the circle is outside of the image)
 * - fix triangle drawing function
 * - fix breseham line drawing algorithm
 * - add load function
 * - add characater/string drawing function
 */


#include <fstream>

namespace sbtmp{

    class bitmap{
        public:

        //constructor
        bitmap(uint32_t set_width, uint32_t set_height) {
            if (initialized) {
                return;
            }

            width = set_width;
            height = set_height;

            total_size_in_bytes = pixel_data_offset + height * width * 4;
            raw_data_size = height * width * 4;

            //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
            pixel_data = (uint8_t*)calloc(raw_data_size, sizeof(uint8_t));

            initialized = true;
        }

        //allows not using the constructor
        bitmap() = default;

        //create function (recommended way to init images)
        bool create(uint32_t set_width, uint32_t set_height){
            if(initialized){
                return false;
            }

            width = set_width;
            height = set_height;

            total_size_in_bytes = pixel_data_offset + height * width * 4;
            raw_data_size = height * width * 4;

            //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
            pixel_data = (uint8_t*)calloc (raw_data_size, sizeof(uint8_t));

            initialized = true;

            return true;
        }

        //return width of the image
        uint32_t get_width(){
            return width;
        }

        //returns height of the image
        uint32_t get_height(){
            return height;
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
        void set_pixel(uint32_t x_pos, uint32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if (x_pos > width - 1 || y_pos > height - 1)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] = blue;
            pixel_data[get_index(x_pos, y_pos)+1] = green;
            pixel_data[get_index(x_pos, y_pos)+2] = red;
            pixel_data[get_index(x_pos, y_pos)+3] = alpha;
        }

        #ifdef sbtmp_experimental
        void line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x1 > x2 || y1 > y2 || x1 > width - 1 || y1 > height - 1 || x2 > width - 1 || y2 > height - 1)
                return;
            //Bresenham's line algorithm
            int32_t x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
            dx = x2 - x1;
            dy = y2 - y1;
            dx1 = abs(dx);
            dy1 = abs(dy);
            px = 2 * dy1 - dx1;
            py = 2 * dx1 - dy1;
            if(dy1 <= dx1){
                if(dx >= 0){
                    x = x1;
                    y = y1;
                    xe = x2;
                }
                else{
                    x = x2;
                    y = y2;
                    xe = x1;
                }
                set_pixel(x, y, red, green, blue, alpha);
                for(i = 0; x < xe; i++){
                    x = x + 1;
                    if(px < 0){
                        px = px + 2 * dy1;
                    }
                    else{
                        if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)){
                            y = y + 1;
                        }
                        else{
                            y = y - 1;
                        }
                    }
                    set_pixel(x, y, red, green, blue, alpha);
                }
            }
            else{
                if(dy >= 0){
                    x = x1;
                    y = y1;
                    ye = y2;
                }
                else{
                    x = x2;
                    y = y2;
                    ye = y1;
                }
                set_pixel(x, y, red, green, blue, alpha);
                for(i = 0; y < ye; i++){
                    y = y + 1;
                    if(py <= 0){
                        py = py + 2 * dx1;
                    }
                    else{
                        if((dx < 0 && dy < 0) || (dx > 0 && dy > 0)){
                            x = x + 1;
                        }
                        else{
                            x = x - 1;
                        }
                    }
                    set_pixel(x, y, red, green, blue, alpha);
                }
            }
        }
        #endif

        //sets every pixel of the image to rgba value
        void fill(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            for(uint32_t i = 0; i < height; i++){
                for(uint32_t j = 0; j < width; j++){
                    set_pixel(j, i, red, green, blue, alpha);
                }
            }
        }

        //draws a rectangle of given size
        void rectangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x1 > x2 || y1 > y2 || x1 > width - 1 || y1 > height - 1 || x2 > width - 1 || y2 > height - 1)
                return;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    set_pixel(i, j, red, green, blue, alpha);
                }
            }
        }

        //draws a circle of given size
        void circle(uint32_t x_pos, uint32_t y_pos, int32_t radius, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x_pos > width - 1 || y_pos > height - 1 || x_pos + radius > width - 1 || y_pos + radius > height - 1 || x_pos - radius < 0 || y_pos - radius < 0 || radius < 1)
                return;
            for(int32_t i = -radius; i < radius; i++){
                for(int32_t j = -radius; j < radius; j++){
                    if(i * i + j * j <= radius * radius){
                        set_pixel(x_pos + i, y_pos + j, red, green, blue, alpha);
                    }
                }
            }
        }

        #ifdef sbtmp_experimental
        void triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            line(x1, y1, x2, y2, red, green, blue, alpha);
            line(x2, y2, x3, y3, red, green, blue, alpha);
            line(x3, y3, x1, y1, red, green, blue, alpha);
        }
        #endif

        //converts the image to black and white in the specified area
        void convert_bw(uint32_t x_pos, uint32_t y_pos, uint32_t width, uint32_t height){
            char bw_color;
            for(uint32_t i = 0; i < height; i++){
                for(uint32_t j = 0; j < width; j++){
                    bw_color = (pixel_data[get_index(x_pos + j, y_pos + i)+0] + pixel_data[get_index(x_pos + j, y_pos + i)+1] + pixel_data[get_index(x_pos + j, y_pos + i)+2]) / 3;
                    pixel_data[get_index(x_pos + j, y_pos + i)+0] = bw_color;
                    pixel_data[get_index(x_pos + j, y_pos + i)+1] = bw_color;
                    pixel_data[get_index(x_pos + j, y_pos + i)+2] = bw_color;
                }
            }
        }

        //inverts the rgb values of the image in the specified area
        void rgb_invert(uint32_t x_pos, uint32_t y_pos, uint32_t width, uint32_t height){
            for(uint32_t i = 0; i < height; i++){
                for(uint32_t j = 0; j < width; j++){
                    pixel_data[get_index(x_pos + j, y_pos + i)+0] = 255 - pixel_data[get_index(x_pos + j, y_pos + i)+0];
                    pixel_data[get_index(x_pos + j, y_pos + i)+1] = 255 - pixel_data[get_index(x_pos + j, y_pos + i)+1];
                    pixel_data[get_index(x_pos + j, y_pos + i)+2] = 255 - pixel_data[get_index(x_pos + j, y_pos + i)+2];
                }
            }
        }

        //flips the image vertically
        void flip_vertical(){
            for(uint32_t i = 0; i < height; i++){
                for(uint32_t j = 0; j < width / 2; j++){
                    //this is magic
                    std::swap(pixel_data[get_index(i, j)], pixel_data[get_index(i, width - j - 1)]);
                    std::swap(pixel_data[get_index(i, j)+1], pixel_data[get_index(i, width - j - 1)+1]);
                    std::swap(pixel_data[get_index(i, j)+2], pixel_data[get_index(i, width - j - 1)+2]);
                    std::swap(pixel_data[get_index(i, j)+3], pixel_data[get_index(i, width - j - 1)+3]);
                }
            }
        }

        //flips the image horizontally
        void flip_horizontal(){
            for(uint32_t i = 0; i < height / 2; i++){
                for(uint32_t j = 0; j < width; j++){
                    //this is magic too
                    std::swap(pixel_data[get_index(i, j)], pixel_data[get_index(height - i - 1, j)]);
                    std::swap(pixel_data[get_index(i, j)+1], pixel_data[get_index(height - i - 1, j)+1]);
                    std::swap(pixel_data[get_index(i, j)+2], pixel_data[get_index(height - i - 1, j)+2]);
                    std::swap(pixel_data[get_index(i, j)+3], pixel_data[get_index(height - i - 1, j)+3]);
                }
            }
        }

        #ifdef sbtmp_experimental
        void merge(bitmap& other){
            if(other.get_width() != width || other.get_height() != height){
                return;
            }

            for(uint32_t i = 0; i < width; i++){
                for(uint32_t j = 0; j < height; j++){
                    if(other.get_pixel(i, j, 1) != 0 && other.get_pixel(i, j, 2) != 0 && other.get_pixel(i, j, 3) != 0){
                        set_pixel(i, j, other.get_pixel(i, j, 1), other.get_pixel(i, j, 2), other.get_pixel(i, j, 3), other.get_pixel(i, j, 0));
                    }
                }
            }
        }
        #endif

        //adds rgba value to specified pixel
        void addtpixel(uint32_t x_pos, uint32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x_pos > width - 1 || y_pos > height - 1)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] += blue;
            pixel_data[get_index(x_pos, y_pos)+1] += green;
            pixel_data[get_index(x_pos, y_pos)+2] += red;
            pixel_data[get_index(x_pos, y_pos)+3] += alpha;
        }

        //subtracts rgba value from specified pixel
        void subfpixel(uint32_t x_pos, uint32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x_pos > width - 1 || y_pos > height - 1)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] -= blue;
            pixel_data[get_index(x_pos, y_pos)+1] -= green;
            pixel_data[get_index(x_pos, y_pos)+2] -= red;
            pixel_data[get_index(x_pos, y_pos)+3] -= alpha;
        }

        //return the r-g-b-a value of specified pixel (3rd param: 0=b, 1=g, 2=r, 3=a)
        uint8_t get_pixel(uint32_t x_pos, uint32_t y_pos, uint8_t color_index){
            if(color_index > 3)
                return 0;
            return pixel_data[get_index(x_pos, y_pos) + color_index];
        }

        //changes the size of the image
        bool resize(uint32_t set_width, uint32_t set_height){
            if(!initialized){
                return false;
            }

            if(set_width == width && set_height == height){
                return true;
            }

            if(set_width < width || set_height < height){
                return false;
            }


            width = set_width;
            height = set_height;

            total_size_in_bytes = pixel_data_offset + height * width * 4;
            raw_data_size = height * width * 4;

            pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);

            return true;
        }

        //clears the image
        void clear(){
            for(uint64_t i = 0; i < raw_data_size; i++){
                pixel_data[i] = 0;
            }
        }

        //frees the memory of the image and resets all properties
        bool reset(){
            if(!initialized){
                return false;
            }

            width = 0;
            height = 0;
            total_size_in_bytes = 0;
            raw_data_size = 0;

            pixel_data = (uint8_t*)realloc (pixel_data, 0);

            initialized = false;

            return true;
        }

        //saves the image with given filename
        bool save(const char * filename){
            if(!initialized){
                return false;
            }
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
            out_image.write((char *)&width, 4);
            out_image.write((char *)&height, 4);
            out_image.write((char *)&color_planes, 2);
            out_image.write((char *)&bits_per_pixel, 2);
            out_image.write((char *)&Bl_RGB, 4);
            out_image.write((char *)&raw_data_size, 4);
            out_image.write((char *)&DPI_hor, 4);
            out_image.write((char *)&DPI_ver, 4);
            out_image.write((char *)&color_palette, 4);
            out_image.write((char *)&imp_colors, 4);

            out_image.write((char *)&red_channel_bit_mask, 4);
            out_image.write((char *)&green_channel_bit_mask, 4);
            out_image.write((char *)&blue_channel_bit_mask, 4);
            out_image.write((char *)&alpha_channel_bit_mask, 4);
            out_image.write((char *)&*color_space, 4);
            out_image.write((char *)&*color_space_endpoints, 36);
            out_image.write((char *)&*gamma_rgb, 12);

            //much better way to write pixel data
            out_image.write((char *)&*pixel_data, raw_data_size);

            out_image.close();

            return true;
        }

        //if std::string is included, the save function can be called with a string instead of a char array
        #if defined(_GLIBCXX_STRING_) //gnu gcc compiler (linux)
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
        uint32_t width, height;
        const uint16_t color_planes = 1;
        const uint16_t bits_per_pixel = 32;
        const uint32_t Bl_RGB = 3;
        uint32_t raw_data_size;
        const uint32_t DPI_hor = 2835, DPI_ver = 2835;
        const uint32_t color_palette = 0;
        const uint32_t imp_colors = 0;

        const uint32_t red_channel_bit_mask = 0x00ff0000;
        const uint32_t green_channel_bit_mask = 0x0000ff00;
        const uint32_t blue_channel_bit_mask = 0x000000ff;
        const uint32_t alpha_channel_bit_mask = 0xff000000;
        const char color_space[4] = {' ', 'n', 'i', 'W'};
        const uint8_t color_space_endpoints[36] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        const uint32_t gamma_rgb[3] = {0, 0, 0};

        const uint8_t padding = 0;
        uint8_t padding_size;
        uint8_t * pixel_data = (uint8_t*)malloc (0);
        bool initialized = false;

        //function used to get the array index of any pixel
        uint64_t get_index(uint32_t x_pos, uint32_t y_pos){
            return ((width - y_pos - 1) * width + x_pos) * 4; //y_pos is inverted because of the way the image is stored
        }
    };
}