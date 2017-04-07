#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <stdlib.h>

static ALCdevice  *dev = NULL;
static ALCcontext *ctx = NULL;

struct alc_info {
	const ALCchar *device_default;
	const ALCchar *capture_default;
	const ALCchar *device;
	const ALCchar *capture;
	const ALCchar *extensions;
	ALCint major, minor;
	ALCint attributes;
} alc_info;

static void cleanup(void)
{
	if (ctx) {
		alcMakeContextCurrent(NULL);
		alcDestroyContext(ctx);
		ctx = NULL;
	}
	if (dev) {
		alcCloseDevice(dev);
		dev = NULL;
	}
}

static const char *strerror_alc(ALCenum err)
{
	switch (err) {
	case ALC_NO_ERROR      : return "no error";
	case ALC_INVALID_DEVICE: return "invalid device";
	case ALC_INVALID_ENUM  : return "invalid enum";
	case ALC_INVALID_VALUE : return "invalid value";
	case ALC_OUT_OF_MEMORY : return "out of memory";
	default: return "unknown error";
	}
}

static const char *strerror_al(ALenum err)
{
	switch (err) {
	case AL_NO_ERROR: return "no error";
	case AL_INVALID_NAME: return "invalid name";
	case AL_INVALID_ENUM: return "invalid enum";
	case AL_INVALID_VALUE: return "invalid value";
	case AL_INVALID_OPERATION: return "invalid operation";
	case AL_OUT_OF_MEMORY: return "out of memory";
	default: return "unknown error";
	}
}

static void perror_alc2(const char *str, ALCenum err)
{
	fprintf(stderr, "%s: %s\n", str, strerror_alc(err));
}

static void perror_alc(const char *str)
{
	perror_alc2(str, alcGetError(dev));
}

static void perror_al2(const char *str, ALenum err)
{
	fprintf(stderr, "%s: %s\n", str, strerror_al(err));
}

static void perror_al(const char *str)
{
	perror_al2(str, alGetError());
}

static void alc_init(struct alc_info *info, ALCdevice *dev)
{
#define chk(str) \
	if ((err = alcGetError(NULL)) != ALC_NO_ERROR)\
		perror_alc2(str, err)
	ALCenum err;
	info->device_default = alcGetString(dev, ALC_DEFAULT_DEVICE_SPECIFIER);
	chk("No default OpenAL device specifier");
	info->capture_default = alcGetString(dev, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	chk("No default OpenAL capture device specifier");
	info->device = alcGetString(dev, ALC_DEVICE_SPECIFIER);
	chk("No OpenAL device specifier");
	info->capture = alcGetString(dev, ALC_CAPTURE_DEVICE_SPECIFIER);
	chk("No OpenAL capture device specifier");
	info->extensions = alcGetString(dev, ALC_EXTENSIONS);
	chk("No OpenAL extensions");
	alcGetIntegerv(dev, ALC_MAJOR_VERSION, sizeof(info->major), &info->major);
	chk("No OpenAL major version");
	alcGetIntegerv(dev, ALC_MINOR_VERSION, sizeof(info->minor), &info->minor);
	chk("No OpenAL minor version");
	alcGetIntegerv(dev, ALC_ATTRIBUTES_SIZE, sizeof(info->attributes), &info->attributes);
	chk("No OpenAL attributes");
#undef chk
}

static void alc_dump(const struct alc_info *info)
{
	puts("OpenAL context info:");
	printf("Default device specifier: %s\n", info->device_default);
	printf("Default capture device specifier: %s\n", info->capture_default);
	printf("Device specifier: %s\n", info->device);
	printf("Capture device specifier: %s\n", info->capture);
	printf("Extensions: %s\n", info->extensions);
	printf("Version %d.%d\n", info->major, info->minor);
	printf("Attributes size: %d\n", info->attributes);
}

int main(void)
{
	ALCenum alcerr;
	ALenum alerr;
	int ret = 1;
	atexit(cleanup);
	puts("Default headless setup:");
	alc_init(&alc_info, NULL);
	alc_dump(&alc_info);
	dev = alcOpenDevice(NULL);
	if (!dev) {
		perror_alc("Can't open OpenAL device");
		goto fail;
	}
	puts("Default created setup:");
	alc_init(&alc_info, dev);
	alc_dump(&alc_info);
	ctx = alcCreateContext(dev, NULL);
	if (!ctx) {
		perror_alc("Can't create OpenAL context");
		goto fail;
	}
	alcMakeContextCurrent(ctx);
	ret = 0;
fail:
	return ret;
}
