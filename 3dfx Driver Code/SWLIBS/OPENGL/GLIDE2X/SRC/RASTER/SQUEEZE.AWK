#! awk

BEGIN {
    blank = 0;
}

{
    if (NF == 0) {
	blank=1;
    } else {
	if (blank) {
	    print "";
	    blank = 0;
	}
	print $0;
    }
}
