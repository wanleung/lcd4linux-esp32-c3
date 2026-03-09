/* $Id$
 * $URL$
 *
 * Driver for ESP32-C3 with I2C OLED display via serial communication
 *
 * Copyright (C) 2026 LCD4Linux Team
 *
 * This file is part of LCD4Linux.
 *
 * LCD4Linux is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * LCD4Linux is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * exported fuctions:
 *
 * struct DRIVER drv_ESP32OLED
 *
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "debug.h"
#include "cfg.h"
#include "qprintf.h"
#include "udelay.h"
#include "plugin.h"
#include "widget.h"
#include "widget_text.h"
#include "widget_icon.h"
#include "widget_bar.h"
#include "drv.h"
#include "drv_generic_text.h"
#include "drv_generic_graphic.h"

static char Name[] = "ESP32OLED";

static int drv_ESP32_fd = -1;
static char *drv_ESP32_port = NULL;
static int drv_ESP32_speed = 115200;


/****************************************/
/***  hardware dependant functions    ***/
/****************************************/

/* Send command to ESP32-C3 and wait for response */
static int drv_ESP32_send_command(const char *cmd)
{
    char buffer[256];
    int len, n;
    fd_set fds;
    struct timeval tv;
    
    if (drv_ESP32_fd < 0) {
        error("%s: Serial port not open", Name);
        return -1;
    }
    
    /* Send command */
    len = snprintf(buffer, sizeof(buffer), "%s\n", cmd);
    n = write(drv_ESP32_fd, buffer, len);
    if (n != len) {
        error("%s: write() failed: %s", Name, strerror(errno));
        return -1;
    }
    
    /* Wait for response with timeout */
    FD_ZERO(&fds);
    FD_SET(drv_ESP32_fd, &fds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    
    n = select(drv_ESP32_fd + 1, &fds, NULL, NULL, &tv);
    if (n <= 0) {
        error("%s: No response from device", Name);
        return -1;
    }
    
    /* Read response */
    n = read(drv_ESP32_fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        /* Check for OK response */
        if (strstr(buffer, "OK:") != NULL) {
            return 0;
        }
        if (strstr(buffer, "ERR:") != NULL) {
            error("%s: Device error: %s", Name, buffer);
            return -1;
        }
    }
    
    return 0;
}


static int drv_ESP32_open(const char *port, int speed)
{
    struct termios tios;
    int fd;
    
    info("%s: Opening serial port %s at %d baud", Name, port, speed);
    
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        error("%s: open(%s) failed: %s", Name, port, strerror(errno));
        return -1;
    }
    
    /* Configure serial port */
    if (tcgetattr(fd, &tios) < 0) {
        error("%s: tcgetattr() failed: %s", Name, strerror(errno));
        close(fd);
        return -1;
    }
    
    /* Set baud rate */
    speed_t baud = B115200;
    switch (speed) {
        case 9600:   baud = B9600;   break;
        case 19200:  baud = B19200;  break;
        case 38400:  baud = B38400;  break;
        case 57600:  baud = B57600;  break;
        case 115200: baud = B115200; break;
        default:
            error("%s: Unsupported baud rate %d", Name, speed);
            close(fd);
            return -1;
    }
    
    cfsetispeed(&tios, baud);
    cfsetospeed(&tios, baud);
    
    /* 8N1, no flow control */
    tios.c_cflag &= ~PARENB;
    tios.c_cflag &= ~CSTOPB;
    tios.c_cflag &= ~CSIZE;
    tios.c_cflag |= CS8;
    tios.c_cflag &= ~CRTSCTS;
    tios.c_cflag |= CREAD | CLOCAL;
    
    /* Raw mode */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);
    tios.c_oflag &= ~OPOST;
    
    /* Set timeouts */
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 10;
    
    if (tcsetattr(fd, TCSANOW, &tios) < 0) {
        error("%s: tcsetattr() failed: %s", Name, strerror(errno));
        close(fd);
        return -1;
    }
    
    /* Flush buffers */
    tcflush(fd, TCIOFLUSH);
    
    /* Wait for ESP32 to initialize */
    sleep(2);
    
    /* Clear any startup messages */
    char buffer[256];
    while (read(fd, buffer, sizeof(buffer)) > 0);
    
    return fd;
}


static int drv_ESP32_close(void)
{
    if (drv_ESP32_fd >= 0) {
        close(drv_ESP32_fd);
        drv_ESP32_fd = -1;
    }
    return 0;
}


static void drv_ESP32_clear(void)
{
    drv_ESP32_send_command("CLEAR");
}


static void drv_ESP32_write(const int row, const int col, const char *data, const int len)
{
    char cmd[256];
    char text[128];
    int x, y;
    
    /* Convert row/col to pixel coordinates */
    /* Assuming 6x8 font, adjust as needed */
    x = col * 6;
    y = row * 8;
    
    /* Escape colons in text */
    int i, j = 0;
    for (i = 0; i < len && i < 127; i++) {
        if (data[i] == ':') {
            if (j < 126) text[j++] = '\\';
        }
        if (j < 127) text[j++] = data[i];
    }
    text[j] = '\0';
    
    snprintf(cmd, sizeof(cmd), "TEXT:%d:%d:1:%s", x, y, text);
    drv_ESP32_send_command(cmd);
}


static void drv_ESP32_blit(const int row, const int col, const int height, const int width)
{
    /* Update display */
    drv_ESP32_send_command("DISPLAY");
}


static int drv_ESP32_start(const char *section)
{
    char *s;
    int rows = -1, cols = -1;
    
    /* Get port from config */
    s = cfg_get(section, "Port", NULL);
    if (s == NULL || *s == '\0') {
        error("%s: no '%s.Port' entry from %s", Name, section, cfg_source());
        return -1;
    }
    drv_ESP32_port = strdup(s);
    
    /* Get baud rate */
    if (cfg_number(section, "Speed", 115200, 9600, 115200, &drv_ESP32_speed) < 0)
        return -1;
    
    /* Get display size */
    if (sscanf(s = cfg_get(section, "Size", "128x32"), "%dx%d", &cols, &rows) != 2 || rows < 1 || cols < 1) {
        error("%s: bad %s.Size '%s' from %s", Name, section, s, cfg_source());
        free(s);
        return -1;
    }
    
    DROWS = rows;
    DCOLS = cols;
    
    /* Open serial port */
    drv_ESP32_fd = drv_ESP32_open(drv_ESP32_port, drv_ESP32_speed);
    if (drv_ESP32_fd < 0) {
        return -1;
    }
    
    /* Test connection */
    if (drv_ESP32_send_command("PING") < 0) {
        error("%s: Device not responding", Name);
        drv_ESP32_close();
        return -1;
    }
    
    info("%s: Display initialized (%dx%d)", Name, cols, rows);
    
    /* Clear display */
    drv_ESP32_clear();
    drv_ESP32_send_command("DISPLAY");
    
    return 0;
}


/****************************************/
/***        graphic functions         ***/
/****************************************/

static void drv_ESP32_GFX_blit(const int row, const int col, const int height, const int width)
{
    int r, c;
    
    for (r = row; r < row + height; r++) {
        for (c = col; c < col + width; c++) {
            char cmd[64];
            int pixel = drv_generic_graphic_gray(r, c);
            snprintf(cmd, sizeof(cmd), "PIXEL:%d:%d:%d", c, r, pixel > 0 ? 1 : 0);
            drv_ESP32_send_command(cmd);
        }
    }
    
    drv_ESP32_send_command("DISPLAY");
}


/****************************************/
/***            plugins               ***/
/****************************************/

/* none */


/****************************************/
/***        widget callbacks          ***/
/****************************************/

/* using drv_generic_text_draw() */
/* using drv_generic_text_icon_draw() */
/* using drv_generic_text_bar_draw() */
/* using drv_generic_graphic_draw() */
/* using drv_generic_graphic_icon_draw() */


/****************************************/
/***        exported functions        ***/
/****************************************/


/* list models */
int drv_ESP32_list(void)
{
    printf("ESP32-C3 with I2C OLED");
    return 0;
}


/* initialize driver & display */
int drv_ESP32_init(const char *section, const int quiet)
{
    int ret;
    
    info("%s: %s", Name, "$Rev$");
    
    /* real worker functions */
    drv_generic_graphic_real_blit = drv_ESP32_GFX_blit;
    
    /* start display */
    if ((ret = drv_ESP32_start(section)) != 0)
        return ret;
    
    /* initialize generic graphic driver */
    if ((ret = drv_generic_graphic_init(section, Name)) != 0)
        return ret;
    
    if (!quiet) {
        char buffer[40];
        qprintf(buffer, sizeof(buffer), "%s %dx%d", Name, DCOLS, DROWS);
        if (drv_generic_graphic_greet(buffer, NULL)) {
            sleep(3);
            drv_generic_graphic_clear();
        }
    }
    
    /* register text widget */
    wc = Widget_Text;
    wc.draw = drv_generic_graphic_draw;
    widget_register(&wc);
    
    /* register icon widget */
    wc = Widget_Icon;
    wc.draw = drv_generic_graphic_icon_draw;
    widget_register(&wc);
    
    /* register bar widget */
    wc = Widget_Bar;
    wc.draw = drv_generic_graphic_bar_draw;
    widget_register(&wc);
    
    return 0;
}


/* close driver & display */
int drv_ESP32_quit(const int quiet)
{
    info("%s: shutting down.", Name);
    
    drv_generic_graphic_quit();
    
    if (!quiet) {
        drv_ESP32_clear();
        drv_ESP32_send_command("DISPLAY");
    }
    
    drv_ESP32_close();
    
    if (drv_ESP32_port) {
        free(drv_ESP32_port);
        drv_ESP32_port = NULL;
    }
    
    return (0);
}


DRIVER drv_ESP32OLED = {
    .name = Name,
    .list = drv_ESP32_list,
    .init = drv_ESP32_init,
    .quit = drv_ESP32_quit,
};
