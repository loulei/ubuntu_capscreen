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
					printf("key %d %s\n", t.code, t.value ? "Pressed" : "Release");
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
						}
					}
				}
			}
		}
	}

	close(keys_fd);

	return EXIT_SUCCESS;
}
