/*
license: public domain
SIGnature Magic identifier
similar to file(1), around 90 file formats are supported
*/
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#define debug(x) x
#else
#define debug(x) ((void)0)
#endif

void usage(int argc, char **argv) {
	fprintf(stderr, "usage: %s files\n", argc > 0 ? argv[0] : "sigm");
}

typedef const unsigned const char hs_t[];

hs_t hs_null = { 0x00 };
hs_t hs_pdb = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
hs_t hs_dba = { 0xBE, 0xBA, 0xFE, 0xCA };
hs_t hs_dba2 = { 0x00, 0x01, 0x42, 0x44 };
hs_t hs_tda = { 0x00, 0x01, 0x44, 0x54 };
hs_t hs_ddf = { 0x00, 0x01, 0x00, 0x00 };
hs_t hs_ico = { 0x00, 0x00, 0x01, 0x00 };
hs_t hs_3gpp = { 0x00, 0x00, 0x00, 0x14, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70 };
hs_t hs_3gpp2 = { 0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70 };
hs_t hs_mpeg4 = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x33, 0x67, 0x70 };
hs_t hs_lzw = { 0x1F, 0x9D };
hs_t hs_lzh = { 0x1F, 0xA0 };
hs_t hs_bz2 = { 0x42, 0x5A, 0x68 };
hs_t hs_gif87a = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
hs_t hs_gif89a = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
hs_t hs_tiffle = { 0x49, 0x49, 0x2A, 0x00 };
hs_t hs_tiffbe = { 0x4D, 0x4D, 0x00, 0x2A };
hs_t hs_cin = { 0x80, 0x2A, 0x5F, 0xD7 };
hs_t hs_dpxle = { 0x53, 0x44, 0x50, 0x58 };
hs_t hs_dpxbe = { 0x58, 0x50, 0x44, 0x53 };
hs_t hs_exr = { 0x76, 0x2F, 0x31, 0x01 };
hs_t hs_jpeg = { 0xFF, 0xD8, 0xFF };
hs_t hs_dosexe = { 0x4D, 0x5A };
hs_t hs_earc = { 0x50, 0x4B, 0x03, 0x04 };
hs_t hs_earc2 = { 0x50, 0x4B, 0x05, 0x06 };
hs_t hs_sarc = { 0x50, 0x4B, 0x07, 0x08 };
hs_t hs_rar = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
hs_t hs_rar5 = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00 };
hs_t hs_elf = { 0x7F, 0x45, 0x4C, 0x46 };
hs_t hs_png = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
hs_t hs_class = { 0xCA, 0xFE, 0xBA, 0xBE };
hs_t hs_utf8 = { 0xEF, 0xBB, 0xBF };
hs_t hs_mach32 = { 0xFE, 0xED, 0xFA, 0xCE };
hs_t hs_mach64 = { 0xFE, 0xED, 0xFA, 0xCF };
hs_t hs_mach32r = { 0xCE, 0xFA, 0xED, 0xFE };
hs_t hs_mach64r = { 0xCF, 0xFA, 0xED, 0xFE };
hs_t hs_utf16le = { 0xFF, 0xFE };
hs_t hs_utf32le = { 0xFF, 0xFE, 0x00, 0x00 };
hs_t hs_ps = { 0x25, 0x21, 0x50, 0x53 };
hs_t hs_pdf = { 0x25, 0x50, 0x44, 0x46 };
hs_t hs_asf = { 0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11 };
hs_t hs_asf2 = { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C };
hs_t hs_sdi = { 0x24, 0x53, 0x44, 0x49, 0x30, 0x30, 0x30, 0x31 };
hs_t hs_ogg = { 0x4F, 0x67, 0x67, 0x53 };
hs_t hs_psd = { 0x38, 0x42, 0x50, 0x53 };
hs_t hs_mpeg3wot = { 0xFF, 0xFB };
hs_t hs_mpeg3 = { 0x49, 0x44, 0x33 };
hs_t hs_bmp = { 0x42, 0x4D };
hs_t hs_iso = { 0x43, 0x44, 0x30, 0x30, 0x31 };
hs_t hs_fits0 = { 0x53, 0x49, 0x4D, 0x50, 0x4C, 0x45, 0x20, 0x20 };
hs_t hs_fits1 = { 0x3D, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
hs_t hs_fits2 = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
hs_t hs_fits3 = { 0x20, 0x20, 0x20, 0x20, 0x54 };
hs_t hs_flac = { 0x66, 0x4C, 0x61, 0x43 };
hs_t hs_midi = { 0x4D, 0x54, 0x68, 0x64 };
hs_t hs_modoc = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 };
hs_t hs_dex = { 0x64, 0x65, 0x78, 0x0A, 0x30, 0x33, 0x35, 0x00 };
hs_t hs_vmdk = { 0x4B, 0x44, 0x4D };
hs_t hs_crx = { 0x43, 0x72, 0x32, 0x34 };
hs_t hs_fh8 = { 0x41, 0x47, 0x44, 0x33 };
hs_t hs_cwk5 = {
	0x05, 0x07, 0x00, 0x00, 0x42, 0x4F, 0x42, 0x4F,
	0x05, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
hs_t hs_cwk6 = {
	0x06, 0x07, 0xE1, 0x00, 0x42, 0x4F, 0x42, 0x4F,
	0x06, 0x07, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};
hs_t hs_toast0 = { 0x42, 0x52, 0x02, 0x00, 0x00, 0x00 };
hs_t hs_toast1 = { 0x8B, 0x45, 0x52, 0x02, 0x00, 0x00, 0x00 };
hs_t hs_dmg = { 0x78, 0x01, 0x73, 0x0D, 0x62, 0x62, 0x60 };
hs_t hs_xar = { 0x78, 0x61, 0x72, 0x21 };
hs_t hs_dat = { 0x50, 0x4D, 0x4F, 0x43, 0x43, 0x4D, 0x4F, 0x43 };
hs_t hs_nes = { 0x4E, 0x45, 0x53, 0x1A };
hs_t hs_tar0 = { 0x75, 0x73, 0x74, 0x61, 0x72, 0x00, 0x30, 0x30 };
hs_t hs_tar1 = { 0x75, 0x73, 0x74, 0x61, 0x72, 0x20, 0x20, 0x00 };
hs_t hs_mlv = { 0x4D, 0x4C, 0x56, 0x49 };

struct fs_t {
	const unsigned p, l;
	const unsigned const char *s;
	const char const *ext, *desc;
}
fsig[] = {
#define hsig(p, sig, ext, desc) {p,sizeof(sig),sig,ext,desc}
	hsig(0, hs_null, "PIC", "IBM Storyboard bitmap file"),
	hsig(0, hs_null, "PIF", "Windows Program Information File"),
	hsig(0, hs_null, "SEA", "Mac StuffIt Self-Extracting Archive"),
	hsig(0, hs_null, "YTR", "IRIS OCR data file"),
	hsig(11, hs_pdb, "PDB", "PalmPilot Database/Document File"),
	hsig(0, hs_dba, "DBA", "Palm Desktop Calendar Archive"),
	hsig(0, hs_dba2, "DBA", "Palm Desktop To Do Archive"),
	hsig(0, hs_tda, "TDA", "Palm Desktop Calendar Archive"),
	hsig(0, hs_ddf, "...", "Palm Desktop Data File (Access Format)"),
	hsig(0, hs_ico, "ico", "Computer icon"),
	hsig(0, hs_3gpp, "3GPP", "3rd Generation Partnership Project multimedia file"),
	hsig(0, hs_3gpp2, "3GPP2", "3rd Generation Partnership Project multimedia file"),
	hsig(0, hs_mpeg4, "MP4", "MPEG-4 video file"),
	hsig(0, hs_lzw, "Z, TAR.Z", "compressed file using Lempel-Ziv-Welch algorithm"),
	hsig(0, hs_lzh, "Z, TAR.Z", "compressed file using LZH algorithm"),
	hsig(0, hs_bz2, "BZ2", "compressed file using Bzip2 algorithm"),
	hsig(0, hs_gif87a, "GIF", "Image file encoded in the Graphics Interchange Format"),
	hsig(0, hs_gif89a, "GIF", "Image file encoded in the Graphics Interchange Format"),
	hsig(0, hs_tiffle, "TIF, TIFF", "Tagged Image File Format (little endian)"),
	hsig(0, hs_tiffbe, "TIF, TIFF", "Tagged Image File Format (big endian)"),
	hsig(0, hs_cin, "CIN", "Kodak Cineon image"),
	hsig(0, hs_dpxle, "SDPX, XPDS", "SPMTE DPX image (little endian)"),
	hsig(0, hs_dpxbe, "SDPX, XPDS", "SPMTE DPX image (big endian)"),
	hsig(0, hs_exr, "EXR", "OpenEXR image"),
	hsig(0, hs_jpeg, "JPG, JPEG", "Joint Photographic Experts Group image"),
	hsig(0, hs_dosexe, "EXE", "DOS MZ executable file format and its descendants (including NE and PE)"),
	hsig(0, hs_earc, "ZIP, JAR, ODT, ODS, ODP, DOCX, XLSX, PPTX, APK, ODF, OOXML", "empty archive file"),
	hsig(0, hs_earc2, "ZIP, JAR, ODT, ODS, ODP, DOCX, XLSX, PPTX, APK, ODF, OOXML", "empty archive file"),
	hsig(0, hs_sarc, "ZIP, JAR, ODT, ODS, ODP, DOCX, XLSX, PPTX, APK, ODF, OOXML", "spanned archive file"),
	hsig(0, hs_rar, "RAR", "RAR archive version 1.50 onwards"),
	hsig(0, hs_rar5, "RAR", "RAR archive version 5.0 onwards"),
	hsig(0, hs_elf, "ELF", "Executable and Linkable Format"),
	hsig(0, hs_png, "PNG", "Portable Network Graphics image"),
	hsig(0, hs_class, "CLASS", "Java class file, Mach-O Fat binary"),
	hsig(0, hs_utf8, "...", "UTF-8 encoded Unicode byte order mark, commonly seen in text files."),
	hsig(0, hs_mach32, "...", "Mach-O binary (32-bit)"),
	hsig(0x1000, hs_mach32, "...", "Mach-O binary (32-bit)"),
	hsig(0, hs_mach64, "...", "Mach-O binary (64-bit)"),
	hsig(0x1000, hs_mach64, "...", "Mach-O binary (64-bit)"),
	hsig(0, hs_mach32r, "...", "Mach-O binary (reverse byte ordering scheme, 32-bit)"),
	hsig(0x1000, hs_mach32r, "...", "Mach-O binary (reverse byte ordering scheme, 32-bit)"),
	hsig(0, hs_mach64r, "...", "Mach-O binary (reverse byte ordering scheme, 64-bit)"),
	hsig(0x1000, hs_mach64r, "...", "Mach-O binary (reverse byte ordering scheme, 64-bit)"),
	hsig(0, hs_utf16le, "...", "Byte-order mark for text file encoded in little-endian 16-bit Unicode Transfer Format"),
	hsig(0, hs_utf32le, "...", "Byte-order mark for text file encoded in little-endian 32-bit Unicode Transfer Format"),
	hsig(0, hs_ps, "PS", "PostScript document"),
	hsig(0, hs_pdf, "PDF", "Portable Document Format"),
	hsig(0, hs_asf, "ASF, WMA, WMV", "Advanced Systems Format"),
	hsig(0, hs_asf2, "ASF, WMA, WMV", "Advanced Systems Format"),
	hsig(0, hs_sdi, "...", "Microsoft's System Deployment Image disk image format"),
	hsig(0, hs_ogg, "OGG, OGA, OGV", "Open source media container"),
	hsig(0, hs_psd, "PSD", "Adobe's native Photoshop Document file"),
	hsig(0, hs_mpeg3wot, "MP3", "MPEG-1 Layer 3 without ID3 tag or with ID3v1 tag appended at end of file"),
	hsig(0, hs_mpeg3, "MP3", "MPEG-1 Layer 3 file with ID3v2 container"),
	hsig(0, hs_bmp, "BMP, DIB", "bitmap image"),
	hsig(0x8001, hs_iso, "ISO", "ISO9660 CD/DVD image"),
	hsig(0x8801, hs_iso, "ISO", "ISO9660 CD/DVD image"),
	hsig(0x9001, hs_iso, "ISO", "ISO9660 CD/DVD image"),
	hsig(0, hs_fits0, "FITS", "Flexible Image Transport System"),
	hsig(0, hs_fits1, "FITS", "Flexible Image Transport System"),
	hsig(0, hs_fits2, "FITS", "Flexible Image Transport System"),
	hsig(0, hs_fits3, "FITS", "Flexible Image Transport System"),
	hsig(0, hs_flac, "FLAC", "Free Lossless Audio Codec"),
	hsig(0, hs_midi, "MID, MIDI", "Musical Instrument Digital Interface sound file"),
	hsig(0, hs_modoc, "DOC, XLS, PPT", "Microsoft Office document"),
	hsig(0, hs_dex, "DEX", "Dalvik Executable"),
	hsig(0, hs_vmdk, "VMDK", "VMDK"),
	hsig(0, hs_crx, "CRX", "Google Chrome extension or packaged application"),
	hsig(0, hs_fh8, "FH8", "FreeHand 8 document"),
	hsig(0, hs_cwk5, "CWK", "AppleWorks 5 document"),
	hsig(0, hs_cwk6, "CWK", "AppleWorks 6 document"),
	hsig(0, hs_toast0, "TOAST", "Roxio Toast disc image or Apple Disk image"),
	hsig(0, hs_toast1, "TOAST", "Roxio Toast disc image or Apple Disk image"),
	hsig(0, hs_dmg, "DMG", "Apple Disk image"),
	hsig(0, hs_xar, "XAR", "eXtensible ARchive"),
	hsig(0, hs_dat, "DAT", "Windows Files And Settings Transfer Repository"),
	hsig(0, hs_nes, "NES", "Nintendo Entertainment System ROM"),
	hsig(0, hs_tar0, "TAR", "TAR archive"),
	hsig(0, hs_tar1, "TAR", "TAR archive"),
	hsig(0, hs_mlv, "MLV", "Magic Lantern Video"),
#undef hsig
};

/*
todo:

46 4F 52 4D nn nn nn nn 49 4C 42 4D 	FORM....ILBM 	0, any 	ilbm, lbm, ibm, iff 	IFF Interleaved Bitmap Image
46 4F 52 4D nn nn nn nn 38 53 56 58 	FORM....8SVX 	0, any 	8svx, 8sv, svx, snd, iff 	IFF 8-Bit Sampled Voice
46 4F 52 4D nn nn nn nn 41 43 42 4D 	FORM....ACBM 	0, any 	acbm, iff 	Amiga Contiguous Bitmap
46 4F 52 4D nn nn nn nn 41 4E 42 4D 	FORM....ANBM 	0, any 	anbm, iff 	IFF Animated Bitmap
46 4F 52 4D nn nn nn nn 41 4E 49 4D 	FORM....ANIM 	0, any 	anim, iff 	IFF CEL Animation
46 4F 52 4D nn nn nn nn 46 41 58 58 	FORM....FAXX 	0, any 	faxx, fax, iff 	IFF Facsimile Image
46 4F 52 4D nn nn nn nn 46 54 58 54 	FORM....FTXT 	0, any 	ftxt, txt, iff 	IFF Formatted Text
46 4F 52 4D nn nn nn nn 53 4D 55 53 	FORM....SMUS 	0, any 	smus, smu, mus, iff 	IFF Simple Musical Score
46 4F 52 4D nn nn nn nn 43 4D 55 53 	FORM....CMUS 	0, any 	cmus, mus, iff 	IFF Musical Score
46 4F 52 4D nn nn nn nn 59 55 56 4E 	FORM....YUVN 	0, any 	yuvn, yuv, iff 	IFF YUV Image
46 4F 52 4D nn nn nn nn 46 41 4E 54 	FORM....FANT 	0, any 	iff 	Amiga Fantavision Movie
46 4F 52 4D nn nn nn nn 41 49 46 46 	FORM....AIFF 	0, any 	aiff, aif, aifc, snd, iff 	Audio Interchange File Format
52 49 46 46 nn nn nn nn 57 41 56 45 	RIFF....WAVE 	0 	wav 	Waveform Audio File Format
*/

void psigs(void) {
	unsigned i;
	printf("number of formats: %lu\n", (unsigned long) sizeof(fsig) / sizeof(fsig[0]));
	for (i = 0; i < sizeof(fsig) / sizeof(fsig[0]); ++i) {
		struct fs_t fs = fsig[i];
		unsigned j;
		printf("%u, %u, %s, %s\n", fs.p, fs.l, fs.ext, fs.desc);
		for (j = 0; j < fs.l; ++j)
			printf("%02hX ", (unsigned short) fs.s[j]);
		if (fs.l)
			putchar('\n');
	}
}

#define BUFSZ 1024

unsigned char buf[BUFSZ];

void psig(FILE *f, char *name) {
	unsigned long sz;
	unsigned i, m = 0;
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	printf("%s:\n", name);
	debug(printf("length: %lu\n", sz));
	for (i = 0; i < sizeof(fsig) / sizeof(fsig[0]); ++i) {
		struct fs_t fs = fsig[i];
		if (fs.p + fs.l > sz)
			continue;
		debug(printf("checking match for `%s' (%s)\n", fs.ext, fs.desc));
		fseek(f, fs.p, SEEK_SET);
		if (fs.l > BUFSZ) {
			fprintf(stderr, "%u: too long (max. %u)\n", fs.l, (unsigned) BUFSZ);
			continue;
		}
		fread(buf, sizeof(char), BUFSZ, f);
		if (memcmp(fs.s, buf, fs.l) == 0) {
			printf("%s, %s\n", fs.ext, fs.desc);
			++m;
		}
	}
	if (m)
		printf("%s: matches %u %s\n", name, m, m == 1 ? "format" : "formats");
	else
		printf("%s: unknown signature\n", name);
}

int main(int argc, char **argv) {
	int i;
	if (argc < 2) {
		usage(argc, argv);
		psigs();
		return 1;
	}
	for (i = 1; i < argc; ++i) {
		FILE *f = fopen(argv[i], "rb");
		if (!f) {
			perror(argv[i]);
			continue;
		}
		psig(f, argv[i]);
		fclose(f);
	}
	return 0;
}

