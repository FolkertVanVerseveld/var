#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <smt/smt.h>

#define WIDTH 640
#define HEIGHT 480

#define TEXSZ 2

#define INIT_GFX 1
#define INIT_SFX 2

static unsigned win = SMT_RES_INVALID, gl = SMT_RES_INVALID;
static unsigned init = 0, mode = 0;
static unsigned x = 0, y = 0;
static unsigned width = WIDTH, height = HEIGHT;
static unsigned spr[TEXSZ];
static GLuint tex[TEXSZ];

static ALuint *buf = NULL;
static ALuint *ch  = NULL;

static size_t nogg;

static void sfx_free(void)
{
	if (!(init & INIT_SFX)) return;
	init &= ~INIT_SFX;
	if (buf) {
		alDeleteBuffers(nogg, buf);
		free(buf);
		buf = NULL;
	}
	if (ch) {
		alDeleteSources(nogg, ch);
		free(ch);
		ch = NULL;
	}
}

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
	sfx_free();
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
	glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
	glViewport(0, 0, width, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(1, 0); glVertex2f(WIDTH, 0);
	glTexCoord2f(1, 1); glVertex2f(WIDTH, HEIGHT);
	glTexCoord2f(0, 1); glVertex2f(0, HEIGHT);
	glEnd();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static int sfx_init(void)
{
	int ret = 1;
	struct dirent **names;
	int n = scandir("data", &names, NULL, NULL);
	init |= INIT_SFX;
	nogg = 0;
	if (n < 0) {
		perror("data");
		goto fail;
	}
	// not very efficient, but i'm lazy
	for (int i = 0; i < n; ++i) {
		const char *name = names[i]->d_name;
		size_t len = strlen(name);
		if (len > strlen(".ogg") && !strcmp(".ogg", name + len - strlen(".ogg")))
			++nogg;
	}
	buf = malloc(nogg * sizeof(ALuint));
	if (!buf) {
		fputs("no buffers: out of memory\n", stderr);
		goto fail;
	}
	alGenBuffers(nogg, buf);
	ch = malloc(nogg * sizeof(ALuint));
	if (!ch) {
		fputs("no channels: out of memory\n", stderr);
		goto fail;
	}
	alGenSources(nogg, ch);
	unsigned o_i = 0;
	for (int i = 0; i < n; ++i) {
		const char *name = names[i]->d_name;
		size_t len = strlen(name);
		char *ogg;
		ALenum format;
		ALsizei freq;
		size_t n;
		if (len > strlen(".ogg") && !strcmp(".ogg", name + len - strlen(".ogg"))) {
			puts(name);
			char fname[256];
			snprintf(fname, sizeof fname, "data/%s", name);
			if (smtOggfv(fname, &ogg, &n, &format, &freq)) {
				char msg[256];
				snprintf(msg, sizeof msg, "invalid clip: %s\n", fname);
				fputs(msg, stderr);
				smtMsg(SMT_MSG_ERR, 0, "Fatal error", msg);
				goto fail;
			}
			alBufferData(buf[o_i], format, ogg, n, freq);
			free(ogg);
			++o_i;
		}
	}
	ret = 0;
fail:
	if (n) {
		for (int i = 0; i < n; ++i)
			free(names[i]);
		free(names);
	}
	return ret;
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

static void fetchbounds(void)
{
	int xp, yp;
	unsigned w, h;
	if (!smtGetPoswin(win, &xp, &yp)) {
		x = xp;
		y = yp;
	}
	if (!smtGetSizewin(win, &w, &h)) {
		width = w;
		height = h;
	}
}

static int fkey(unsigned key)
{
	switch (key) {
		case 27: return 1;
		case SMT_KEY_F(4): {
			unsigned modep = mode ^ SMT_WIN_FULL_FAKE;
			if (!smtMode(win, modep))
				mode = modep;
			break;
		}
	}
	return 0;
}

unsigned sfx_play(int nr, unsigned index)
{
	if (index >= nogg) {
		fprintf(stderr, "sfx: bad index: %u\n", index);
		return -1;
	}
	if (nr < 0) {
		nr = 0; // default channel
		for (unsigned i = 0; i < nogg; ++i) {
			ALint state;
			alGetSourcei(ch[i], AL_SOURCE_STATE, &state);
			if (state == AL_PLAYING)
				continue;
			nr = i;
			break;
		}
	} else if (nr > 0)
		nr %= nogg;
	ALuint src = ch[nr];
	alSourcei(src, AL_BUFFER, buf[index]);
	alSourcePlay(src);
	return nr;
}

int main(int argc, char **argv) {
	int ret = 1;
	ret = smtInit(&argc, argv);
	if (ret) goto fail;
	atexit(cleanup);
	ret = smtCreatewin(&win, WIDTH, HEIGHT, "ClickOMatic", SMT_WIN_VISIBLE | SMT_WIN_BORDER);
	if (ret) goto fail;
	ret = smtCreategl(&gl, win);
	if (ret) goto fail;
	if (gfx_init())
		goto fail;
	if (sfx_init())
		goto fail;
	srand(time(NULL));
	while (1) {
		unsigned ev;
		while ((ev = smtPollev()) != SMT_EV_DONE) {
			switch (ev) {
			case SMT_EV_QUIT: goto end;
			case SMT_EV_KEY_UP:
				if (fkey(smt.kbp.virt))
					goto end;
				break;
			case SMT_EV_KEY_DOWN:
				sfx_play(-1, rand() % nogg);
				break;
			}
		}
		fetchbounds();
		display();
		smtSwapgl(win);
	}
end:
	ret = 0;
fail:
	cleanup();
	return ret;
}
