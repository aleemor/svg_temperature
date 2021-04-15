#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<math.h>
#include<string.h>
#include<time.h>

typedef struct svg
{
    char* svg;
    int height;
    int width;
    bool finalized;
} svg;

float temp[64];
float max_range = 100.0;
float min_range = 1.0;

void my_rect(void);
svg* svg_create(int width, int height);
void svg_rectangle(svg* psvg, int width, int height, int x, int y, char* fill, char* stroke, int strokewidth, int radiusx, int radiusy);
void svg_finalize(svg* psvg);
void svg_save(svg* psvg, char* filepath);
void svg_free(svg* psvg);
static void appendstringtosvg(svg* psvg, char* text);
static void appendnumbertosvg(svg* psvg, int n);

void create_array();



int main(void)
{
    puts("-----------------");
    puts("| codedrome.com |");
    puts("| SVG Library   |");
    puts("-----------------\n"); 


    create_array();
    my_rect(); 
    
    /*printf("temp[0] = %.1f\n",temp[0]);
    printf("temp[1] = %.1f\n",temp[1]);
    printf("temp[2] = %.1f\n",temp[2]);
    */

    return EXIT_SUCCESS;
}


void create_array(){
    int a[64];

	srand(time(0)); 
	for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){

	           a[j]=(rand()%10); 
               temp[j+8*i]=(float)a[j]+i*12.0;
               printf("%f - ",temp[j+8*i]);
        }
        printf("\n");
    }
}





void my_rect(void)
{   
    int x=0;
    int y=0;
    svg* psvg;
    psvg = svg_create(512, 512);

    if(psvg == NULL)
    {
        puts("psvg is NULL");
    }
    else
    {
        for(y=0;y<8;y++){
            for(x=0;x<8;x++){
                uint8_t reddd=((temp[x+8*y]-min_range)/(max_range-min_range))*255;
                uint8_t red, green, blue;
                if(reddd>0 && reddd<62 ){
                    blue=255;
                    red=0;
                    green=reddd;
                }
                else if (reddd>=62 && reddd<124){
                    blue=255-reddd;
                    red=0;
                    green=255;
                }
                else if (reddd>=124 && reddd<186){
                    blue=0;
                    red=reddd;
                    green=255;

                }
                else if (reddd>=186 && reddd<248){
                    blue=0;
                    red=255;
                    green=255-reddd;                    
                }

                char rgb_string[20];
                sprintf(rgb_string,"rgb(%d,%d,%d)",red,green,blue);
                svg_rectangle(psvg, 64, 64, (64*x), (64*y) , rgb_string, "black", 0, 0, 0);
            }
        }
        svg_finalize(psvg);
        svg_save(psvg, "my_rect.svg");
        svg_free(psvg);
    }
}

svg* svg_create(int width, int height)
{
    svg* psvg = malloc(sizeof(svg));

    if(psvg != NULL)
    {
        *psvg = (svg){.svg = NULL, .width = width, .height = height, .finalized = false};

        psvg->svg = malloc(1);

        sprintf(psvg->svg, "%s", "\0");

        appendstringtosvg(psvg, "<svg width='");
        appendnumbertosvg(psvg, width);
        appendstringtosvg(psvg, "px' height='");
        appendnumbertosvg(psvg, height);
        appendstringtosvg(psvg, "px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>\n");

        return psvg;
    }
    else
    {
        return NULL;
    }
}

void svg_rectangle(svg* psvg, int width, int height, int x, int y, char* fill, char* stroke, int strokewidth, int radiusx, int radiusy)
{
    appendstringtosvg(psvg, "    <rect fill='");
    appendstringtosvg(psvg, fill);
    appendstringtosvg(psvg, "' stroke='");
    appendstringtosvg(psvg, stroke);
    appendstringtosvg(psvg, "' stroke-width='");
    appendnumbertosvg(psvg, strokewidth);
    appendstringtosvg(psvg, "px' width='");
    appendnumbertosvg(psvg, width);
    appendstringtosvg(psvg, "' height='");
    appendnumbertosvg(psvg, height);
    appendstringtosvg(psvg, "' y='");
    appendnumbertosvg(psvg, y);
    appendstringtosvg(psvg, "' x='");
    appendnumbertosvg(psvg, x);
    appendstringtosvg(psvg, "' ry='");
    appendnumbertosvg(psvg, radiusy);
    appendstringtosvg(psvg, "' rx='");
    appendnumbertosvg(psvg, radiusx);
    appendstringtosvg(psvg, "' />\n");
}

void svg_finalize(svg* psvg)
{
    appendstringtosvg(psvg, "</svg>");

    psvg->finalized = true;
}

void svg_save(svg* psvg, char* filepath)
{
    if(!psvg->finalized)
    {
        svg_finalize(psvg);
    }

    FILE* fp;

	fp = fopen(filepath, "w");

	if(fp != NULL)
	{
		fwrite(psvg->svg, 1, strlen(psvg->svg), fp);

		fclose(fp);
	}
}

void svg_free(svg* psvg)
{
    free(psvg->svg);

    free(psvg);
}

static void appendstringtosvg(svg* psvg, char* text)
{
    int l = strlen(psvg->svg) + strlen(text) + 1;

    char* p = realloc(psvg->svg, l);

    if(p)
    {
        psvg->svg = p;
    }

    strcat(psvg->svg, text);
}

static void appendnumbertosvg(svg* psvg, int n)
{
    char sn[16];

    sprintf(sn, "%d", n);

    appendstringtosvg(psvg, sn);
}