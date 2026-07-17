# testsum.awk
#
#
BEGIN {
}

/^#/ { next; }

{
    counts[$2]++
    tests++
}

END {
    printf ("Diagnostics summary:\n");
    printf ("%4d %s\n",counts["made"]+counts["making"],"not run");
    printf ("%4d %s\n",counts["running"],"running");
    printf ("%4d %s\n",counts["passed"],"passed");
    printf ("%4d %s\n",counts["incomplete"],"incomplete");
    printf ("%4d %s\n",counts["***"],"failed");
    printf ("--------------\n");
    printf ("%4d total\n", tests);
}
