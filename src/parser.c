#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char buf[1024 * 1024];

    if (argc != 2) {
        printf("Usage: %s <infile.xml>", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "%s: can't open %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    while ((fgets(buf, sizeof(buf), fp)) != NULL)
        printf(buf);

    fclose(fp);

    exit(EXIT_SUCCESS);
}
