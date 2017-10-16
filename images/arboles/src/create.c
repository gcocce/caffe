#include <cairo.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define TRAINING_SET   	10000
#define TEST_SET		2000
#define VALIDATION_SET	200

#define WIDTH 300
#define HEIGHT 300
#define SIZE           300   // determines size of window

#define PI 3.141592653589793

#define SCALE          3     // determines how quickly branches shrink (higher value means faster shrinking)
#define BRANCHES       9    // number of branches
#define ROTATION_SCALE 0.75  // determines how slowly the angle between branches shrinks (higher value means slower shrinking)
#define INITIAL_LENGTH 5    // length of first branch

struct stat st = {0};

void dibujar_circulo(cairo_t *cr);
void dibujar_linea(cairo_t *cr);

void dibujar_patron_de_lineas(cairo_t *cr, cairo_surface_t *surface, int random);

void draw_tree(cairo_surface_t *surface , double offsetx, double offsety,
               double directionx, double directiony, double size,
               double rotation, int depth);

double rand_fl(){
  return (double)rand() / (double)RAND_MAX;
}

void line(cairo_t  * cr, double x1, double y1, double x2, double y2){
		cairo_move_to(cr, (int)x1, (int)y1);
		cairo_line_to(cr, (int)x2, (int)y2);
		cairo_stroke(cr);		
}

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

  //surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
  //cr = cairo_create(surface);

  srand(time(NULL));   // should only be called once

  FILE *f = fopen("train_file.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  printf("Se crean las imagenes de entrenamiento.");
  fflush(stdout);

  for (int x=0;x<TRAINING_SET; x++){
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
	cr = cairo_create(surface);	  

    printf(".");  
    fflush(stdout);

    // Dibujar imagen con arcos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_circulo(cr);
    }
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    
    //*********************************************************************
    // Dibujar patron de lineas
    //cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    //cairo_paint (cr);  
    //cairo_set_source_rgb(cr, 0, 0, 0);

    int random_number = (int) rand() % 2;

    dibujar_patron_de_lineas(cr, surface, random_number);

    sprintf(str, "%d", x);
    strcpy(file_name, "project/img/muestra");
    strcat(file_name, str);
    strcat(file_name, ".png");
    
    cairo_status_t status= cairo_surface_write_to_png(surface, file_name);

    if (status!=CAIRO_STATUS_SUCCESS){
      printf("Error cairo_surface_write_to_png: %d\n", status);
      exit(1);
    }
    
    if (random_number==0){
        fprintf(f, "%s 0\n", file_name);
    }else{
//		sprintf(str, "%d", x);
//		strcpy(file_name, "project/img/arbol");
//		strcat(file_name, str);
//		strcat(file_name, ".png");
//        cairo_status_t status= cairo_surface_write_to_png(surface, file_name);
//
//        if (status!=CAIRO_STATUS_SUCCESS){
//          printf("Error cairo_surface_write_to_png: %d\n", status);
//          exit(1);
//        }        

		fprintf(f, "%s 1\n", file_name);
    } 
    
    cairo_destroy(cr);
    cairo_surface_destroy (surface);
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

  for (int x=0;x<TEST_SET; x++){
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
	cr = cairo_create(surface);	  

    printf("."); 
    fflush(stdout); 

    // Dibujar imagen con arcos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* blank to white */
    cairo_paint (cr);  

    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_circulo(cr);
    }
    
    cairo_set_source_rgb(cr, 0, 0, 0);
    
    //*********************************************************************
    // Dibujar patron de lineas
    //cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    //cairo_paint (cr);  
    //cairo_set_source_rgb(cr, 0, 0, 0);

    int random_number = (int) rand() % 2;

    dibujar_patron_de_lineas(cr, surface, random_number);
    
    if (random_number==0){
        sprintf(str, "%d", x);
        strcpy(file_name, "project/test/lineas");
        strcat(file_name, str);
        strcat(file_name, ".png");
        cairo_surface_write_to_png(surface, file_name);
        fprintf(f, "%s 0\n", file_name);
    }else{
		sprintf(str, "%d", x);
		strcpy(file_name, "project/test/arbol");
		strcat(file_name, str);
		strcat(file_name, ".png");
		cairo_surface_write_to_png(surface, file_name);
		fprintf(f, "%s 1\n", file_name);
    }  
    
    cairo_destroy(cr);
    cairo_surface_destroy (surface);
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

  for (int x=0;x<VALIDATION_SET; x++){
	  
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
	cr = cairo_create(surface);	  
	
	

    printf(".");
    fflush(stdout);

    // Dibujar imagen con arcos
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5); /* grey */
    cairo_paint (cr);  
    cairo_set_source_rgb(cr, 0, 0, 0);

    for (int y=0;y<10; y++){
      dibujar_circulo(cr);
    }
    
    cairo_set_source_rgb(cr, 0, 0, 0);

    //*********************************************************************
    // Dibujar patron de lineas
    //cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    //cairo_paint (cr);  
    //cairo_set_source_rgb(cr, 0, 0, 0);

    int random_number = (int) rand() % 2;

    dibujar_patron_de_lineas(cr, surface, random_number);
    
    if (random_number==0){
        sprintf(str, "%d", x);
        strcpy(file_name, "project/validate/basura");
        strcat(file_name, str);
        strcat(file_name, ".png");
        cairo_surface_write_to_png(surface, file_name);
        fprintf(f, "%s 0\n", file_name);
    }else{
		sprintf(str, "%d", x);
		strcpy(file_name, "project/validate/arbol");
		strcat(file_name, str);
		strcat(file_name, ".png");
		cairo_surface_write_to_png(surface, file_name);
		fprintf(f, "%s 1\n", file_name);
    }
    
    cairo_destroy(cr);
    cairo_surface_destroy (surface);
  }  

  fclose(f);


  //cairo_destroy(cr);
  //cairo_surface_destroy(surface);

  printf("\nFinish\n");

  return 0;
}

void dibujar_circulo(cairo_t *cr){

	cairo_pattern_t *pat;
	
    /* random int between 1 and WIDTH*2/3 */
    int r = rand() % 20 + 10;

    /* random int between 1 and 360 */
    int angle = 360;

    double xc = rand() % (WIDTH-60) + 30;
    double yc = rand() % (HEIGHT-60) + 30;

    double radius = (double)r;
    double angle1 = 0  * (PI/180.0);  /* angles are specified */
    double angle2 = angle * (PI/180.0);  /* in radians           */
    
    int random_number = (int) rand() % 2;

	pat = cairo_pattern_create_radial (xc - (int)radius/ 4, yc - (int)radius/2, (int)radius/4,
                                   xc - (int)radius/2,  yc-(int)radius/2, (int)radius*2);
    
    if(random_number==0){
        cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
        cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
    }else{
        cairo_pattern_add_color_stop_rgba (pat, 0, 0, 0, 0, 1);
    	cairo_pattern_add_color_stop_rgba (pat, 1, 1, 1, 1, 1);
    }
    
    
    cairo_set_source (cr, pat);
    //cairo_arc (cr, 128.0, 128.0, 76.8, 0, 2 * PI);
    cairo_arc (cr, xc, yc, radius, 0, 2 * PI);
    cairo_fill (cr);
    cairo_pattern_destroy (pat);    
    

    //cairo_set_line_width (cr, 1.0);
    //cairo_stroke (cr);
}

void dibujar_linea(cairo_t *cr){
    //cairo_move_to(cr, xc, yc);
    /* random int between 1 and WIDTH*2/3 */
    int alpha = (int)rand() % 360;
    int r =(int) rand() % INITIAL_LENGTH + 5;
    
    double x1c = rand() % WIDTH + 1;
    double y1c = rand() % HEIGHT + 1;

    double x2c = x1c + cos(alpha * PI / 180) * r;
    double y2c = y1c + sin(alpha * PI / 180) * r;

    cairo_set_line_width (cr, 1.0);

    cairo_move_to(cr, x1c, y1c);
    cairo_line_to(cr, x2c, y2c);    
    cairo_stroke (cr);
}


double DegreesToRadians( double degrees )
{
    return((double)((double)degrees * ( (double)M_PI/(double)180.0 )));
}

void dibujar_patron_de_lineas(cairo_t *cr, cairo_surface_t *surface, int random){

	int lineas=rand() % 200 + 200;
	
    for (int y=0;y<lineas; y++){
      dibujar_linea(cr);
    }
    
	if (random==1){
		int initial_length = rand() % INITIAL_LENGTH + 30;
		
		int x = WIDTH / 2.0 + rand() % 20 - rand() % 20; 
		int y = HEIGHT /2.0 - rand() % 10 + 1 + rand() % 10 + 1;
		
		// Random entre 1 y -1
		float directionx= (rand_fl() - 0.5) * 2;
		float directiony= (rand_fl() - 0.5) * 2;
		
		draw_tree(surface, x, y, directionx, directiony, initial_length, PI / (rand() % 4 + 6),BRANCHES);
		//draw_tree(surface, x, y, directionx, directiony, initial_length, PI / 8,BRANCHES);
	}
		
}

void draw_tree(cairo_surface_t *surface , double offsetx, double offsety, double directionx, double directiony, double size,
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
  
  cairo_destroy(ct);
  cairo_surface_destroy (surf);
  //free (my_data);
}
