#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
 
#define WIDTH 300
#define HEIGHT 300

typedef struct{
	double x,y;
}point;

double rand_fl(){
  return (double)rand() / (double)RAND_MAX;
}

void line(cairo_t  * cr, double x1, double y1, double x2, double y2){
		cairo_move_to(cr, (int)x1, (int)y1);
		cairo_line_to(cr, (int)x2, (int)y2);
		cairo_stroke(cr);		
}
 
void pythagorasTree(cairo_t *cr, point a,point b,int times){
 
 	printf("Iter: %d a: %f %f  b: %f %f\n", times, a.x, a.y, b.x, b.y);

	point c,d,e;
 
	c.x = b.x - (a.y -  b.y);
	c.y = b.y - (b.x - a.x);
 
	d.x = a.x - (a.y -  b.y);
	d.y = a.y - (b.x - a.x);
 
	e.x = d.x +  ( b.x - a.x - (a.y -  b.y) ) / 2;
	e.y = d.y -  ( b.x - a.x + a.y -  b.y ) / 2;
 
	if(times>0){
		//setcolor(rand()%15 + 1);
 
		line(cr, a.x,a.y,b.x,b.y);
		line(cr, c.x,c.y,b.x,b.y);
		line(cr, c.x,c.y,d.x,d.y);
		line(cr, a.x,a.y,d.x,d.y);

			
		pythagorasTree(cr, d,e,times-1);
		pythagorasTree(cr, e,c,times-1);
	}
}

 
int main(void){
 
	printf("Dibujanto un arbol de pitagoras...\n");

   	srand((unsigned)time(NULL));

   	cairo_surface_t *surface = cairo_image_surface_create( CAIRO_FORMAT_RGB24, WIDTH, HEIGHT);

	cairo_t *cr;
	cr = cairo_create(surface);
	cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
	cairo_paint (cr);

	cairo_set_line_width(cr, 1);
	cairo_set_source_rgba(cr, 0,0,0,1);

	point a,b;
	double side;
	int iter;

	//side = rand_fl();
	side = 100.0f;
	iter=rand() % 5 + 10;

	printf("Side: %f\n", side);
	printf("Iter: %d\n", iter);	
 
	a.x = 6*side/2 - side/2;
	a.y = 4*side;
	b.x = 6*side/2 + side/2;
	b.y = 4*side;

	fflush(stdout);
 
	pythagorasTree(cr, a,b, iter);

	char str[20];
	char file_name[128];  	

	strcpy(file_name, "tree_");
	//strcat(file_name, str);
	strcat(file_name, ".png");	

	cairo_status_t status= cairo_surface_write_to_png(surface, file_name);

	if (status!=CAIRO_STATUS_SUCCESS){
		printf("Error cairo_surface_write_to_png: %d\n", status);
		exit(1);
	}
 
	return 0;
}