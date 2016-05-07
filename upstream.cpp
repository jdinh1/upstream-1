//██╗   ██╗██████╗ ███████╗████████╗██████╗ ███████╗ █████╗ ███╗   ███╗
//██║   ██║██╔══██╗██╔════╝╚══██╔══╝██╔══██╗██╔════╝██╔══██╗████╗ ████║
//██║   ██║██████╔╝███████╗   ██║   ██████╔╝█████╗  ███████║██╔████╔██║
//██║   ██║██╔═══╝ ╚════██║   ██║   ██╔══██╗██╔══╝  ██╔══██║██║╚██╔╝██║
//╚██████╔╝██║     ███████║   ██║   ██║  ██║███████╗██║  ██║██║ ╚═╝ ██║
// ╚═════╝ ╚═╝     ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝
///////////////////////////////////////////////////////////////////////
// CONTRIBUTORS: JONATHAN DINH
//               QUY NGUYEN
//               JOHN HARGREAVES
//               KEVIN JENKIN
///////////////////////////////////////////////////////////////////////
#include "upstream.h"

int main(void)
{
	int done=0;
	srand(time(NULL));
	srand(time(0));

	//declare game object
	Game menu;
	init_menu_opengl(&menu);
	init_menu(&menu);

	Game game;
	initXWindows(&game);
	init_opengl(&game);
	init_game(&game);

	//background sound
	playSounds("./wav/background.wav", 0.1f, true, game.muted);

	//while (!menu.menu_done) {
	//while(XPending(dpy)) {
	//XEvent e;
	//XNextEvent(dpy, &e);
	//check_mouse(&e, &game);
	//menu.menu_done = check_keys(&e, &game);
	//}
	//render_menu(&game);
	//glXSwapBuffers(dpy, win);
	//}

	while (!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		// physics();
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

double timeDiff(struct timespec *start, struct timespec *end)
{
	return (double)(end->tv_sec - start->tv_sec ) +
		(double)(end->tv_nsec - start->tv_nsec) * oobillion;
}

void timeCopy(struct timespec *dest, struct timespec *source)
{
	memcpy(dest, source, sizeof(struct timespec));
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Testing framework for Upstream");
}

void cleanupXWindows(void)
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(Game *game)
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=game->windowWidth, h=game->windowHeight;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		//std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
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

void init_opengl(Game *game)
{
	//OpenGL initialization
	glViewport(0, 0, game->windowWidth, game->windowHeight);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, game->windowWidth, 0, game->windowHeight, -1, 1);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	//Set the screen background color
	glClearColor(0.1, 40.1, 200.1, 1.0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
}

void movement(Game *game)
{
	collision(game);

	//move frog
	if (game->playing==true && game->splash->getFrame()>=400) {
        if (!game->frog->rocket() ){
		game->frog->move((float)game->c.center[0], (float)game->c.center[1],
				(float)game->c.velocity[0], (float)game->c.velocity[1]);
		}else{
				game->frog->move( (float)game->c.center[0], game->frog->getYpos(),
				game->frog->getXvel(), game->frog->getYvel() );
				game->c.center[0] = game->frog->getXpos();
				game->c.center[1] = game->frog->getYpos();
				game->c.velocity[0] = game->frog->getXvel();
				game->c.velocity[1] = game->frog->getYvel();
				game->c.isJumping = true;
				game->c.isStanding = false;
				}
	//move fly
	int x=rand()%10+1;int y =rand()%10+1;
	x=6-x;
	y=6-y;
	game->fly->move(game->fly->getXpos()+x/2,game->fly->getYpos()+y,x/10,y/10);
		checkLilies(game);
	}
}

void drawCircle(float x, float y, float radius, int detail)
{
	float radian = 2.0 * 3.14;
	glPushMatrix();
	glColor3ub(90,140,90);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i <= detail; i++) {
		glVertex2f(
				x + (radius * cos(i * radian / detail)),
				y + (radius * sin(i * radian / detail))
				);
	}
	glEnd();
	glPopMatrix();
}

void render(Game *game)
{
	glClear(GL_COLOR_BUFFER_BIT);
	game->water[0]->render();
	game->water[1]->render();
	game->water[2]->render();
	drawScore(game->score, game, 20);
	drawLilies(game);
	screenUpdate(game);
	for (int i=0;i<4;i++) {
		game->log[i]->render();
	}
	game->gator->render();
	game->bridge->render();
	game->frog->render();
	game->splash->render();
	game->fly->render();

	//place holder for sound button
	drawCircle(480, game->windowHeight-30, 10, 10);
	// TEXT ====================================
    std::string mode;
    if (game->difficulty==EASY)
        mode ="EASY";
    if (game->difficulty==MED)
        mode ="Regular";
    if (game->difficulty==HARD)
        mode ="HARD";

	glBindTexture(GL_TEXTURE_2D, 0);
	Rect r;
	r.bot = game->windowHeight - 50;
	r.left = 300;
	r.center = 300;
	ggfrog40b(&r, 50, 0, "UPSTREAM!");
	ggprint40(&r, 50, 0, "Current Score: %d Mode: %s", game->score,mode.c_str());

	//std::cout<<" Score: "<<game->score<<" Mode: "<<mode<<std::endl;
	maxScore(game);
	ggprint40(&r, 0, 0, "High Score: %d", game->highscore[0]);


	r.bot -=50;
	//std::cout<<game->highscore[0]<<std::endl;
	//ggprint40(&r, 0, 0, "FROG Y: %f", game->windowHeight - game->c.newPosY);
	//ggprint40(&r, 0, 0, "FROG x: %f", game->c.newPosX);

	// ==========================================================
}
