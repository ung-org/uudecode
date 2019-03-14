/*
 * UNG's Not GNU
 * 
 * Copyright (c) 2011, Jakob Kaivo <jakob@kaivo.net>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

const char *uudecode_desc = "decode a binary file";
const char *uudecode_inv  = "uudecode [-o file] [file]";

int uudecode (FILE *in, FILE *out)
{
  char bytes = 0;
  char buf[4];
  int nread = 0;

  // FIXME: not checking for \nend\n
  while (!feof (in)) {
    fread (&bytes, sizeof(char), 1, in);
    bytes -= 0x20;
    while (bytes > 0) {
      fread (buf, sizeof(char), 4, in);
      fputc (((buf[0] << 2) | ((buf[1] & 0x30) >> 4)) - 0x20, out);
      fputc ((((buf[1] & 0xf) << 4) | ((buf[2] & 0x3c) >> 2)) - 0x20, out);
      fputc ((((buf[2] & 0x3) << 6) | buf[3]) - 0x20, out);
      bytes -= 3;
    }
  }

  return 0;
}

static const char *b64s =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// FIXME: this is broken, and I don't understand why 
int base64decode (FILE *in, FILE *out)
{
  char bin[3], bout[76];
  int nread = 0;
  int lpos = 0;

  fflush (stdout);
  while (nread += fread (bin, sizeof(char), 3, in)) {
    bout[lpos++] = b64s[bin[0] >> 2]; 
    bout[lpos++] = b64s[((bin[0] & 0x3) << 4) | (bin[1] >> 4)];
    bout[lpos++] = b64s[((bin[1] & 0xf) << 2) | (bin[2] >> 6)]; 
    bout[lpos++] = b64s[bin[2] & 0x3f];
    bin[0] = bin[1] = bin[2] = 0;
    if (nread % 3 > 0)
      bout[lpos] = '=';
    if (nread % 3 > 1)
      bout[lpos-1] = '=';

    if (feof(in) || nread > 54) {
      fwrite (bout, sizeof (char), lpos, stdout);
      fputc ('\n', stdout);
      nread = 0;
      lpos = 0;
    }
  }
  printf ("====\n");

  return 0;
}

int
main(int argc, char **argv)
{
  int b64 = 0;
  int mode = 0644;
  char ofilebuf[255];
  char *ofile;
  FILE *output;
  FILE *input;
  char encoding[16];
  struct stat st;
  int c;

  while ((c = getopt (argc, argv, ":o:")) != -1) {
    switch (c) {
      case 'o':
        ofile = optarg;
        break;
      default:
        return 1;
    }
  }

  if (optind >= argc)
    return 1;

  if (argc > 2)
    input = fopen (argv[argc-1], "r");
  else
    input = stdin;

  if (input == NULL)
    return 1;	// File I/O error

  fscanf (input, "%s %o %s\n", encoding, &mode, ofilebuf);
  // output = fopen (ofile, mode);

  if (ofile != NULL && !strcmp ("-", ofile))
    output = stdout;
  else if (ofile != NULL)
    output = fopen (ofile, "w");
  else
    output = fopen (ofilebuf, "w");

  if (output == NULL)
    return 1;	// File I/O error

  if (b64)
    return base64decode (input, output);

  return uudecode (input, output);
}
