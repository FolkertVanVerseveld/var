#include <stdio.h>
#include <stdlib.h>
#include <smt/smt.h>

#define TEXSZ 2

#define INIT_GFX 1

unsigned win = SMT_RES_INVALID, gl = SMT_RES_INVALID;
unsigned init = 0;
unsigned spr[TEXSZ];
GLuint tex[TEXSZ];

static void gfx_free(void)
{
	if (!(init & INIT_GFX)) return;
	init &= ~INIT_GFX;
	for (unsigned i = 0; i < TEXSZ; ++i) {
		if (spr[i] != SMT_RES_INVALID)
			smtFreespr(spr[i]);
		spr[i] = SMT_RES_INVALID;
	}
	glDeleteTextures(TEXSZ, tex);
}

static void cleanup(void)
{
	gfx_free();
	if (gl != SMT_RES_INVALID) {
		smtFreegl(gl);
		gl = SMT_RES_INVALID;
	}
	if (win != SMT_RES_INVALID) {
		smtFreewin(win);
		win = SMT_RES_INVALID;
	}
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(1, 0); glVertex2f(1, 0);
	glTexCoord2f(1, 1); glVertex2f(1, 1);
	glTexCoord2f(0, 1); glVertex2f(0, 1);
	glEnd();
}

static int gfx_init(void)
{
	const char *name = NULL;
	char buf[256];
	for (unsigned i = 0; i < TEXSZ; ++i)
		spr[i] = SMT_RES_INVALID;
	glGenTextures(TEXSZ, tex);
	init |= INIT_GFX;
	if (smtCreatespr(&spr[0], 0, 0, name = "data/background.png", tex[0], 0))
		goto fail;
	if (smtCreatespr(&spr[1], 0, 0, name = "data/font.png", tex[1], 0))
		goto fail;
	return 0;
fail:
	snprintf(buf, sizeof buf, "No such file or corrupted:\n%s", name);
	smtMsg(SMT_MSG_ERR, 0, "Fatal error", buf);
	return 1;
}

int main(int argc, char **argv) {
	int ret = 1;
	ret = smtInit(&argc, argv);
	if (ret) goto fail;
	atexit(cleanup);
	ret = smtCreatewin(&win, 400, 300, "ClickOMatic", SMT_WIN_VISIBLE | SMT_WIN_BORDER);
	if (ret) goto fail;
	ret = smtCreategl(&gl, win);
	if (ret) goto fail;
	if (gfx_init())
		goto fail;
	while (1) {
		unsigned ev;
		while ((ev = smtPollev()) != SMT_EV_DONE) {
			switch (ev) {
			case SMT_EV_QUIT: goto end;
			}
		}
		display();
		smtSwapgl(win);
	}
end:
fail:
	cleanup();
	return ret;
}
