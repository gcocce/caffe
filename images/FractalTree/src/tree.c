#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PI 3.1415926535
#define SIZE           800   // determines size of window
#define SCALE          5     // determines how quickly branches shrink (higher value means faster shrinking)
#define BRANCHES       14    // number of branches
#define ROTATION_SCALE 0.75  // determines how slowly the angle between branches shrinks (higher value means slower shrinking)
#define INITIAL_LENGTH 50    // length of first branch

#define WIDTH 800
#define HEIGHT 800
 
double rand_fl(){
  return (double)rand() / (double)RAND_MAX;
}
 
void draw_tree(cairo_surface_t *surface , double offsetx, double offsety,
               double directionx, double directiony, double size,
               double rotation, int depth) {

  int stride;
  stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, WIDTH);
  unsigned char *my_data = cairo_image_surface_get_data (surface);

  cairo_surface_t *surf = cairo_image_surface_create_for_data(my_data, CAIRO_FORMAT_RGB24, WIDTH, HEIGHT, stride);

  cairo_t *ct = cairo_create(surf);
 
  cairo_set_line_width(ct, 1);
  cairo_set_source_rgba(ct, 0,0,0,1);
  cairo_move_to(ct, (int)offsetx, (int)offsety);
  cairo_line_to(ct, (int)(offsetx + directionx * size), (int)(offsety + directiony * size));
  cairo_stroke(ct);

  if (depth > 0){
    // draw left branch
    draw_tree(surf,
        offsetx + directionx * size,
        offsety + directiony * size,
        directionx * cos(rotation) + directiony * sin(rotation),
        directionx * -sin(rotation) + directiony * cos(rotation),
        size * rand_fl() / SCALE + size * (SCALE - 1) / SCALE,
        rotation * ROTATION_SCALE,
        depth - 1);
 
    // draw right branch
    draw_tree(surf,
        offsetx + directionx * size,
        offsety + directiony * size,
        directionx * cos(-rotation) + directiony * sin(-rotation),
        directionx * -sin(-rotation) + directiony * cos(-rotation),
        size * rand_fl() / SCALE + size * (SCALE - 1) / SCALE,
        rotation * ROTATION_SCALE,
        depth - 1);
  }
}
 
void render(int index){

  char str[20];
  char file_name[128];  

  sprintf(str, "%d", index);

  strcpy(file_name, "tree_");
  strcat(file_name, str);
  strcat(file_name, ".png");

  cairo_surface_t *surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, WIDTH, HEIGHT);

  cairo_t *cr;
  cr = cairo_create(surface);
  cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
  cairo_paint (cr);

  int initial_length = rand() % INITIAL_LENGTH + 10;
  
  draw_tree(surface, WIDTH / 2.0,
      HEIGHT - 10.0,
      0.0, -1.0,
      initial_length,
      PI / (rand() % 16 + 4),
      BRANCHES);

  cairo_status_t status= cairo_surface_write_to_png(surface, file_name);

  if (status!=CAIRO_STATUS_SUCCESS){
    printf("Error cairo_surface_write_to_png: %d\n", status);
    exit(1);
  }

}
 
int main(void){
 
  srand((unsigned)time(NULL));

  for (int x=0; x<10; x++){
    render(x);  
  }

  return 0;
}