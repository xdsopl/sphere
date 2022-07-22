/*
sphere - tessellate a sphere through subdivision of triangles
Written in 2005 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <math.h>
#include <stdlib.h>
#include <getopt.h>
#include <SDL.h>
#include <GL/gl.h>

int order_n;
int normalize;
int platonic_solid;
int flat_triangles;
int mouse_rot;
int screen_w;
int screen_h;
int focus;
GLint sphere_list;

GLfloat rot[16], ident[16] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};
GLfloat light_vector[4] = {4.0, 4.0, 10.0, 0.0};
GLfloat light_ambient[4] = {0.2, 0.2, 0.2, 0.0};
GLfloat material_diffuse[4] = {0.05, 0.05, 0.05, 0.0};
GLfloat material_specular[4] = {0.2, 0.2, 0.2, 0.0};
GLfloat material_shiny[1] = {50.0};

#define NORM(X, Y, Z) do { double f = 1.0 / sqrt(X * X + Y * Y + Z * Z); X *= f; Y *= f; Z *= f; } while (0)

void triangle(int n, double Ax, double Ay, double Az, double Bx, double By, double Bz, double Cx, double Cy, double Cz)
{
	double Nx, Ny, Nz; /* normals needed for opengl to make lighting work */
	double Dx, Dy, Dz, Ex, Ey, Ez, Fx, Fy, Fz;

	if (n < order_n) {
		Dx = 0.5 * (Ax + Bx);
		Dy = 0.5 * (Ay + By);
		Dz = 0.5 * (Az + Bz);
		Ex = 0.5 * (Bx + Cx);
		Ey = 0.5 * (By + Cy);
		Ez = 0.5 * (Bz + Cz);
		Fx = 0.5 * (Ax + Cx);
		Fy = 0.5 * (Ay + Cy);
		Fz = 0.5 * (Az + Cz);
		triangle(n + 1, Ax, Ay, Az, Dx, Dy, Dz, Fx, Fy, Fz);
		triangle(n + 1, Dx, Dy, Dz, Bx, By, Bz, Ex, Ey, Ez);
		triangle(n + 1, Fx, Fy, Fz, Ex, Ey, Ez, Cx, Cy, Cz);
		triangle(n + 1, Fx, Fy, Fz, Dx, Dy, Dz, Ex, Ey, Ez);
		return;
	}

	if (normalize) {
		/* with normalizing, we put them on the spheres surface */
		NORM(Ax, Ay, Az);
		NORM(Bx, By, Bz);
		NORM(Cx, Cy, Cz);
		if (flat_triangles) {
			/* to get flat triangles, we use the normal on the middle of the triangle */
			Nx = (1.0 / 3.0) * (Ax + Bx + Cx);
			Ny = (1.0 / 3.0) * (Ay + By + Cy);
			Nz = (1.0 / 3.0) * (Az + Bz + Cz);
			NORM(Nx, Ny, Nz);
			glNormal3d(Nx, Ny, Nz);
			glVertex3d(Ax, Ay, Az);
			glVertex3d(Bx, By, Bz);
			glVertex3d(Cx, Cy, Cz);
		} else {
			glNormal3d(Ax, Ay, Az);
			glVertex3d(Ax, Ay, Az);
			glNormal3d(Bx, By, Bz);
			glVertex3d(Bx, By, Bz);
			glNormal3d(Cx, Cy, Cz);
			glVertex3d(Cx, Cy, Cz);
		}
	} else {
		/* without normalizing it makes no sence to use non flat triangles */
		Nx = (Ay - By) * (Bz - Cz) - (Az - Bz) * (By - Cy);
		Ny = (Az - Bz) * (Bx - Cx) - (Ax - Bx) * (Bz - Cz);
		Nz = (Ax - Bx) * (By - Cy) - (Ay - By) * (Bx - Cx);
		NORM(Nx, Ny, Nz);
		glNormal3d(Nx, Ny, Nz);
		glVertex3d(Ax, Ay, Az);
		glVertex3d(Bx, By, Bz);
		glVertex3d(Cx, Cy, Cz);
	}
}

/* tetrahedron with radius 1 */

#define tet_r (1.0)
#define tet_a_2 (3.0 / (2.0 * sqrt(3.0)) * tet_r)
#define tet_d (0.5 * tet_r)

#define tet_Ax (-tet_a_2)
#define tet_Ay (-tet_d)
#define tet_Az (tet_d)

#define tet_Bx (tet_a_2)
#define tet_By (-tet_d)
#define tet_Bz (tet_d)

#define tet_Cx (0.0)
#define tet_Cy (tet_r)
#define tet_Cz (0.0)

#define tet_Dx (0.0)
#define tet_Dy (-tet_d)
#define tet_Dz (-tet_r)

void tetrahedron()
{
	triangle(0, tet_Ax, tet_Ay, tet_Az, tet_Bx, tet_By, tet_Bz, tet_Cx, tet_Cy, tet_Cz);
	triangle(0, tet_Bx, tet_By, tet_Bz, tet_Dx, tet_Dy, tet_Dz, tet_Cx, tet_Cy, tet_Cz);
	triangle(0, tet_Dx, tet_Dy, tet_Dz, tet_Ax, tet_Ay, tet_Az, tet_Cx, tet_Cy, tet_Cz);
	triangle(0, tet_Ax, tet_Ay, tet_Az, tet_Dx, tet_Dy, tet_Dz, tet_Bx, tet_By, tet_Bz);
}

/* octahedron with radius 1 */

#define oct_r (1.0)
#define oct_a_2 (1.0 / sqrt(2.0) * oct_r)

#define oct_Ax (-oct_a_2)
#define oct_Ay (0.0)
#define oct_Az (oct_a_2)

#define oct_Bx (oct_a_2)
#define oct_By (0.0)
#define oct_Bz (oct_a_2)

#define oct_Cx (0.0)
#define oct_Cy (oct_r)
#define oct_Cz (0.0)

#define oct_Dx (oct_a_2)
#define oct_Dy (0.0)
#define oct_Dz (-oct_a_2)

#define oct_Ex (-oct_a_2)
#define oct_Ey (0.0)
#define oct_Ez (-oct_a_2)

#define oct_Fx (0.0)
#define oct_Fy (-oct_r)
#define oct_Fz (0.0)

void octahedron()
{
	triangle(0, oct_Ax, oct_Ay, oct_Az, oct_Bx, oct_By, oct_Bz, oct_Cx, oct_Cy, oct_Cz);
	triangle(0, oct_Bx, oct_By, oct_Bz, oct_Dx, oct_Dy, oct_Dz, oct_Cx, oct_Cy, oct_Cz);
	triangle(0, oct_Dx, oct_Dy, oct_Dz, oct_Ex, oct_Ey, oct_Ez, oct_Cx, oct_Cy, oct_Cz);
	triangle(0, oct_Ex, oct_Ey, oct_Ez, oct_Ax, oct_Ay, oct_Az, oct_Cx, oct_Cy, oct_Cz);
	triangle(0, oct_Ax, oct_Ay, oct_Az, oct_Ex, oct_Ey, oct_Ez, oct_Fx, oct_Fy, oct_Fz);
	triangle(0, oct_Ax, oct_Ay, oct_Az, oct_Ex, oct_Ey, oct_Ez, oct_Fx, oct_Fy, oct_Fz);
	triangle(0, oct_Ax, oct_Ay, oct_Az, oct_Ex, oct_Ey, oct_Ez, oct_Fx, oct_Fy, oct_Fz);
	triangle(0, oct_Ex, oct_Ey, oct_Ez, oct_Dx, oct_Dy, oct_Dz, oct_Fx, oct_Fy, oct_Fz);
	triangle(0, oct_Dx, oct_Dy, oct_Dz, oct_Bx, oct_By, oct_Bz, oct_Fx, oct_Fy, oct_Fz);
	triangle(0, oct_Bx, oct_By, oct_Bz, oct_Ax, oct_Ay, oct_Az, oct_Fx, oct_Fy, oct_Fz);
}

/* icosahedron with radius 1 */
#define ico_r (1.0)
#define ico_a_2 (2.0 / sqrt(10.0 + 2.0 * sqrt(5.0)) * ico_r)
#define ico_b_2 (0.5 * (1.0 + sqrt(5.0)) * ico_a_2)

#define ico_Ax (-ico_a_2)
#define ico_Ay (-ico_b_2)
#define ico_Az (0.0)

#define ico_Bx (ico_a_2)
#define ico_By (-ico_b_2)
#define ico_Bz (0.0)

#define ico_Cx (0.0)
#define ico_Cy (-ico_a_2)
#define ico_Cz (ico_b_2)

#define ico_Dx (ico_b_2)
#define ico_Dy (0.0)
#define ico_Dz (ico_a_2)

#define ico_Ex (-ico_b_2)
#define ico_Ey (0.0)
#define ico_Ez (ico_a_2)

#define ico_Fx (0.0)
#define ico_Fy (ico_a_2)
#define ico_Fz (ico_b_2)

#define ico_Gx (ico_a_2)
#define ico_Gy (ico_b_2)
#define ico_Gz (0.0)

#define ico_Hx (-ico_a_2)
#define ico_Hy (ico_b_2)
#define ico_Hz (0.0)

#define ico_Ix (0.0)
#define ico_Iy (-ico_a_2)
#define ico_Iz (-ico_b_2)

#define ico_Jx (ico_b_2)
#define ico_Jy (0.0)
#define ico_Jz (-ico_a_2)

#define ico_Kx (-ico_b_2)
#define ico_Ky (0.0)
#define ico_Kz (-ico_a_2)

#define ico_Lx (0.0)
#define ico_Ly (ico_a_2)
#define ico_Lz (-ico_b_2)

void icosahedron()
{
	triangle(0, ico_Ax, ico_Ay, ico_Az, ico_Bx, ico_By, ico_Bz, ico_Cx, ico_Cy, ico_Cz);
	triangle(0, ico_Bx, ico_By, ico_Bz, ico_Dx, ico_Dy, ico_Dz, ico_Cx, ico_Cy, ico_Cz);
	triangle(0, ico_Ax, ico_Ay, ico_Az, ico_Cx, ico_Cy, ico_Cz, ico_Ex, ico_Ey, ico_Ez);
	triangle(0, ico_Cx, ico_Cy, ico_Cz, ico_Fx, ico_Fy, ico_Fz, ico_Ex, ico_Ey, ico_Ez);
	triangle(0, ico_Cx, ico_Cy, ico_Cz, ico_Dx, ico_Dy, ico_Dz, ico_Fx, ico_Fy, ico_Fz);
	triangle(0, ico_Fx, ico_Fy, ico_Fz, ico_Dx, ico_Dy, ico_Dz, ico_Gx, ico_Gy, ico_Gz);
	triangle(0, ico_Fx, ico_Fy, ico_Fz, ico_Hx, ico_Hy, ico_Hz, ico_Ex, ico_Ey, ico_Ez);
	triangle(0, ico_Fx, ico_Fy, ico_Fz, ico_Gx, ico_Gy, ico_Gz, ico_Hx, ico_Hy, ico_Hz);
	triangle(0, ico_Ix, ico_Iy, ico_Iz, ico_Bx, ico_By, ico_Bz, ico_Ax, ico_Ay, ico_Az);
	triangle(0, ico_Bx, ico_By, ico_Bz, ico_Ix, ico_Iy, ico_Iz, ico_Jx, ico_Jy, ico_Jz);
	triangle(0, ico_Ax, ico_Ay, ico_Az, ico_Kx, ico_Ky, ico_Kz, ico_Ix, ico_Iy, ico_Iz);
	triangle(0, ico_Ix, ico_Iy, ico_Iz, ico_Kx, ico_Ky, ico_Kz, ico_Lx, ico_Ly, ico_Lz);
	triangle(0, ico_Ix, ico_Iy, ico_Iz, ico_Lx, ico_Ly, ico_Lz, ico_Jx, ico_Jy, ico_Jz);
	triangle(0, ico_Jx, ico_Jy, ico_Jz, ico_Lx, ico_Ly, ico_Lz, ico_Gx, ico_Gy, ico_Gz);
	triangle(0, ico_Kx, ico_Ky, ico_Kz, ico_Hx, ico_Hy, ico_Hz, ico_Lx, ico_Ly, ico_Lz);
	triangle(0, ico_Hx, ico_Hy, ico_Hz, ico_Gx, ico_Gy, ico_Gz, ico_Lx, ico_Ly, ico_Lz);
	triangle(0, ico_Dx, ico_Dy, ico_Dz, ico_Bx, ico_By, ico_Bz, ico_Jx, ico_Jy, ico_Jz);
	triangle(0, ico_Gx, ico_Gy, ico_Gz, ico_Dx, ico_Dy, ico_Dz, ico_Jx, ico_Jy, ico_Jz);
	triangle(0, ico_Ax, ico_Ay, ico_Az, ico_Ex, ico_Ey, ico_Ez, ico_Kx, ico_Ky, ico_Kz);
	triangle(0, ico_Ex, ico_Ey, ico_Ez, ico_Hx, ico_Hy, ico_Hz, ico_Kx, ico_Ky, ico_Kz);
}

void init_sphere()
{
	GLfloat color[4] = {1.0, 1.0, 1.0, 0.0};
	sphere_list = glGenLists(1);
	glNewList(sphere_list, GL_COMPILE);
		glColor4fv(color);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, material_shiny);
		glBegin(GL_TRIANGLES);
			if (0 == platonic_solid) tetrahedron();
			if (1 == platonic_solid) octahedron();
			if (2 == platonic_solid) icosahedron();
		glEnd();
	glEndList();
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glMultMatrixf(rot);
	glCallList(sphere_list);
	glFlush();
	SDL_GL_SwapBuffers();
}

void init_gl()
{
	GLfloat diffuse[4] = {10.0, 10.0, 10.0, 0.0};
	GLfloat ambient[4] = {0.02, 0.02, 0.02, 0.0};
	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glLightfv(GL_LIGHT0, GL_POSITION, light_vector);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glLineWidth(1.0);
	glMatrixMode(GL_MODELVIEW);
}

void resize_gl(const int width, const int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat wh = (GLfloat)width / (GLfloat)height;
	GLfloat hw = (GLfloat)height / (GLfloat)width;
	if (height < width)
		glFrustum(-wh, wh, -1.0, 1.0, 5.0, 60.0);
	else
		glFrustum(-1.0, 1.0, -hw, hw, 5.0, 60.0);
	glMatrixMode(GL_MODELVIEW);
}

void rotate(GLfloat *m, const GLfloat a, const GLfloat x, const GLfloat y, const GLfloat z)
{
	GLfloat c = cos(a);
	GLfloat s = sin(a);
	GLfloat r[16];
	r[0]=x*x*(1.0-c)+c; r[1]=x*y*(1.0-c)-z*s; r[2]=x*z*(1.0-c)+y*s; r[3]=0.0;
	r[4]=y*x*(1.0-c)+z*s; r[5]=y*y*(1.0-c)+c; r[6]=y*z*(1.0-c)-x*s; r[7]=0.0;
	r[8]=x*z*(1.0-c)-y*s; r[9]=y*z*(1.0-c)+x*s; r[10]=z*z*(1.0-c)+c; r[11]=0.0;
	r[12]=0.0; r[13]=0.0; r[14]=0.0; r[15]=1.0;
	GLfloat t[16];

	for (int i = 0; i < 4; i++) {
		for (int k = 0; k < 4; k++) {
			t[4*i+k] = 0.0;
			for (int j = 0; j < 4; j++) {
				t[4*i+k] += m[4*i+j] * r[4*j+k];
			}
		}
	}
	for (int i = 0; i < 16; i++)
		m[i] = t[i];
}

void handle_events()
{
	SDL_Surface *screen;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_ACTIVEEVENT:
				if (event.active.gain)
					focus = 1;
				else
					focus = 0;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_r:
						memcpy(rot, ident, sizeof(rot));
						break;
					case SDLK_p:
						glPolygonMode(GL_FRONT, GL_POINT);
						glDisable(GL_LIGHTING);
						glEnable(GL_CULL_FACE);
						glEnable(GL_DEPTH_TEST);
						break;
					case SDLK_o:
						glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
						glDisable(GL_LIGHTING);
						glDisable(GL_CULL_FACE);
						glDisable(GL_DEPTH_TEST);
						break;
					case SDLK_l:
						glPolygonMode(GL_FRONT, GL_LINE);
						glDisable(GL_LIGHTING);
						glEnable(GL_CULL_FACE);
						glEnable(GL_DEPTH_TEST);
						break;
					case SDLK_m:
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						glDisable(GL_LIGHTING);
						glDisable(GL_CULL_FACE);
						glDisable(GL_DEPTH_TEST);
						break;
					case SDLK_f:
						glPolygonMode(GL_FRONT, GL_FILL);
						glEnable(GL_LIGHTING);
						glEnable(GL_CULL_FACE);
						glEnable(GL_DEPTH_TEST);
						break;
					case SDLK_g:
						glPolygonMode(GL_FRONT, GL_FILL);
						glDisable(GL_LIGHTING);
						glEnable(GL_CULL_FACE);
						glEnable(GL_DEPTH_TEST);
						break;
					case SDLK_q:
						exit(0);
						break;
					case SDLK_ESCAPE:
						exit(0);
						break;
					default:
						break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse_rot = 1;
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_rot = 0;
				break;
			case SDL_MOUSEMOTION:
				if (mouse_rot) {
					rotate(rot, -3.14159265 * event.motion.yrel / screen_h, 1.0, 0.0, 0.0);
					rotate(rot, -3.14159265 * event.motion.xrel / screen_w, 0.0, 1.0, 0.0);
				}
				break;
			case SDL_VIDEORESIZE:
				screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
				if (screen) {
					screen_w = screen->w;
					screen_h = screen->h;
					resize_gl(screen_w, screen_h);
				}
				break;
			case SDL_QUIT:
				exit(1);
				break;
			default:
				break;
		}
	}

}

void init(int argc, char **argv)
{
	for (;;) {
		switch (getopt (argc, argv, "htfn:p:")) {
			case 'h':
				fprintf(stderr, "usage: %s [-n {0..6}] [-h] [-t] [-f] [-p {0..2}]\n", argv[0]);
				exit(0);
				break;
			case 't':
				normalize = 0;
				break;
			case 'f':
				flat_triangles = 1;
				break;
			case 'n':
				order_n = atoi(optarg);
				if (order_n < 0 || order_n > 6) {
					fprintf(stderr, "only 0 <= n <= 6 allowed.\n");
					/* using n > 7 might fuck up your machine. so change the source if you really want to do it */
					exit(0);
				}
				break;
			case 'p':
				platonic_solid = atoi(optarg);
				if (platonic_solid < 0 || platonic_solid > 2) {
					fprintf(stderr, "only 0 <= p <= 2 allowed.\n");
					exit(0);
				}
				break;
			case -1:
				return;
				break;
			default:
				fprintf(stderr, "usage: %s [-n {0..6}] [-h] [-t] [-f] [-p {0..2}]\n", argv[0]);
				exit(1);
				break;
		}
	}
}

int main(int argc, char **argv)
{
	SDL_Surface *screen;
	screen_w = 640;
	screen_h = 480;
	mouse_rot = 0;
	memcpy(rot, ident, sizeof(rot));
	order_n = 2;
	normalize = 1;
	platonic_solid = 2;
	focus = 1;
	flat_triangles = 0;
	init(argc, argv);
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(screen_w, screen_h, 32, SDL_OPENGL | SDL_RESIZABLE | SDL_DOUBLEBUF);
	if (!screen)
		return 1;
	SDL_WM_SetCaption("Sphere /(^,^)\\", "sphere");
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	init_gl();
	resize_gl(screen_w, screen_h);
	init_sphere();
	fprintf(stderr, "press 'l' and 'm' for line modes, 'p' and 'o' for point modes and 'f' and 'g' for fill modes. 'r' to reset rotation and 'q' or escape to quit.\n");
	fflush(stderr);
	for (;;) {
		draw();
		if (!focus) SDL_Delay(100);
		handle_events();
	}
	SDL_Quit();
	return 0;
}

