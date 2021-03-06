#ifndef _utilz__
#define _utilz__
#include "CImg.h"
#include <map>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <math.h>


// helper function that adds pixels to the border around the image
cimg_library::CImg<> add_padding_to_image(cimg_library::CImg<> image, int pixels) {
    cimg_library::CImg<> padded_image(image.width() + 2*pixels, image.height() + 2*pixels, 1, image.spectrum());
    for (int channel = 0; channel < image.spectrum(); channel++) {
        for (int r = 0; r < image.height(); r++) {
            for (int c = 0 ; c < image.width(); c++) {
                padded_image(r + pixels, c + pixels, 0, channel) = (int) image(r, c, 0, channel);
            }
        }
    }
    return padded_image;
}

//applies the given filter to the image (1st param) inside the padding and produces an output image
cimg_library::CImg<> apply_sobel_filter(cimg_library::CImg<> image, float filter[3][3], int padding) {

    cimg_library::CImg<> output(image.width(), image.height(), 1, image.spectrum());

    for (int channel = 0; channel < image.spectrum(); channel++) {

        //loop through each pixel INSIDE the padding
        for (int r = padding ; r < image.height() - padding; r++) {
            for (int c = padding ; c < image.width() - padding; c++) {

                float value =
                    filter[0][0] * (float) image(r - 1, c - 1, 0, channel) +
                    filter[0][1] * (float) image(r - 1, c, 0, channel) +
                    filter[0][2] * (float) image(r - 1, c + 1, 0, channel) +
                    filter[1][0] * (float) image(r, c - 1, 0, channel) +
                    filter[1][1] * (float) image(r, c, 0, channel) +
                    filter[1][2] * (float) image(r, c + 1, 0, channel) +
                    filter[2][0] * (float) image(r + 1, c - 1, 0, channel) +
                    filter[2][1] * (float) image(r + 1, c, 0, channel) +
                    filter[2][2] * (float) image(r + 1, c + 1, 0, channel);

                //scale the value to [0, 255];

                value = (value + 255) / 510 * 255;

                output(r, c, 0, channel) = (int) value;
            }
        }
    }
    return output;
}




// Given the x & y gradients, compute the magnitude
float get_magnitude(int ix, int iy) {
    int value = sqrt( pow(ix, 2) + pow(iy, 2));

    //scale the value from [0, 360.6] to [0, 255]
    return 255*(value/360.6);
}


// Computes the magnitude with magnitude = sqrt(Ix^2 + Iy^2). For color images, each channel is
// computed individually so the resulting image can be displayed as RGB

cimg_library::CImg<> compute_magnitude(cimg_library::CImg<> Ix, cimg_library::CImg<> Iy) {
    cimg_library::CImg<> output(Ix.width(), Ix.height(), 1, Ix.spectrum());
    for (int channel = 0; channel < Ix.spectrum(); channel++) {
        for (int r = 0; r < Ix.height(); r++) {
            for (int c = 0 ; c < Ix.width(); c++) {
                output(r, c, channel) = (int) get_magnitude(Ix(r, c, channel), Iy(r, c, channel));
            }
        }
    }
    return output;
}


// color LUT using HSV to RGB conversion where H = theta (the angle from (b) properly transformed), S = 1.0, V= 1.0
cimg_library::CImg<> compute_orientation(cimg_library::CImg<> input_image, cimg_library::CImg<> Ix, cimg_library::CImg<> Iy) {
    //create an RGB image for displaying the orientation
    cimg_library::CImg<> output(input_image);



    int channel = 0;
    for (int r = 0; r < Ix.height(); r += 10) {
        for (int c = 0 ; c < Ix.width(); c += 10) {


            int Iy_value = Iy(r, c, channel);
            int Ix_value = Ix(r, c, channel);

            if (Iy_value == 0 && Ix_value == 0) {

            } else {
            	//angle in radians
                float angle = atan2(Iy_value, Ix_value); // * 180 / cimg_library::cimg::PI;

                //ensure value is always positive
                // if (angle < 0) angle += 360;

                unsigned char randomColor[3];
                randomColor[0] = rand() % 256;
                randomColor[1] = rand() % 256;
                randomColor[2] = rand() % 256;


                float magnitude = get_magnitude(Ix_value, Iy_value) / 255.0 * 10.0;

                int y2 = magnitude * sin(angle);
                int x2 = magnitude * cos(angle);

                //compute the arrow x2 y2 coordinates

                //maximum length of arrow is 10px

                //draw an arrow
                output.draw_arrow(r, c, r - y2, x2 + c, randomColor);
            }
        }
    }
    return output;
}

void detect_edges(cimg_library::CImg<> input_image) {
    /** add padding to input image **/

    int padding = 2;
    cimg_library::CImg<> padded_image = add_padding_to_image(input_image, padding);

    /** define sobel filters **/
    float G_x[3][3] = {
      {0.25, 0, -0.25},
      {0.50, 0, -0.50},
      {0.25, 0, -0.25}
    };

    float G_y[3][3] = {
      { 0.25,  0.50,  0.25},
      { 0,     0,     0},
      {-0.25, -0.50, -0.25}
    };


    // /** Apply sobel filters to image **/
    cimg_library::CImg<> Ix = apply_sobel_filter(padded_image, G_x, padding);
    cimg_library::CImg<> Iy = apply_sobel_filter(padded_image, G_y, padding);

    Ix.display("Ix");
    Iy.display("Iy");


    // Calculate the edge magnitude
    cimg_library::CImg<> magnitude = compute_magnitude(Ix, Iy);
    magnitude.display("magnitude");

    //For grayscale images, compute the orientation
    if (input_image.spectrum() == 1) {
        cimg_library::CImg<> color_orientation_plot = compute_orientation(padded_image, Ix, Iy);
        color_orientation_plot.display("Orientation");
    }


}

#endif