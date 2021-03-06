#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
# include <unistd.h>
#endif
#include "utils.h"

int cgfile, cgbase, cgzone, cgcoord;
int size[9];

#define NUM_SIDE 5

float coord[NUM_SIDE*NUM_SIDE*NUM_SIDE];

int main (int argc, char **argv)
{
    int n, nz, nzones = 250;
    double start, finish;
    char name[33], linkpath[65];
    static char *fname = "zones.cgns";
    static char *linkname = "zones_link.cgns";

    for (n = 0; n < 3; n++) {
        size[n]   = NUM_SIDE;
        size[n+3] = NUM_SIDE - 1;
        size[n+6] = 0;
    }
    if (argc > 1)
        nzones = atoi (argv[1]);
    printf ("number of zones  = %d\n", nzones);

    unlink (fname);
    printf ("creating zones ...");
    fflush (stdout);
    start = elapsed_time ();
    if (cg_open (fname, CG_MODE_WRITE, &cgfile) ||
        cg_base_write (cgfile, "Base", 3, 3, &cgbase))
        cg_error_exit();
    for (nz = 1; nz <= nzones; nz++) {
        sprintf (name, "Zone%d", nz);
        if (cg_zone_write (cgfile, cgbase, name, size, Structured, &cgzone))
            cg_error_exit();
        if (cg_coord_write(cgfile, cgbase, cgzone, RealSingle,
                "CoordinateX", coord, &cgcoord) ||
            cg_coord_write(cgfile, cgbase, cgzone, RealSingle,
                "CoordinateY", coord, &cgcoord) ||
            cg_coord_write(cgfile, cgbase, cgzone, RealSingle,
                "CoordinateZ", coord, &cgcoord))
           cg_error_exit();
    }
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);

    printf ("closing file   ...");
    fflush (stdout);
    start = elapsed_time ();
    if (cg_close(cgfile)) cg_error_exit();
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);
    printf ("file size        = %g Mb\n", file_size(fname));

    printf ("opening file   ...");
    fflush (stdout);
    start = elapsed_time ();
    if (cg_open (fname, CG_MODE_MODIFY, &cgfile)) cg_error_exit();
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);
    cgbase = 1;

    printf ("modifying file ...");
    fflush (stdout);
    start = elapsed_time ();
    for (nz = 1; nz <= nzones; nz++) {
        sprintf (name, "Zone%d", nz);
        if (cg_zone_write (cgfile, cgbase, name, size, Structured, &cgzone))
            cg_error_exit();
        if (cg_coord_write(cgfile, cgbase, cgzone, RealSingle,
                "CoordinateX", coord, &cgcoord) ||
            cg_coord_write(cgfile, cgbase, cgzone, RealSingle,
                "CoordinateY", coord, &cgcoord) ||
            cg_coord_write(cgfile, cgbase, cgzone, RealSingle,
                "CoordinateZ", coord, &cgcoord))
           cg_error_exit();
    }
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);

    printf ("writing file   ...");
    fflush (stdout);
    start = elapsed_time ();
    if (cg_close (cgfile)) cg_error_exit();
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);
    printf ("file size        = %g Mb\n", file_size(fname));

    unlink (linkname);
    printf ("creating link file ...");
    fflush (stdout);
    start = elapsed_time ();
    if (cg_open (linkname, CG_MODE_WRITE, &cgfile) ||
        cg_base_write (cgfile, "Base", 3, 3, &cgbase))
        cg_error_exit();
    for (nz = 1; nz <= nzones; nz++) {
        sprintf (name, "Zone%d", nz);
        sprintf (linkpath, "/Base/Zone%d", nz);
        if (cg_goto (cgfile, cgbase, "end") ||
            cg_link_write (name, fname, linkpath))
            cg_error_exit();
    }
    cg_close (cgfile);
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);
    printf ("file size        = %g Mb\n", file_size(linkname));

    printf ("opening link file  ...");
    fflush (stdout);
    start = elapsed_time ();
    if (cg_open (linkname, CG_MODE_READ, &cgfile)) cg_error_exit();
    finish = elapsed_time ();
    printf (" %g secs\n", finish - start);
    cg_close (cgfile);

    return 0;
}

