# simple-bitmap
Very simple functions for bitmap creation, manipulation and saving

This is my first (public) GitHub repo.

# Usage:

## creating / resizing / resetting / clearing / saving:

### Creating a bitmap

//creates a new bitmap "test"
bitmap test;


### Initializing a bitmap

//Sets width and height of the bitmap
test.create(<width>, <height>);


### Resizing a bitmap

//Sets width and height of the bitmap
test.resize(<width>, <height>);
  
### Clearing a bitmap
  
//clears the image (makes it black) but keeps the dimensions
test.clear();
  
### Resetting a bitmap

//This will free all bitmap data and reset the dimensions
test.reset();
  
### Saving a bitmap
  
//<string> not included  
test.save((char*)"abcd.bmp");
//<string> included  
test.save("abcd.bmp");          
  

## editing:

### changing pixels
  
//sets the pixel at (x_pos|y_pos) to color value  
test.set_pixel(<x_pos>, <y_pos>, <red>, <green>, <blue>, <alpha>);    
  
### filling the image
  
//fills the entire image with color value  
test.fill(<red>, <green>, <blue>, <alpha>);     
  
### drawing rectangles
  
//fills area (x1|y1) to (x2|y2) with color value  
test.rect(<x1>, <y1>, <x2>, <y2>, <red>, <green>, <blue>, <alpha>);     
  
### drawing circles
  
test.circle(<x_pos>, <y_pos>, <radius>, <red>, <green>, <blue>, <alpha>);

  
### I don't have time to continue writing right now. The rest will come eventually...
