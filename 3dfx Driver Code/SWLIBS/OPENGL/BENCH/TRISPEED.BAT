echo ... > trispeed.log

call glcosnewo.bat
echo "cosmo gl" >> trispeed.log
pause

trispeed -t 4 -P tstrip -a t	>> trispeed.log
trispeed -t 4 -P tstrip -a c >> trispeed.log
trispeed -t 4 -P tstrip -a ct >> trispeed.log

trispeed -t 4 -P tri -a t	>> trispeed.log
trispeed -t 4 -P tri -a c	>> trispeed.log
trispeed -t 4 -P tri -a ct	>> trispeed.log

call glqk.bat
echo "minidriver gl" >> trispeed.log
pause

trispeed -t 4 -P tstrip -a t	>> trispeed.log
trispeed -t 4 -P tstrip -a c >> trispeed.log
trispeed -t 4 -P tstrip -a ct >> trispeed.log

trispeed -t 4 -P tri -a t	>> trispeed.log
trispeed -t 4 -P tri -a c	>> trispeed.log
trispeed -t 4 -P tri -a ct	>> trispeed.log

call glfx.bat
echo "goodin gl" >> trispeed.log
pause

trispeed -t 4 -P tstrip -a t	>> trispeed.log
trispeed -t 4 -P tstrip -a c	>> trispeed.log
trispeed -t 4 -P tstrip -a ct >> trispeed.log

trispeed -t 4 -P tri -a t	>> trispeed.log
trispeed -t 4 -P tri -a c	>> trispeed.log
trispeed -t 4 -P tri -a ct	>> trispeed.log


