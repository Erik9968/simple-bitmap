#include "sbtmp2.0_base.hpp"

//shut up clangd, I won't use "namespace sbtmp{ namespace formats{ }}"
namespace sbtmp::formats{
    class Bitmap32 : public sbtmp::base::image{
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
    
        Bitmap32() = default;

        ~Bitmap32(){
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
        void set_pixel(int32_t x, int32_t y, color::Color col) override {
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

        //returns size of the raw data array in bytes
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
}