//cs335 Spring 2015 HW-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 500
#define MAX_BOXES 10
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[MAX_BOXES];
    Shape sphere;
	Particle particle[MAX_PARTICLES];
	int n;
};

int bub = 0;

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//declare a box shape
	for(int i=0; i<5; i++){
        game.box[i].width = 100;
	    game.box[i].height = 10;
	    game.box[i].center.x = 120 + 5*65 + (i*15);
	    game.box[i].center.y = 500 - 5*60 + (i*10);
    }

    game.sphere.radius = 100;
	game.sphere.center.x = 80 + 5*65;
	game.sphere.center.y = 600 - 5*60;

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle   B = turn on water");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.0, 0.0, 0.3, 1.0);
}

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = -4.0;
	p->velocity.x = 0.5;
	//p->velocity.x =  game->n/10 + 1;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
		    return;
		int y = WINDOW_HEIGHT - e->xbutton.y;
		makeParticle(game, e->xbutton.x, y);
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
        if (key == XK_B) {
            //turn bubbler on or off
            bub =^ 1;
            while(bub){
                makeParticle(game, 100, 20);
                usleep(100000);
            }
        }
		//You may check other keys here.

	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	for(int i =0; i<game->n; i++){
		p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= 0.2;

		//check for collision with shapes...
		for(int i=0; i<5; i++) {
            Shape *s;
    		s = &game->box[i];

		    if (p->s.center.y >= s->center.y - (s->height) &&
	       	    	p->s.center.y <= s->center.y + (s->height) &&
			    p->s.center.x >= s->center.x - (s->width) &&
			    p->s.center.x <= s->center.x + (s->width)){
			    p->velocity.y *= -0.5;
			    //p->velocity.x += 0.5;
		    }
        }

        Shape *d;
        d = &game->sphere;
		if (p->s.center.y >= d->center.y - (d->radius) &&
			p->s.center.y <= d->center.y + (d->radius) &&
			p->s.center.x >= d->center.x - (d->radius) &&
			p->s.center.x <= d->center.x + (d->radius)){
		    p->velocity.y *= -0.5;
		    //p->velocity.x += 0.5;
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[game->n-1];
			game->n--;
		}

	}
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	//draw box
	for(int i=0; i<5; i++) {
        Shape *s;
	    glColor3ub(50,140,50);
	    s = &game->box[i];
	    glPushMatrix();
	    glTranslatef(s->center.x, s->center.y, s->center.z);
	    w = s->width;
	    h = s->height;
	    glBegin(GL_QUADS);
		    glVertex2i(-w,-h);
		    glVertex2i(-w, h);
		    glVertex2i( w, h);
		    glVertex2i( w,-h);
	    glEnd();
	    glPopMatrix();
    }

	Shape *d;
	glColor3ub(140, 50, 50);
	d = &game->sphere;
	glPushMatrix();
	glTranslatef(d->center.x, d->center.y, d->center.z);
	float x, y;
    float radius = d->radius;
	float twicePi = 2.0 * 3.142;
    x = d->center.x;
    y = d->center.y;
	glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for(int i=0; i<=20; i++) {
        glVertex2f (
                (x + (radius * cos(i *twicePi / 20))), (y + (radius * sin(i * twicePi / 20)))
                );
    }
	glEnd();
	glPopMatrix();


	//draw all particles here
	glPushMatrix();
	glColor3ub(100,100,250);
	for (int i=0; i<game->n; i++) {
		Vec *c = &game->particle[i].s.center;
		w = 3;
		h = 3;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}



