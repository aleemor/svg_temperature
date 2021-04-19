#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <stdbool.h>
#include <netdb.h>
#include<math.h>

// prametri seriale
#define serial_baudrate B115200


int serial_file_descriptor,bytes_read;


unsigned char read_buffer[100]; 



char serial_port[] = "/dev/cu.usbmodem14101";
//char serial_port[] = "/dev/cu.usbserial-FT2K0SI2";
//char serial_port[] = "/dev/cu.usbserial-FTHFGYDY";
//char serial_port[] = "/dev/cu.usbserial-1410";
//char serial_port[] = "/dev/ttyACM0";

struct IR_sct {
  uint8_t id;
  uint8_t row_pkt_num;
  uint8_t lenght;
  uint8_t temp2;
  uint16_t half_pixels[32];
  uint16_t pixels_5[8];
  uint8_t pixel[12];
  uint8_t tlm_enfd;
  uint8_t tlm_ednd;
  uint8_t tlm_end;
  uint8_t tlm_endd;
} IR_sens;

typedef struct svg
{
    char* svg;
    int height;
    int width;
    bool finalized;
} svg;

float temp[64];
uint16_t temp_received[64];
float max_range = 100.0;
float min_range = 1.0;

char time_string[64] = " ";

uint8_t time_hour, dayy, time_minutes;

void time_human();

void my_rect(void);
svg* svg_create(int width, int height);
void svg_rectangle(svg* psvg, int width, int height, int x, int y, char* fill, char* stroke, int strokewidth, int radiusx, int radiusy);
void svg_finalize(svg* psvg);
void svg_save(svg* psvg, char* filepath);
void svg_free(svg* psvg);
static void appendstringtosvg(svg* psvg, char* text);
static void appendnumbertosvg(svg* psvg, int n);

void serial_initialize();
void serial_listen();
void create_array();



int main(void){

    //create_array();
    serial_initialize();
    serial_listen();
    
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
        
        time_human();
        char filename_svg[34] = {};
        char buf[] = {"IR_pictures/%s.svg"};
        sprintf(filename_svg,buf,time_string);

        svg_save(psvg, filename_svg);
        svg_free(psvg);
    }
}

void time_human() {
  struct timeval  now;
  struct tm*      local;
  gettimeofday(&now, NULL);
  local = localtime(&now.tv_sec);
    char stri[] = {"%02d%02d%02d-%02d%02d%02d"};
    /////global variables:
  sprintf(time_string,stri,local->tm_year+1900, local->tm_mon+1, local->tm_mday,local->tm_hour, local->tm_min, local->tm_sec);
  time_hour = local->tm_hour;
  dayy = local->tm_mday;
  time_minutes = local->tm_min;
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

void serial_initialize() {
	serial_file_descriptor = open(serial_port,O_RDWR );
    if(serial_file_descriptor == -1) {
        printf("Serial port error\n");
    }
    else {
        printf("Serial port opened\n");
    }
    struct termios SerialPortSettings;
    tcgetattr(serial_file_descriptor, &SerialPortSettings);
    cfsetispeed(&SerialPortSettings,serial_baudrate);
    cfsetospeed(&SerialPortSettings,serial_baudrate);
    SerialPortSettings.c_cflag &= ~PARENB;
    SerialPortSettings.c_cflag &= ~CSTOPB;
    SerialPortSettings.c_cflag &= ~CSIZE;
    SerialPortSettings.c_cflag |=  CS8;
    SerialPortSettings.c_cflag &= ~CRTSCTS;
    SerialPortSettings.c_cflag |= CREAD | CLOCAL;
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
    SerialPortSettings.c_iflag &= ~(  ECHO | ECHOE | ISIG);
    SerialPortSettings.c_oflag &= ~OPOST;
    SerialPortSettings.c_cc[VMIN] = 100;
    SerialPortSettings.c_cc[VTIME] = 1;
    if((tcsetattr(serial_file_descriptor,TCSANOW,&SerialPortSettings)) != 0) {
        printf("ERROR ! in Setting attributes\n");
    }
}

void serial_listen(){
    while (true){
        bytes_read = read(serial_file_descriptor,&read_buffer,100);
        if(bytes_read==100 && read_buffer[0]==0x20 ) {
            memcpy(&IR_sens,read_buffer,100);
            if(IR_sens.row_pkt_num==0){
                memcpy(&temp_received[0],&IR_sens.half_pixels[0],32*sizeof(uint16_t));
            }
            else if(IR_sens.row_pkt_num==1){
                memcpy(&temp_received[32],&IR_sens.half_pixels[0],32*sizeof(uint16_t));
                for(int i=0;i<64;i++){
                    temp[i]=temp_received[i]/100.0;
                    printf("%f\n",temp[i]);
                    my_rect(); 
                }
            }
        }
    }
}
