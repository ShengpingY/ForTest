// Keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <sys/select.h>

class Keyboard {
public:
    // ³õÊ¼»¯ÖÕ¶ËÒÔÖ§³Ö·Ç×èÈû¶ÁÈ¡
    static void initTermios() {
        tcgetattr(STDIN_FILENO, &orig_termios); // ±£´æµ±Ç°ÉèÖÃ
        current_termios = orig_termios;

        // ÐÞ¸ÄÉèÖÃÎª·Ç×èÈû
        current_termios.c_lflag &= ~(ICANON | ECHO); // ÉèÖÃÎª·Ç¹æ·¶Ä£Ê½£¬¹Ø±Õ»ØÏÔ
        tcsetattr(STDIN_FILENO, TCSANOW, &current_termios);

        // ÉèÖÃstdinÎª·Ç×èÈû
        tcflush(STDIN_FILENO, TCIFLUSH);
    }

    // »Ö¸´ÖÕ¶ËÉèÖÃ
    static void resetTermios() {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    }

    // ¼ì²éÊÇ·ñÓÐ°´¼üÊäÈë
    static bool kbhit() {
        struct timeval tv = {0L, 0L}; // ²»µÈ´ý
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(1, &fds, NULL, NULL, &tv) > 0;
    }

    // »ñÈ¡µ¥¸ö×Ö·û£¬·Ç×èÈû
    static char getch() {
        if(kbhit()) {
            char ch;
            read(STDIN_FILENO, &ch, 1);
            return ch;
        }
        return EOF; // Ã»ÓÐÊäÈëµÄÇé¿öÏÂ·µ»ØEOF×Ö·û
    }

private:
    static struct termios orig_termios;
    static struct termios current_termios;
};

// ¾²Ì¬³ÉÔ±±äÁ¿¶¨Òå
struct termios Keyboard::orig_termios;
struct termios Keyboard::current_termios;

#endif // KEYBOARD_H