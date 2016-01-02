/*
 * NAME: 
 * USERID: 
 *
 * Fall 2015 CS349 Assignment 1:  An implementation of Breakout in C/C++ and Xlib.
 * 
 * 
 *
 * Commands to compile and run:
 *
 *  g++ -o a1 a1.cpp -L/usr/X11R6/lib -lX11 -lstdc++
 *  ./a1
 *
 * Note: the -L option and -lstdc++ may not be needed on some machines.
 */



#include <cstdlib>
#include <iostream>
#include <list>
#include <unistd.h>
#include <map>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <sys/time.h>
#include <math.h>



/*
 * Header files for X functions
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifndef sina
#define sina false
#endif

using namespace std;

int widthofwinsow = 655;
int heightofwindow = 650;
int widthofblock = 60;
int heightofblock = 40;
int r_ball = 10;
bool start = false;
int sizeofpaddle = 7;
int FPS = 30;
int blockgap = 4;
int widthofpaddle = 150;
int heightofpaddle = 10;
int BufferSize = 10;
int speed = 8;
int directioninx;
int directioniny;
bool gameover = false;
int paddlexcor;
int paddleycor;
int score = 0;
int speedofpaddle = 6;
bool GoldBlocksChosen = false;
bool largepaddle = false;
bool splashscreen = true;
bool Pause = false;


/*
 * Information to draw on the window.
 */
struct XInfo {
    Display*    display;
    int      screen;
    Window   window;
    GC gc[8];   // 6 different graphics contexts
    Pixmap  pixmap;     // double buffer
    int     width;      // size of pixmap
    int     height;
};


/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
    cerr << str << endl;
    exit(0);
}


/*
* Function to get the time useful to resolve the eventloop problem
*/
unsigned long GetTheCurrentTime() {
    timeval Time;
    gettimeofday(&Time, NULL);
    int result = Time.tv_sec * 1000000 + Time.tv_usec;
    return result;
}

unsigned long GetSeconds() {
    timeval Time;
    gettimeofday(&Time, NULL);
    int result = Time.tv_sec;
    return result;
}

unsigned int paddlestart;
unsigned int paddleend;


map < pair<int,int>, bool> destroyed;
map < pair<int,int>, bool> GoldBlock;
struct block {
    int xcor;
    int ycor;
} theblock;
vector<block> BlockCoordinates;

///// Choosing random Gold blocks:
int RandomGold(int a = 4) {
    //srand(time(NULL));
    int n;
    n = (rand() % a)+1;
    return n;
}

////////////// Code for displayables Start here ///////////////////////////////////////////////////////

// An abstarct class for representing desplayable objects
class Displayable {
    public:
        virtual void paint(XInfo &xinfo) = 0;
}; 


// Blocks:
class blocks : public Displayable 
{
public:
    virtual void paint( XInfo &xinfo ) {
        bool stillblocks = false;
        x = 40;
        y = 40;
        BlockCoordinates.clear();
        while (x + widthofblock < widthofwinsow - 40) {
            if( !destroyed[ make_pair(x,y) ] ) {
                stillblocks = true;
                int ran = RandomGold(10);
                if( ran == 3 && GoldBlocksChosen == false) {
                    GoldBlock[make_pair(x,y)] = true;
                }
                if(GoldBlock[make_pair(x,y)]) {
                    XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[5], x, y, widthofblock, heightofblock );
                }
                else {
                    XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, widthofblock, heightofblock );
                    XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[2], x+7, y+7, widthofblock-14, heightofblock-14 );
                    XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[6], x+15, y+15, widthofblock-30, heightofblock-30 );
                }
            }
            theblock.xcor = x;
            theblock.ycor = y;
            BlockCoordinates.push_back(theblock);
            y = y + heightofblock + blockgap;
            while (y < heightofwindow/2.5 ) {
                if( !destroyed[make_pair(x,y)] ) {
                    stillblocks = true;
                    int r = RandomGold(8);
                    if(r == 3 && GoldBlocksChosen == false) {
                        GoldBlock[make_pair(x,y)] = true;
                    }
                    if(GoldBlock[make_pair(x,y)]) {
                        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[5], x, y, widthofblock, heightofblock );
                    }
                    else {
                        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, widthofblock, heightofblock );
                        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[2], x+7, y+7, widthofblock-14, heightofblock-14 );
                        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[6], x+15, y+15, widthofblock-30, heightofblock-30 );
                    }
                }
                theblock.xcor = x;
                theblock.ycor = y;
                BlockCoordinates.push_back(theblock);
                y = y + heightofblock + blockgap;
            }
            x = x + widthofblock + blockgap;
            y = 40;
        }
        GoldBlocksChosen = true;
        if(!stillblocks) {
            start = false;
                        for(int i=0;i<BlockCoordinates.size();i++) {
                destroyed[ make_pair(BlockCoordinates[i].xcor,BlockCoordinates[i].ycor)] = false;
            }
            directioninx = speed;
            directioniny = speed;
            score = 0;
            GoldBlocksChosen = false;
            GoldBlock.clear();
            if(largepaddle) {
                widthofpaddle = widthofpaddle/2;
                largepaddle = false;
            }
        }
    }

    // Constructor:
    blocks (int x, int y) {}

private:
    int x;
    int y;

};

// Ball:
class ball : public Displayable 
{
public:
    virtual void paint( XInfo &xinfo ) {
        if (start == false) {
            x = paddlexcor+(widthofpaddle/2);
            y = paddleycor-((r_ball));
        }
        XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[3], x, y, r_ball, r_ball, 0, 360*64);
    }

    void move(XInfo &xinfo) {
        // Ball touching one of the blocks:
        for(int i=0;i<BlockCoordinates.size();i++) {
            if( y <= BlockCoordinates[i].ycor + heightofblock &&
                y+r_ball >= BlockCoordinates[i].ycor &&
                x <= BlockCoordinates[i].xcor + widthofblock &&
                x+r_ball >= BlockCoordinates[i].xcor
            ) {
                if( !destroyed[ make_pair(BlockCoordinates[i].xcor,BlockCoordinates[i].ycor) ]  ){
                    destroyed[ make_pair(BlockCoordinates[i].xcor,BlockCoordinates[i].ycor) ] = true;
                    int d1 = min(abs((y) - (BlockCoordinates[i].ycor + heightofblock)) ,  abs((y+r_ball) - BlockCoordinates[i].ycor) );
                    int d2 = min(  abs((x) - (BlockCoordinates[i].xcor + widthofblock) ),  abs((x+r_ball) -  BlockCoordinates[i].xcor));
                    if( d1 == d2 ) {
                        directioniny = -1*directioniny;
                        directioninx = -1*directioninx;
                    }
                    else if( d1 < d2 ) {
                        directioniny = -1*directioniny;
                    }
                    else if( d2 < d1 ) {
                        directioninx = -1*directioninx;
                    }

                    score = score + 100;
                    if( GoldBlock[make_pair(BlockCoordinates[i].xcor,BlockCoordinates[i].ycor)] && largepaddle == false) {
                        widthofpaddle = widthofpaddle*2;
                        largepaddle = true;
                        paddlestart = GetSeconds();
                    }
                    BlockCoordinates.erase(BlockCoordinates.begin() + i);
                    break;
                }
            }
        }
        // Ball touching the wall
        if (
             x <=0 || x >= widthofwinsow-r_ball
        ) {
            directioninx = -1*directioninx;
        }
        if ( y<= 0) {
            directioniny = -1*directioniny;
        }
        if ( y >= heightofwindow ) {
            // loosing
            start = false; 
            for(int i=0;i<BlockCoordinates.size();i++) {
                destroyed[ make_pair(BlockCoordinates[i].xcor,BlockCoordinates[i].ycor)] = false;
            }
            directioninx = speed;
            directioniny = speed;
            score = 0;
            GoldBlocksChosen = false;
            GoldBlock.clear();
            if(largepaddle) {
                widthofpaddle = widthofpaddle/2;
                largepaddle = false;
            }

        }
        // Ball touching the paddle
        if(
            x - (r_ball/2) <= paddlexcor + widthofpaddle &&
            x + (r_ball/2)>= paddlexcor &&
            y <= paddleycor + heightofpaddle &&
            y + (r_ball/2) >= paddleycor 
        ) {
            directioniny = -1*directioniny;
        }



        x = x + directioninx ;
        y = y - directioniny;
    }

    ball(int x, int y) {}

private:
    int x;
    int y;

};

// Paddle:
class paddle : public Displayable 
{
public:
    virtual void paint( XInfo &xinfo ) {
        if ( !start ) {
            x = widthofwinsow/2-(widthofpaddle/2);
            y = heightofwindow/(1.1);
            paddlexcor = x;
            paddleycor = y;
        }
        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[6], x, y, 30, heightofpaddle );
        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[2], x+30, y, widthofpaddle-60, heightofpaddle );
        XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[6], x+30+(widthofpaddle-60), y,30 , heightofpaddle );
    }

    void moveto(int xcoordinate) {
        int halfofpaddle = widthofpaddle/2;
        if( xcoordinate <= halfofpaddle ) {
            x = 0;
            paddlexcor = x;
        }
        else if( xcoordinate >= widthofwinsow - halfofpaddle ) {
            x = widthofwinsow - widthofpaddle;
            paddlexcor = x;
        }
        else {
            x = xcoordinate-halfofpaddle;
            paddlexcor = x;
        }
    }

    void left() {
        if( (x > 0 && x-speedofpaddle <=0) || x<=0 ){
            x = 0;
            paddlexcor = x;
        }
        else {
            x = x -speedofpaddle;
            paddlexcor = x;
        }  
    }

    void right() {
        if( (x < widthofwinsow - widthofpaddle && x+speedofpaddle >= widthofwinsow - widthofpaddle) || x >= widthofwinsow - widthofpaddle) {
            x = widthofwinsow - widthofpaddle;
            paddlexcor = x;
        }
        else {
            x = x+speedofpaddle;
            paddlexcor = x;
        }
    }



    paddle(int x, int y) {}

    int getx() {
        return x;
    }
    int gety() {
        return y;
    }

private:
    int x;
    int y;

};

class text : public Displayable {
public:
    virtual void paint( XInfo &xinfo ) {
        XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 12, heightofwindow-20, "Press 's' to start", 18 );
        XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 12, heightofwindow-7, "Press 'q' to quit", 17 );
        

        // Need to convert score (int) to string
        string scoreingame; 
        stringstream conversion; 
        conversion << score;
        scoreingame = conversion.str();
        XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], widthofwinsow - 120 , heightofwindow-20, "Total Score:", 12 );
        XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], widthofwinsow -40, heightofwindow-20, scoreingame.c_str(), scoreingame.length() );
        XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], widthofwinsow - 120, heightofwindow-7, "Press 'h' for help", 18 );
            if( splashscreen ) {
            XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[0], 0, 0, widthofwinsow, heightofwindow );
            XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[3], 180, 40, 300, 120 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 60, "Name: Sina Motevalli Bashi", 26 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 80, "Student #: 20455091", 19 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 100, "This is the help section", 24 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 120, "Press 'h'  to get into the game", 31 );
            XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[4], 180, 200, 300, 150 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 200 , 220, "Added feature:", 14 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 200 , 240, "A number of randomly chosen bricks", 34 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 200 , 260, "are coloured pink, if the ball hits them", 40 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 200 , 280, "the size of the paddle gets doubled for", 39 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[0], 200 , 300, "a few seconds", 12 );
            XFillRectangle( xinfo.display, xinfo.pixmap, xinfo.gc[6], 180, 400, 300, 150 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 420, "In the Game", 11 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 440, "Press 's' to start", 18 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 460, "Press 'q' to quit", 17 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 480, "Press 'p' to pause the game", 27 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 500, "Press 'd' to move right", 23 );
            XDrawString( xinfo.display, xinfo.pixmap, xinfo.gc[4], 200 , 520, "Press 'a' to move left", 22 );
        }
    }   

    text (int x) {}
};


list<Displayable *> dList;
blocks BLOCKS(0,0);
ball BALL(0,0);
paddle PADDLE(0,0);
text TEXT(0);

////////////// Code for displayables Ends here ///////////////////////////////////////////////////////


/*
 * Create a window
 */
void initX(int argc, char* argv[], XInfo& xInfo) {

    XSizeHints hints;

    /*
    * Display opening uses the DISPLAY  environment variable.
    * It can go wrong if DISPLAY isn't set, or you don't have permission.
    */
    xInfo.display = XOpenDisplay( "" );
    if ( !xInfo.display )   {
        error( "Can't open display." );
    }

    /*
    * Find out some things about the display you're using.
    */
    xInfo.screen = DefaultScreen( xInfo.display ); // macro to get default screen index

    unsigned long white, black;
    white = XWhitePixel( xInfo.display, xInfo.screen ); 
    black = XBlackPixel( xInfo.display, xInfo.screen );

    hints.x = 100;
    hints.y = 100;
    hints.width = widthofwinsow;
    hints.height = heightofwindow;
    hints.flags = PPosition | PSize;

    xInfo.window = XCreateSimpleWindow(
       xInfo.display,               // display where window appears
       DefaultRootWindow( xInfo.display ), // window's parent in window tree
       hints.x, hints.y,                       // upper left corner location
       hints.width, hints.height,                  // size of the window
       5,                            // width of window's border
       black,                       // window border colour
       white );                     // window background colour

    // extra window properties like a window title
    XSetStandardProperties(
        xInfo.display,      // display containing the window
        xInfo.window,       // window whose properties are set
        "Breakout Game",    // window's title
        "BO",               // icon's title
        None,               // pixmap for the icon
        argv, argc,         // applications command line args
        None );         // size hints for the window


    /*
    * Allocating colors
    */
    Colormap colors;
    XColor green;
    colors = DefaultColormap(xInfo.display, xInfo.screen);
    XAllocNamedColor(xInfo.display, colors, "green", &green, &green);


    /*
    * Creating Graphics contexts
    */
    int i = 0;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground(xInfo.display, xInfo.gc[i], white);
    XSetBackground(xInfo.display, xInfo.gc[i], black);
    XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
    XSetLineAttributes(xInfo.display, xInfo.gc[i],
                         1, LineSolid, CapButt, JoinRound);

    i=1;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground( xInfo.display, xInfo.gc[i], green.pixel );
    XSetBackground( xInfo.display, xInfo.gc[i], black );
    XSetFillStyle( xInfo.display, xInfo.gc[i], FillStippled);



    XColor blue;
    XAllocNamedColor(xInfo.display, colors, "blue", &blue, &blue);


    i=2;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground( xInfo.display, xInfo.gc[i], blue.pixel );

    XColor red;
    XAllocNamedColor(xInfo.display, colors, "red", &red, &red);

    i=3;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground( xInfo.display, xInfo.gc[i], red.pixel );

    i=4;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground( xInfo.display, xInfo.gc[i], black );

    XColor pink;
    XAllocNamedColor(xInfo.display, colors, "pink", &pink, &pink);

    i=5;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground( xInfo.display, xInfo.gc[i], pink.pixel );

    XColor yellow;
    XAllocNamedColor(xInfo.display, colors, "yellow", &yellow, &yellow);

    i=6;
    xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
    XSetForeground( xInfo.display, xInfo.gc[i], yellow.pixel );



    int depth = DefaultDepth(xInfo.display, DefaultScreen(xInfo.display));
    xInfo.pixmap = XCreatePixmap(xInfo.display, xInfo.window, hints.width, hints.height, depth);
    xInfo.width = hints.width;
    xInfo.height = hints.height;


    /*
    * Input selecting
    */
    XSelectInput(xInfo.display, xInfo.window, 
        ButtonPressMask | KeyPressMask | 
        PointerMotionMask | 
        EnterWindowMask | LeaveWindowMask |
        KeyReleaseMask | StructureNotifyMask);


    /*
     * Don't paint the background -- reduce flickering
     */
    XSetWindowBackgroundPixmap(xInfo.display, xInfo.window, None);

    /*
     * Put the window on the screen.
     */
    XMapRaised( xInfo.display, xInfo.window );

    XFlush(xInfo.display);
}


/*
 * Function to repaint a display list
 */
void repaint( XInfo &xinfo) {
    list<Displayable *>::const_iterator begin = dList.begin();
    list<Displayable *>::const_iterator end = dList.end();

    XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[4], 0, 0, xinfo.width, xinfo.height);

    while( begin != end ) {
        Displayable *d = *begin;
        d->paint(xinfo);
        begin++;
    }


    // copy buffer to window
    XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0], 
        0, 0, widthofwinsow, heightofwindow,  // region of pixmap to copy
        0, 0); // position to put top left corner of pixmap in window

    XFlush( xinfo.display );
}

/*
* Eventloop helper functions:
*/
void HandleKeyRelease(XInfo &xinfo, XEvent &event) {
    KeySym key;
    char text[BufferSize];
    int i = XLookupString( 
        (XKeyEvent *)&event,    // the keyboard event
        text,                   // buffer when text will be written
        BufferSize,             // size of the text buffer
        &key,                   // workstation-independent key symbol
        NULL );                 // pointer to a composeStatus structure (unused)
    if ( i == 1) {
        //printf("Got key press -- %c\n", text[0]);
        if (text[0] == 'q') {
            error("Terminating normally.");
        }
        else if(text[0] == 's') {
            if(!splashscreen) {
                start = true;
            }
        }
        else if(text[0] == 'h') {
            if(splashscreen) {
                splashscreen = false;
            }
            else {
                splashscreen = true;
            }
        }
        else if(text[0] == 'p') {
            if(Pause) {
                Pause = false;
            }
            else {
                Pause = true;
            }
        }
    }    
}

void HandleAnimation(XInfo &xinfo) {
    if(!Pause){
        BALL.move(xinfo);
    }
}

void HandleMotion(XEvent &event) {
    if (start && !Pause) {
        PADDLE.moveto(event.xbutton.x);
    }
}

// void handleResize(XInfo &xinfo, XEvent &event) {
//     XConfigureEvent xce = event.xconfigure;
//     //fprintf(stderr, "Handling resize  w=%d  h=%d\n", xce.width, xce.height);
//     if (xce.width != widthofwinsow || xce.height != heightofwindow) {
//         widthofwinsow = xce.width;
//         heightofwindow = xce.height;
//     }
// }


void handleResize(XInfo &xinfo, XEvent &event) {
    XConfigureEvent xce = event.xconfigure;
    //fprintf(stderr, "Handling resize  w=%d  h=%d\n", xce.width, xce.height);
    if (xce.width != xinfo.width || xce.height != xinfo.height) {
        XFreePixmap(xinfo.display, xinfo.pixmap);
        int depth = DefaultDepth(xinfo.display, DefaultScreen(xinfo.display));
        xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, xce.width, xce.height, depth);
        xinfo.width = xce.width;
        xinfo.height = xce.height;
        widthofwinsow = xce.width;
        heightofwindow = xce.height;
    }
    start = false;
    for(int i=0;i<BlockCoordinates.size();i++) {
                destroyed[ make_pair(BlockCoordinates[i].xcor,BlockCoordinates[i].ycor)] = false;
            }
    //GoldBlock.clear();        
    if(largepaddle) {
        widthofpaddle = widthofpaddle/2;
        largepaddle = false;
    }
}

void HandleKeyPress(XInfo &xinfo, XEvent &event) {
    KeySym key;
    char text[BufferSize];
    int i = XLookupString( 
        (XKeyEvent *)&event,    // the keyboard event
        text,                   // buffer when text will be written
        BufferSize,             // size of the text buffer
        &key,                   // workstation-independent key symbol
        NULL );                 // pointer to a composeStatus structure (unused)
    if ( i == 1) {
        //printf("Got key press -- %c\n", text[0]);
        if(text[0] == 'a' && start) {
            PADDLE.left();
        }
        else if(text[0] == 'd' && start) {
            PADDLE.right();
        }
    }    
}







/*
* Eventloop
*/
void eventloop(XInfo& xinfo) {
    dList.push_front(&TEXT);
    dList.push_front(&BLOCKS);
    dList.push_front(&PADDLE);
    dList.push_front(&BALL);
    //dList.push_front(&TEXT);
    unsigned int after = 0;

    XEvent event;
    while (true) {
        if( XPending( xinfo.display )>0 ) {
            XNextEvent( xinfo.display, &event );
        }
        switch ( event.type )
        {
            case KeyRelease:
                HandleKeyRelease(xinfo, event);
                break;  
            case MotionNotify:
                HandleMotion(event);
                break; 
            case ConfigureNotify:
                handleResize(xinfo, event);
                break; 
            case KeyPress:   
                HandleKeyPress(xinfo,event); 
                break;   
        }
        


        // usleep(1000000/60);
        // if (start == true) {
        //     HandleAnimation(xinfo);
        // }
        // repaint(xinfo);

        unsigned int before = GetTheCurrentTime();
        if( before-after > 1000000/FPS) {
            if(start) {
                HandleAnimation(xinfo);
            }
            paddleend = GetSeconds();
            if( paddleend - paddlestart >=9 && largepaddle ){
                largepaddle = false;
                widthofpaddle = widthofpaddle/2;
            }
            repaint(xinfo);
            after = GetTheCurrentTime();
        }
        else if (XPending(xinfo.display) == 0) {
            usleep(1000000/FPS - (before-after));
        }
    }
}




/*
 *   Start executing here.
 *   First initialize window.
 *   Next loop responding to events.
 *   Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char* argv[] ) {

    XInfo xInfo;
    if (!sina) {
    cout << "Enter the desired frame-rate" << endl;
    cin >> FPS;
    cout << "Enter the speed of the Ball " << endl;
    cin >> speed; 
    }
    directioninx = speed;
    directioniny = speed;

    initX(argc, argv, xInfo);
    eventloop(xInfo);
    XCloseDisplay(xInfo.display);
}
