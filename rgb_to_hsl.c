
/******************************************************************************
NAME:        RGB2HSL
 
PURPOSE      Process red, green and blue bands (RGB) to hue, saturation and
             lightness (HSL)
  
             Get red, green, blue from input buffer
             Create the HSL bands
             Write to output buffer as FCELL
   
ASSUMPTION:  The input images are read to the input buffer.
 
******************************************************************************/
/* For GRASS one row from each cell map is passed in and each cell in
   each band is processed and written out.   CWU GIS Lab: DBS 8/90 */

#include <grass/gis.h>
#include <grass/glocale.h>
#include "globals.h"
#include <math.h>

/*
 * (input/output) rowbuffer
 * columns
 * maximum level of range 2^bit_depth
 */

void rgb_to_hsl(DCELL *rowbuffer[3], int columns, double max_level)

{
    int column;         /* column indicator               */
    double red;         /* the red band output            */
    double green;       /* the green band output          */
    double blue;        /* the blue band output           */
    double min;         /* minimum among red, green, blue */
    double max;         /* maximum among red, green, blue */
    float chroma;       /* chrome, intermediate value     */
    float lightness;    /* lightness                      */
    float saturation;   /* saturation                     */
    float hue = 0.0L;   /* hue                            */

for (column = 0; column < columns; column++) {

    if (Rast_is_d_null_value(&rowbuffer[0][column]) ||
        Rast_is_d_null_value(&rowbuffer[1][column]) ||
        Rast_is_d_null_value(&rowbuffer[2][column])) {
            Rast_set_d_null_value(&rowbuffer[0][column], 1);
            Rast_set_d_null_value(&rowbuffer[1][column], 1);
            Rast_set_d_null_value(&rowbuffer[2][column], 1);
            continue;
        }

    /* scale r, g, b to [0.0,1.0] */

    red = rowbuffer[0][column];
    red /= max_level;

    green = rowbuffer[1][column];
    green /= max_level;

    blue = rowbuffer[2][column];
    blue /= max_level;

    /* max of {r,g,b} */

    max = red;
    if (green > max)
        max = green;

    if (blue > max)
        max = blue;


    /* min of {r,g,b} */

    min = red;
    if (green < min)
        min = green;

    if (blue < min)
        min = blue;


    /* chroma and lightness */

    chroma = max - min;
    lightness = ((max + min) / 2.0);


    /* if R == G == B, then min == max, which is achromatic */

    if (chroma == 0.0) {

        saturation = 0.0;

        /* undefined hue, set to -1. */

        hue = -1.0;

    }

    /*  else chromatic */

    else if (chroma != 0.0) {

        saturation = chroma / (1.0 - fabs(2.0 * lightness - 1.0));

        if (red == max)
            hue = fmod(((green - blue) / chroma), 6.0);

        else if (green == max)
            hue = ((blue - red) / chroma) + 2.0;

        else if (blue == max)
            hue = ((red - green) / chroma) + 4.0;

        /* convert hue to degrees */
        hue *= 60.0;

        /* make nonnegative */
        if (hue < 0.0)
            hue += 360.0;

    }

    G_debug(2, "Maximum level among r, g, b: %f", max);
    G_debug(2, "Minimum level among r, g, b: %f", min);
    G_debug(2, "Lightness: %f", lightness);

    /* HSL output values */

    /* set hue = -1 to NULL */
    if (hue == -1.0) {
        Rast_set_d_null_value(&rowbuffer[0][column], 1);
    }
    else
    {
        rowbuffer[0][column] = (FCELL)hue;
    }

    /* set saturation, lightness */
    rowbuffer[1][column] = (FCELL)saturation;
    rowbuffer[2][column] = (FCELL)lightness;

    G_debug(3, "Output rowbuffers 0, 1, 2: %f, %f, %f\n",
            rowbuffer[0][column], rowbuffer[1][column], rowbuffer[2][column]);
  }
}