/*
 ============================================================================
 Name        : capscreen.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <jpeglib.h>

int jpegWriteFileFromImage(char *filename, XImage *img){
	FILE *fp;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	fp = fopen(filename, "wb");
	if(!fp){
		printf("open file %s fail!\n", filename);
		return 0;
	}

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, fp);
	cinfo.image_width = img->width;
	cinfo.image_height = img->height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer[1];
	unsigned char *pBuf = (unsigned char *)malloc(3*img->width);
	row_pointer[0] = pBuf;

	int i = 0;
	while(cinfo.next_scanline < cinfo.image_height){
		int j = 0;
		for(i=0; i<img->width; i++){
			*(pBuf+j) = *(img->data + cinfo.next_scanline*img->bytes_per_line+i*4+2);
			*(pBuf+j+1) = *(img->data + cinfo.next_scanline*img->bytes_per_line+i*4+1);
			*(pBuf+j+2) = *(img->data + cinfo.next_scanline*img->bytes_per_line+i*4);
			j+=3;
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	free(pBuf);
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	return 1;
}

int capscreen(){
	Window desktop;
	Display *display;
	XImage *img;

	int screen_width;
	int screen_height;

	display = XOpenDisplay(NULL);
	if(!display){
		printf("cannot connect to local display\n");
		return 0;
	}

	desktop = RootWindow(display, 0);
	if(!desktop){
		printf("cannot get root window\n");
		return 0;
	}

	screen_width = DisplayWidth(display, 0);
	screen_height = DisplayHeight(display, 0);

	img = XGetImage(display, desktop, 0, 0, screen_width, screen_height, ~0, ZPixmap);

	jpegWriteFileFromImage("screen.jpeg", img);

	XFree(img);
	XCloseDisplay(display);
	return 1;
}

int main(void) {

	int keys_fd;
	char ret[2];
	struct input_event t;

	int mode_capscreen = 0;
	__u16 shortcut_keys[3];

	keys_fd = open("/dev/input/event2", O_RDONLY);
	if(keys_fd <= 0){
		printf("open /dev/input/event2 device error!\n");
		return EXIT_FAILURE;
	}

	while(1){
		if(read(keys_fd, &t, sizeof(t)) == sizeof(t)){
			if(t.type == EV_KEY){
				if(t.value == 0 || t.value == 1){
//					printf("key %d %s\n", t.code, t.value ? "Pressed" : "Release");
					if(t.code == KEY_ESC){
						break;
					}
					if(t.code == KEY_LEFTCTRL){
						mode_capscreen = 0;
						shortcut_keys[mode_capscreen] = t.code;
						mode_capscreen++;
					}else if(mode_capscreen && mode_capscreen < 3){
						shortcut_keys[mode_capscreen] = t.code;
						mode_capscreen++;
					}else if(mode_capscreen == 3){
						mode_capscreen = 0;
						if(shortcut_keys[0] == KEY_LEFTCTRL && shortcut_keys[1] == KEY_LEFTALT && shortcut_keys[2] == KEY_A){
							printf("capscreen !!!\n");
							if(capscreen()){
								printf("capscreen success !\n");
							}else{
								printf("capscreen fail !\n");
							}
						}
					}
				}
			}
		}
	}

	close(keys_fd);

	return EXIT_SUCCESS;
}
