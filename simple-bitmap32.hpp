/* very simple bitmap library version exp 0.36
 * by Erik S.
 * 
 * This library is an improved version of my original bitmap library.
 * The original library used vectors to store the bitmap data. (It was too slow)
 * 
 * A few functions are not working properly:
 * breseham line drawing algorithm
 * triangle drawing function (uses bresenham line drawing algorithm)
 * 
 * This library can since ver exp 0.35 load 32bit bitmap image files
 * The function is currently very picky because it expects a very basic bmp and dib header
 * This means that any extra information like embedded icc color profiles or other stuff will
 * be ignored or more likely cause the load function to crash
 * No support for compressed bitmaps
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
 * 0.35:
 * - Added color datatype (no real use yet)
 * - Added image load function
 * - Added floodfill function
 *
 * 0.36:
 * - Circle function is fixed (not all pixels of a circle have to be inside the image)
 *      -Circles can be outside of an image and be visible at the edge
 *      -Circles can be in the corner of an image
 * - Changed color datatype from class to struct
 * - Colors can now be passed to a function directly
 *      -some_func(sbtmp::color(sbtmp_blue)); //this works now
 * 
 * 
 * TODO:
 * - fix breseham line drawing algorithm
 * - fix triangle drawing function
 * - add blur function
 * - add color datatype wrapper functions
 * - add function to load bitmap data from arrays
 * - add more filters and resize functions (nearest neighbour | bilinear | bicubic)
 * - add characater/string drawing function
 */


#include <fstream>


namespace sbtmp{

    struct color{

        // public:
        color(uint32_t raw){
            color_data = raw;
        }

        color() = default;

        void set_red(uint8_t red){
            color_data = (color_data & 0x00ffffff) | ((uint32_t)red << 24);
        }

        void set_green(uint8_t green){
            color_data = (color_data & 0xff00ffff) | ((uint32_t)green << 16);
        }

        void set_blue(uint8_t blue){
            color_data = (color_data & 0xffff00ff) | ((uint32_t)blue << 8);
        }

        void set_alpha(uint8_t alpha){
            color_data = (color_data & 0xffffff00) | ((uint32_t)alpha);
        }

        uint8_t get_red(){
            return color_data >> 24;
        }

        uint8_t get_green(){
            return (color_data >> 16) & 0x000000ff;
        }

        uint8_t get_blue(){
            return (color_data >> 8) & 0x000000ff;
        }

        uint8_t get_alpha(){
            return color_data & 0x000000ff;
        }

        uint32_t color_data = 0x000000ff;   //color is not transparent by default
    };





    

    class bitmap{
        public:

        //constructor
        bitmap(uint32_t set_width, uint32_t set_height) {
            if(initialized)
                return;

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
        void create(uint32_t set_width, uint32_t set_height){
            if(initialized)
                return;

            width = set_width;
            height = set_height;

            total_size_in_bytes = pixel_data_offset + height * width * 4;
            raw_data_size = height * width * 4;

            //pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
            pixel_data = (uint8_t*)calloc (raw_data_size, sizeof(uint8_t));

            initialized = true;
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
            if (x_pos > width - 1 || y_pos > height - 1 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] = blue;
            pixel_data[get_index(x_pos, y_pos)+1] = green;
            pixel_data[get_index(x_pos, y_pos)+2] = red;
            pixel_data[get_index(x_pos, y_pos)+3] = alpha;
        }

        void set_pixel(uint32_t x_pos, uint32_t y_pos, color& val){
            set_pixel(x_pos, x_pos, val.get_red(), val.get_green(), val.get_blue(), val.get_alpha());
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
            if(!initialized)
                return;
            for(uint32_t i = 0; i < height; i++){
                for(uint32_t j = 0; j < width; j++){
                    set_pixel(j, i, red, green, blue, alpha);
                }
            }
        }

        void fill(color val){
            fill(val.get_red(), val.get_green(), val.get_blue(), val.get_alpha());
        }

        //floodfill like "bucket" in paint
        void floodfill(uint32_t x_pos, uint32_t y_pos, uint8_t old_red, uint8_t old_green, uint8_t old_blue, uint8_t new_red, uint8_t new_green, uint8_t new_blue){
            if(x_pos > width - 1 || y_pos > height - 1 || !initialized)
                return;
            if(get_pixel(x_pos, y_pos, 0) == old_blue && get_pixel(x_pos, y_pos, 1) == old_green && get_pixel(x_pos, y_pos, 2) == old_red){
                set_pixel(x_pos, y_pos, new_red, new_green, new_blue, get_pixel(x_pos, y_pos, 3));
                floodfill(x_pos + 1, y_pos, old_red, old_green, old_blue, new_red, new_green, new_blue);
                floodfill(x_pos - 1, y_pos, old_red, old_green, old_blue, new_red, new_green, new_blue);
                floodfill(x_pos, y_pos + 1, old_red, old_green, old_blue, new_red, new_green, new_blue);
                floodfill(x_pos, y_pos - 1, old_red, old_green, old_blue, new_red, new_green, new_blue);
            }
        }

        void floodfill(uint32_t x_pos, uint32_t y_pos, color& old_col, color& new_col){
            floodfill(x_pos, y_pos, old_col.get_red(), old_col.get_green(), old_col.get_blue(), new_col.get_red(), new_col.get_green(), new_col.get_blue());
        }

        //draws a rectangle of given size
        void rectangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x1 > x2 || y1 > y2 || x1 > width - 1 || y1 > height - 1 || x2 > width - 1 || y2 > height - 1 || !initialized)
                return;
            for(uint32_t i = x1; i < x2; i++){
                for(uint32_t j = y1; j < y2; j++){
                    set_pixel(i, j, red, green, blue, alpha);
                }
            }
        }

        void rectangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, color val){
            rectangle(x1, y1, x2, y2, val.get_red(), val.get_green(), val.get_blue(), val.get_alpha());
        }

        //draws a circle of given size
        void circle(int32_t x_pos, int32_t y_pos, int32_t radius, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(/*x_pos > width - 1 || y_pos > height - 1 || x_pos + radius > width - 1 || y_pos + radius > height - 1 || x_pos - radius < 0 || y_pos - radius < 0 || */radius < 1 || !initialized)
                return;
            for(int32_t i = -radius; i < radius; i++){
                for(int32_t j = -radius; j < radius; j++){
                    if(i * i + j * j <= radius * radius && x_pos + i < width && x_pos + i >= 0 && y_pos + j < height && y_pos + j >= 0){
                        set_pixel(x_pos + i, y_pos + j, red, green, blue, alpha);
                    }
                }
            }
        }

        void circle(uint32_t x_pos, uint32_t y_pos, int32_t radius, color val){
            circle(x_pos, y_pos, radius, val.get_red(), val.get_green(), val.get_blue(), val.get_alpha());
        }

        #ifdef sbtmp_experimental
        void triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            line(x1, y1, x2, y2, red, green, blue, alpha);
            line(x2, y2, x3, y3, red, green, blue, alpha);
            line(x3, y3, x1, y1, red, green, blue, alpha);
        }
        #endif

        //converts the image to black and white in the specified area
        void convert_bw(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2){
            if(x1 > width || y1 > height || x2 > width || y2 > height || !initialized)
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
            if(x1 > width || y1 > height || x2 > width || y2 > height || !initialized)
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

        //flips the image vertically
        void flip_vertical(){
            if(!initialized)
                return;
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
            if(!initialized)
                return;
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
            if(x_pos > width - 1 || y_pos > height - 1 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] += blue;
            pixel_data[get_index(x_pos, y_pos)+1] += green;
            pixel_data[get_index(x_pos, y_pos)+2] += red;
            pixel_data[get_index(x_pos, y_pos)+3] += alpha;
        }

        void addtpixel(uint32_t x_pos, uint32_t y_pos, color val){
            addtpixel(x_pos, x_pos, val.get_red(), val.get_green(), val.get_blue(), val.get_alpha());
        }

        //subtracts rgba value from specified pixel
        void subfpixel(uint32_t x_pos, uint32_t y_pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
            if(x_pos > width - 1 || y_pos > height - 1 || !initialized)
                return;
            pixel_data[get_index(x_pos, y_pos)+0] -= blue;
            pixel_data[get_index(x_pos, y_pos)+1] -= green;
            pixel_data[get_index(x_pos, y_pos)+2] -= red;
            pixel_data[get_index(x_pos, y_pos)+3] -= alpha;
        }

        void subfpixel(uint32_t x_pos, uint32_t y_pos, color val){
            subfpixel(x_pos, x_pos, val.get_red(), val.get_green(), val.get_blue(), val.get_alpha());
        }

        //return the r-g-b-a value of specified pixel (3rd param: 0=b, 1=g, 2=r, 3=a)
        uint8_t get_pixel(uint32_t x_pos, uint32_t y_pos, uint8_t color_index){
            if(color_index > 3 || !initialized)
                return 0;
            return pixel_data[get_index(x_pos, y_pos) + color_index];
        }

        color get_pixel(uint32_t x_pos, uint32_t y_pos){
            return color(get_pixel(x_pos, y_pos, 2) << 24 | get_pixel(x_pos, y_pos, 1) << 16 | get_pixel(x_pos, y_pos, 0) << 8 | get_pixel(x_pos, y_pos, 3));
        }

        //changes the size of the image
        void resize(uint32_t set_width, uint32_t set_height){
            if(!initialized)
                return;
            if(set_width == width && set_height == height)
                return;
            if(set_width < width || set_height < height)
                return;

            width = set_width;
            height = set_height;

            total_size_in_bytes = pixel_data_offset + height * width * 4;
            raw_data_size = height * width * 4;

            pixel_data = (uint8_t*)realloc (pixel_data, raw_data_size);
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
        void reset(){
            if(!initialized)
                return;

            width = 0;
            height = 0;
            total_size_in_bytes = 0;
            raw_data_size = 0;

            pixel_data = (uint8_t*)realloc (pixel_data, 0);

            initialized = false;

            //return true;
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
            //missing header data(added since ver exp 0.34)
            out_image.write((char *)&red_channel_bit_mask, 4);
            out_image.write((char *)&green_channel_bit_mask, 4);
            out_image.write((char *)&blue_channel_bit_mask, 4);
            out_image.write((char *)&alpha_channel_bit_mask, 4);
            out_image.write((char *)&*color_space, 4);
            out_image.write((char *)&*color_space_endpoints, 36);
            out_image.write((char *)&*gamma_rgb, 12);

            //much better way to write pixel data(since ver exp 0.27)
            out_image.write((char *)&*pixel_data, raw_data_size);

            out_image.close();

            return true;
        }

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
            if(bm_header[0] != 'B' && bm_header[1] != 'M')
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
            in_image.read((char *)&width, 4);
            //std::cout << width << '\n';
            in_image.read((char *)&height, 4);
            //std::cout << height << '\n';
            in_image.ignore(2); //ignore color planes
            in_image.read((char *)&b_per_pix, 2);
            //std::cout << b_per_pix << '\n';
            if(b_per_pix != 32)
                return false;
            in_image.ignore(4); //ignore Bl_RGB

            //check if raw data size is correct and if  not correct it
            in_image.read((char *)&raw_data_size, 4);
            if(raw_data_size != total_size_in_bytes - offset)
                raw_data_size = total_size_in_bytes - offset;

            //std::cout << raw_data_size << '\n';

            //jump to image data location
            in_image.seekg(offset);
            //std::cout << 1;
            //allocate memory and load the image data
            pixel_data = (uint8_t*)calloc (raw_data_size, sizeof(uint8_t));
            //std::cout << 1;
            in_image.read((char *)&*pixel_data, raw_data_size);
            //std::cout << 1;

            in_image.close();
            //std::cout << 1;

            initialized = true;
            //std::cout << 1;

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
        //missing header data(added since ver exp 0.34)
        const uint32_t red_channel_bit_mask = 0x00ff0000;
        const uint32_t green_channel_bit_mask = 0x0000ff00;
        const uint32_t blue_channel_bit_mask = 0x000000ff;
        const uint32_t alpha_channel_bit_mask = 0xff000000;
        const char color_space[4] = {' ', 'n', 'i', 'W'};
        const uint8_t color_space_endpoints[36] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        const uint32_t gamma_rgb[3] = {0, 0, 0};

        //const uint8_t padding = 0;
        //uint8_t padding_size;
        uint8_t * pixel_data = (uint8_t*)malloc (0);
        bool initialized = false;

        //function used to get the array index of any pixel
        uint64_t get_index(uint32_t x_pos, uint32_t y_pos){
            return ((width - y_pos - 1) * width + x_pos) * 4; //y_pos is inverted because of the way the image is stored
        }
    };



    #define sbtmp_black 0x000000ff
    #define sbtmp_white 0xffffffff
    #define sbtmp_red 0xff0000ff
    #define sbtmp_dark_red 0x800000ff
    #define sbtmp_green 0x00ff00ff
    #define sbtmp_dark_green 0x008000ff
    #define sbtmp_blue 0x0000ffff
    #define sbtmp_dark_blue 0x000080ff
    #define sbtmp_purple 0xff00ffff
    #define sbtmp_dark_purple 0x800080ff
    #define sbtmp_yellow 0xffff00ff
    #define sbtmp_orange 0xffA500ff
    #define sbtmp_cyan 0x00ffffff
}