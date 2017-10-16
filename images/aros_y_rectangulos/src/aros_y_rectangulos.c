/* 
* Aros y Rectangulos
*
*
*/

#include <cairo.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define M_PI 3.141592653589793
#define WIDTH 200
#define HEIGHT 200

struct stat st = {0};

void dibujar_aro(cairo_t *cr);
void dibujar_rectangulo(cairo_t *cr);

int main(void)
{
  char str[20];
  char file_name[128];
  cairo_surface_t *surface;
  cairo_t *cr;

  if (stat("project", &st) == -1) {
    printf("Se crea el directorio project...\n");
    int res=mkdir("project", 0700);
    if (res!=0){
      printf("res: %d\n",res);    
      perror("mkdir");
    }
  }

  if (stat("project/img", &st) == -1) {
    printf("Se crea el directorio img...\n");  
    mkdir("project/img", 0700);
  }

  if (stat("project/test", &st) == -1) {
    printf("Se crea el directorio test...\n");  
    mkdir("project/test", 0700);
  }  

  if (stat("project/validate", &st) == -1) {
    printf("Se crea el directorio validate...\n");  
    mkdir("project/validate", 0700);
  } 

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
  cr = cairo_create(surface);

  srand(time(NULL));   // should only be called once



  FILE *f = fopen("train_file.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  printf("Se crean las imagenes de entrenamiento.");
  fflush(stdout);

  for (int x=0;x<10000; x++){

    printf(".");  
    fflush(stdout);

    // Dibujar imagen con arcos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_aro(cr);
    }
    
    sprintf(str, "%d", x);

    strcpy(file_name, "project/img/aro");
    strcat(file_name, str);
    strcat(file_name, ".png");

    cairo_status_t status= cairo_surface_write_to_png(surface, file_name);

    if (status!=CAIRO_STATUS_SUCCESS){
      printf("Error cairo_surface_write_to_png: %d\n", status);
      exit(1);
    }

    fprintf(f, "%s 0\n", file_name);


    //*********************************************************************
    // Dibujar imagen con líneas
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_rectangulo(cr);
    }
    
    sprintf(str, "%d", x);

    strcpy(file_name, "project/img/rect");
    strcat(file_name, str);
    strcat(file_name, ".png");

    cairo_surface_write_to_png(surface, file_name);

    fprintf(f, "%s 1\n", file_name);    
  }

  fclose(f);

  printf("\n");  

  f = fopen("test_file.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  printf("Se crean las imagenes de testing.");    
  fflush(stdout);

  for (int x=0;x<2000; x++){

    printf("."); 
    fflush(stdout); 

    // Dibujar imagen con arcos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_aro(cr);
    }
    
    sprintf(str, "%d", x);

    strcpy(file_name, "project/test/aro");
    strcat(file_name, str);
    strcat(file_name, ".png");

    cairo_surface_write_to_png(surface, file_name);


    fprintf(f, "%s 0\n", file_name);


    //*********************************************************************
    // Dibujar imagen con líneas
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_rectangulo(cr);
    }
    
    sprintf(str, "%d", x);

    strcpy(file_name, "project/test/rect");
    strcat(file_name, str);
    strcat(file_name, ".png");

    cairo_surface_write_to_png(surface, file_name);

    fprintf(f, "%s 1\n", file_name);    
  }  

  fclose(f);

  printf("\n");

  f = fopen("validate_file.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  printf("Se crean las imagenes de validacion.");
  fflush(stdout);

  for (int x=0;x<100; x++){

    printf(".");
    fflush(stdout);

    // Dibujar imagen con arcos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_aro(cr);
    }
    
    sprintf(str, "%d", x);

    strcpy(file_name, "project/validate/aro");
    strcat(file_name, str);
    strcat(file_name, ".png");

    cairo_surface_write_to_png(surface, file_name);


    fprintf(f, "%s 0\n", file_name);


    //*********************************************************************
    // Dibujar imagen con rectangulos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_rectangulo(cr);
    }
    
    sprintf(str, "%d", x);

    strcpy(file_name, "project/validate/rect");
    strcat(file_name, str);
    strcat(file_name, ".png");

    cairo_surface_write_to_png(surface, file_name);

    fprintf(f, "%s 1\n", file_name);    
  }  

  fclose(f);


  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  printf("\nFinish\n");

  return 0;
}

void dibujar_rectangulo(cairo_t *cr){

    double x1c = rand() % (WIDTH - 60 )+ 1;
    double y1c = rand() % (HEIGHT - 60 )+ 1;

    double x2c = rand() % 60 + 1;
    double y2c = rand() % 60 + 1;

    x2c = x2c + x1c;
    y2c = y2c + y1c;

    cairo_set_line_width (cr, 1.0);

    cairo_move_to(cr, x1c, y1c);
    cairo_line_to(cr, x1c, y2c);
    cairo_line_to(cr, x2c, y2c);
    cairo_line_to(cr, x2c, y1c);
    cairo_line_to(cr, x1c, y1c);
    cairo_stroke (cr);
}

void dibujar_aro(cairo_t *cr){

    /* random int between 1 and WIDTH*2/3 */
    int r = rand() % 20 + 10;

    /* random int between 1 and 360 */
    int angle = 360;

    double xc = rand() % (WIDTH-60) + 30;
    double yc = rand() % (HEIGHT-60) + 30;

    double radius = (double)r;
    double angle1 = 0  * (M_PI/180.0);  /* angles are specified */
    double angle2 = angle * (M_PI/180.0);  /* in radians           */

    cairo_set_line_width (cr, 1.0);
    cairo_arc (cr, xc, yc, radius, angle1, angle2);
    cairo_stroke (cr);
}
