#include <cairo.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define WIDTH 100
#define HEIGHT 50

int main(){

	char buffer [10];
	char file_header[364];
	char first_bm_header[512];
	char bm_header[420];

	//char img_data[16490];

	char img_header[8];

	unsigned char img_data[16384];

	bool notFound=true;
	bool notFinish=true;


	printf ("Se inicia la extracción\n");

	FILE * sourceFILE=fopen("archivo.slide","rb");

	if(sourceFILE==NULL){
		perror("No pudo abrir archivo para lectura");
	}

	//FILE * storeFile=fopen("salida.bmp", "wb+");

	//if(storeFile==NULL){perror("No pudo abrir archivo para escritura");}

	size_t result=fread(&file_header, 1, sizeof(file_header), sourceFILE);
	if (result!=sizeof(file_header)){
		perror("No pudo leer el encabezado del archivo\n");		
	}

	result=fread(&first_bm_header, 1, sizeof(first_bm_header), sourceFILE);
	if (result!=sizeof(first_bm_header)){
		perror("No pudo leer el primer encabezado BM1\n");		
	}	

	char file_name[30];
	char str_tmp[10];

	int i=0;
	while(notFinish){
		notFound=true;

		result=fread(&bm_header, 1, sizeof(bm_header), sourceFILE);
		if (result!=sizeof(bm_header)){
			perror("No pudo leer encabezado BM1 de la imagen\n");		
		}

		result=fread(&img_header, 1, sizeof(img_header), sourceFILE);
		if (result!=sizeof(img_header)){
			perror("No pudo leer encabezado BM1 de la imagen\n");		
		}

	    sprintf(str_tmp, "%d", i);
	    strcpy(file_name, "fragmento_");
	    strcat(file_name, str_tmp);
	    strcat(file_name, ".bmp");

	    FILE * storeFile=fopen(file_name, "wb+");


		result=fread(&img_data, 1, sizeof(img_data), sourceFILE);
		if (result!=sizeof(img_data)){
			perror("No pudo leer encabezado BM1 de la imagen\n");		
		}

		fwrite(&img_data, 1, sizeof(img_data), storeFile);

		char cairo_file_name[128];  

		strcpy(cairo_file_name, "cairo_");
		strcat(cairo_file_name, str_tmp);
		strcat(cairo_file_name, ".png");

		int stride;
		stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, WIDTH);
		//unsigned char *my_data = cairo_image_surface_get_data (surface);

		cairo_surface_t *surface = cairo_image_surface_create_for_data(&img_data, CAIRO_FORMAT_RGB24, WIDTH, HEIGHT, stride);		
		if(surface==NULL){
			perror("No pudo crear cairo surface\n");	
		}

		cairo_status_t status= cairo_surface_write_to_bmp(surface, cairo_file_name);
		if (status!=CAIRO_STATUS_SUCCESS){
			printf("%s\n", cairo_status_to_string (status));
			perror("No pudo guardar archivo\n");	
		}



		int j=0;
		while(notFound && notFinish){
			
			size_t result=fread(&buffer, 1, 1, sourceFILE);
			if (result==1){
				j++;

				if(buffer[0]=='B'){
					size_t result=fread(&buffer, 1, 1, sourceFILE);
					if(result==1){
						j++;
						if(buffer[0]=='M'){
							size_t result=fread(&buffer, 1, 1, sourceFILE);
							if(result==1){
								j++;
								if(buffer[0]=='1'){
									printf ("Se encontro BM1\n");
									notFound=false;
									i++;
								}else{
									//char buf[3];
									//buf[0]='B';
									//buf[1]='M';
									//fwrite(&buf, 2, 1, storeFile);
									//fwrite(&buffer, 1, 1, storeFile);
								}
							}else{
								notFinish=false;
							}
						}else{
							//char buf[2];
							//buf[0]='B';
							//fwrite(&buf, 1, 1, storeFile);
							//fwrite(&buffer, 1, 1, storeFile);
						}					
					}else{
						notFinish=false;
					}
				}else{
					//fwrite(&buffer, 1, 1, storeFile);
				}
			}else{
				notFinish=false;
			}
		}
		//printf("j: %d bytes? \n",j);

		fclose(storeFile);
	}

	printf("i: %d imagenes?\n",i);

	printf ("FIN\n");

	fclose(sourceFILE);
	//fclose(storeFile);
	return 0;
}