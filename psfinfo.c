#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <malloc.h>
#include <unistd.h>

#define PSF1_FONT_MAGIC 0x0436

typedef struct {
    uint16_t magic; // Magic bytes for idnetiifcation.
    uint8_t fontMode; // PSF font mode
    uint8_t characterSize; // PSF character size.
} PSF1_Header;


#define PSF_FONT_MAGIC 0x864ab572

typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF_font;

PSF_font *font;

int main(int argc, char **argv[]) {
    printf("PSF Info by TendingStream73.\n");
    font = (PSF_font *)malloc(sizeof(PSF_font));
    if (argc < 1) {
        printf("Usage: ");
        printf("%s [file]\n", argv[0]);
        free((void *)font);
        return 0;
    }
    int fd = open((const char *)argv[1], O_RDONLY);
    if (fd < 0) {
        perror((const char *)argv[1]);
        goto end;
    }
    size_t size = read(fd, (void *)font, sizeof(PSF_font));
    if (size < 0) {
        perror((const char *)argv[1]);
        goto end;
    }
    if (((PSF1_Header *)font)->magic == PSF1_FONT_MAGIC) {
        printf("Detected font version: PSF1\n");
        printf("fontMode: %u\n", ((PSF1_Header *)font)->fontMode);
        printf("characterSize: %u\n", ((PSF1_Header *)font)->characterSize);
        goto end;
    }
    else if (font->magic == PSF_FONT_MAGIC) {
        printf("Detected font version: PSF\n");
        printf("version(always zero): %u\n", font->version);
        printf("header size: %u\n", font->headersize);
        printf("flags: %u\n", font->flags);
        printf("numglyph(glyph count): %u\n", font->numglyph);
        printf("bytes per glyph: %u\n", font->bytesperglyph);
        printf("height: %u\n", font->height);
        printf("width: %u\n", font->width);
        goto end;
    } else {
        printf("Error: unsuported(or maybe corrupted) file.\n");
    }
end:
    free((void *)font);
    return 0;
}