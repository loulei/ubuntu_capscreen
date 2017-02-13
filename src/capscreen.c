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
#include <X11/Xutil.h>
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

int gc_depth(int depth, Display *display, Window scr, Window root, GC *gc){
	Window win;
	Visual *visual;
	XVisualInfo vis_info;
	XSetWindowAttributes win_attr;
	unsigned long win_mask;

	if(!XMatchVisualInfo(display, scr, depth, TrueColor, &vis_info)){
		printf("%d depth not supported\n", depth);
		return 0;
	}

	visual = vis_info.visual;
	win_attr.colormap = XCreateColormap(display, root, visual, AllocNone);
	win_attr.background_pixel = 0;
	win_attr.border_pixel = 0;

	win_mask = CWBackPixel | CWColormap | CWBorderPixel;
	win = XCreateWindow(display, root, 0, 0, 100, 100, 0, depth, InputOutput, visual, win_mask, &win_attr);

	*gc = XCreateGC(display, win, 0, 0);
	return 1;
}

int cover_screen(){
	int w = 100;
	int h = 100;
	int depth = 32;
	int bitmap_pad = 32;
	int bpl = 0;

	Display *display;
	Window root;
	Window scr;
	GC gc;
	int root_depth;

	Pixmap pm;
	XImage *img;
	unsigned char *buf_img;

	if(!(display = XOpenDisplay(NULL))){
		printf("open display fail\n");
		return 0;
	}

	root = XDefaultRootWindow(display);
	scr = XDefaultScreen(display);

	if((buf_img = malloc(w*h*4)) == NULL){
		printf("alloacte %d bytes fail\n", w*h*4);
		return 0;
	}

	int i;
	for(i=0; i<w*h*4; i++){
		*(buf_img+i) = 0x99;
	}

	root_depth = DefaultDepth(display, scr);
	printf("default depth : %d\n", root_depth);

	if(depth != root_depth){
		if(!gc_depth(depth, display, scr, root, &gc)){
			return 0;
		}
	}else{
		gc = DefaultGC(display, 0);
	}

	img = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, (char*)buf_img, w, h, bitmap_pad, bpl);
	pm = XCreatePixmap(display, root, w, h, depth);
	XPutImage(display, pm, gc, img, 0, 0, 0, 0, w, h);
	XSync(display, TRUE);

	XEvent ev;
	while(1){
		XNextEvent(display, &ev);
	}

	XFreePixmap(display, pm);
	XDestroyImage(img);
	XFreeGC(display, gc);

//	Window desktop;
//	Display *display;
//
//	int screen_width;
//	int screen_height;
//	int win_x;
//	int win_y;
//	int win_border_width = 0;
//
//	display = XOpenDisplay(NULL);
//	if(!display){
//		printf("cannot connect to local display\n");
//		return 0;
//	}
//
//	desktop = RootWindow(display, 0);
//	if(!desktop){
//		printf("cannot get root window\n");
//		return 0;
//	}
//
//	screen_width = DisplayWidth(display, 0);
//	screen_height = DisplayHeight(display, 0);
//
//	win_x = win_y = 0;
//
//	Window win = XCreateSimpleWindow(display, desktop, win_x, win_y, screen_width, screen_height, win_border_width, BlackPixel(display, 0), WhitePixel(display, 0));
//	XMapWindow(display, win);
//
//	GC gc;
//	XGCValues values;
//	unsigned long valuemask = 0;
//	gc = XCreateGC(display, win, valuemask, &values);
//	XSync(display, False);
//	if(gc < 0){
//		printf("xcreategc error\n");
//		return 0;
//	}
//
//	XSetBackground(display, gc, WhitePixel(display, 0));
//	XSetForeground(display, gc, BlackPixel(display, 0));
//
//	XFlush(display);
//	while(1){
//
//	}
//	XCloseDisplay(display);
	return 1;
}

int test_cover(){
	int screen_width;
	int screen_height;
	Display *display;
	int depth = 32;
	int bitmap_pad = 32;
	int bytes_per_line = 0;

	display = XOpenDisplay(NULL);
	if(!display){
		printf("cannot connect to local display\n");
		return 0;
	}
	screen_width = DisplayWidth(display, 0);
	screen_height = DisplayHeight(display, 0);
	printf("width:%d height:%d\n", screen_width, screen_height);
	unsigned char *image32 = (unsigned char *)malloc(screen_width * screen_height * 4);
	int i;
	for(i=0; i<screen_width*screen_height*4; i++){
		*(image32+i) = 0x44;
	}
	XImage *img = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, image32, screen_width, screen_height, bitmap_pad, bytes_per_line);
	Pixmap p = XCreatePixmap(display, XDefaultRootWindow(display), screen_width, screen_height, depth);
	XGCValues gcvalues;
	GC gc = XCreateGC(display, p, 0, &gcvalues);
	XPutImage(display, p, gc, img, 0, 0, 0, 0, screen_width, screen_height);

	XEvent ev;
	while(1){
		XNextEvent(display, &ev);
	}
	return 1;
}

int test_cover2(){
	Display *display = XOpenDisplay(NULL);
	XVisualInfo vinfo;
	XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &vinfo);
	XSetWindowAttributes attr;
	attr.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
	attr.border_pixel = 0;
	attr.background_pixel = 0;

	Window win = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 400, 400, 0, vinfo.depth, InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);
	GC gc = XCreateGC(display, win, 0, 0);

	XSelectInput(display, win, StructureNotifyMask);
	Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(display, win, &wm_delete_window, 1);

	XMapWindow(display, win);

	int keep_running = 1;
	XEvent event;
	while(keep_running){
		XNextEvent(display, &event);
		switch(event.type){
		case ClientMessage:
			if(event.xclient.message_type == XInternAtom(display, "WM_PROTOCOLS", 1) && (Atom)event.xclient.data.l[0] == XInternAtom(display, "WM_DELETE_WINDOW", 1)){
				keep_running = 0;
			}
			break;
		default:
			break;
		}
	}

	XDestroyWindow(display, win);
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
//							cover_screen();
//							test_cover();
							test_cover2();
//							if(capscreen()){
//								printf("capscreen success !\n");
//							}else{
//								printf("capscreen fail !\n");
//							}
						}
					}
				}
			}
		}
	}

	close(keys_fd);

	return EXIT_SUCCESS;
}
