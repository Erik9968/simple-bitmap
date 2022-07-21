# simple-bitmap
Very simple functions for bitmap creation, manipulation and saving

This is my first (public) GitHub repo.

# Usage:

## creating / resizing / resetting / clearing / saving:

### Creating a bitmap

//create a new bitmap "test"  
bitmap test;


### Initializing a bitmap

//Set width and height of the bitmap  
test.create(width​, height​);

### Resizing a bitmap

//Set width and height of the bitmap  
test.resize(width​ , height​);
  
### Clearing a bitmap
  
//clear the image (makes it black) but keeps the dimensions  
test.clear();
  
### Resetting a bitmap

//This will free all bitmap data and reset the dimensions  
test.reset();
  
### Saving a bitmap

//string​ not included  
test.save((char*)"abcd.bmp");  
//string​ included  
test.save("abcd.bmp");  

## editing:

### changing pixels
  
//set the pixel at (x_pos | y_pos)  
test.set_pixel(x_pos, y_pos, red​, green​, blue​, alpha​);
  
### filling the image
  
//fill the entire image  
test.fill(red​, green​, blue​, alpha​);
  
### drawing rectangles
  
//fill area (x1|y1) to (x2|y2)  
test.rect(x1​, y1​, x2​, y2​, red​, green​, blue​, alpha​);
  
### drawing circles

//draw circle at (x_pos | y_pos)  
test.circle(x_pos, y_pos, radius​, red​, green​, blue​, alpha​);

### lines, triangles, rectangle borders are not implemented yet

## filters:

### convert to black and white

//convert a section of the image to black and white  
test.convert_bw(x_pos, y_pos, width, height);

### invert colors

//invert the color of a section of the image  
test.rgb_invert(x_pos, y_pos, width, height);

### flipping images

//flip image horizontally  
test.flip_horizontal();

//flip image vertically  
test.flip_vertically();

## extra functions:

### add and subtract rgb value to/from pixel

//add rgb value to pixel  
test.addtpixel(x_pos, y_pos, red, green, blue, alpha);

//subtract rgb value from pixel  
test.subfpixel(x_pos, y_pos, red, green, blue, alpha);

### get pixel rgb value

//color_index = 0..3 > blue - green - red - alpha  
test.get_pixel(x_pos, y_pos, color_index);

### get image dimensions and size

//get image width  
test.get_width();

//get image heigth  
test.get_height();

//get raw data size  
test.get_size();

//get entire file size(includes header)  
test.get_total_size();