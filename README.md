# simple-bitmap
Adds very simple functions for bitmap creation, manipulation and saving

This is my first GitHub repo and also the first time describing my functions in detail. 

The goal of this project was to create a very simple to use and fast library. I started this project because I needed a simple library to render and save some images. There a basically 2 options when it comes to such image librarys. First are very complicated libarys that require initialisations and have 1000s of functions that you have to study to get anything done with them. The second option are small very very outdated and mostly limited librarys.
Because I couldn't find a library that fits my needs, I wrote my own. It only has 21 functions and does not have a real size limit. Images can be up to 32768x32768 pixels in size!

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
