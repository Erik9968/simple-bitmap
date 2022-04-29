# simple-bitmap
Adds very simple functions for bitmap creation, manipulation and saving

This is my first GitHub repo and also the first time describing my functions in detail. 

The goal of this project was to create a very simple to use and fast library. I started this project because I needed a simple library to render and save some images. There a basically 2 options when it comes to such image librarys. First are very complicated libarys that require initialisations and have 1000s of functions that you have to study to get anything done with them. The second option are small very very outdated and mostly limited librarys.
Because I couldn't find a library that fits my needs, I wrote my own. It only has 21 functions and does not have a real size limit. Images can be up to 32768x32768 pixels in size!

# Usage:

## creating / resizing / resetting / clearing / saving:

### Creating a bitmap

bitmap test;          //creates a new bitmap "test"


### Initializing a bitmap

test.create(<width>, <height>);          //Sets width and height of the bitmap


### Resizing a bitmap

test.resize(<width>, <height>);   //Sets width and height of the bitmap
  
### Clearing a bitmap
  
test.clear();   //clears the image (makes it black) but keeps the dimensions
  
### Resetting a bitmap
  
test.reset();   //This will free all bitmap data and reset the dimensions
  
### Saving a bitmap
  
test.save((char*)"abcd.bmp");   //<string> not included
test.save("abcd.bmp");          //<string> included
  

## editing:

### changing pixels
  
test.set_pixel(<x_pos>, <y_pos>, <red>, <green>, <blue>, <alpha>);    //sets the pixel at (x_pos|y_pos) to color value
  
### filling the image
  
test.fill(<red>, <green>, <blue>, <alpha>);     //fills the entire image with color value
  
### drawing rectangles
  
test.rect(<x1>, <y1>, <x2>, <y2>, <red>, <green>, <blue>, <alpha>);     //fills area (x1|y1) to (x2|y2) with color value
  
### drawing circles
  
test.circle(<x_pos>, <y_pos>, <radius>, <red>, <green>, <blue>, <alpha>);

  
### I don't have time to continue writing right now. The rest will come eventually...
