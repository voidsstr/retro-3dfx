#
# Get AGP Information and print it for now...
#

        $Width =  0x1;
        $Hieght = 0x1;

        $Dest = 0x0; # Destination is always memory for these tests

        $HostX = 0x0;
        $HostY = 0x0;
        $HostStride = 0x1f;

        $DestX = 0x0;
        $DestY = 0x0;
        $DestStride =  640;

        $Size = $Hieght * $Width;

        printf("Moving from X: 0x%x , Y: 0x%x, W: 0x%x, H: 0x%x to X: 0x%x, Y: 0x%x  Size: 0x%x\n",
                        $HostX, $HostY, $Width, $Hieght, $DestX, $DestY, $Size );
 
        DataMove ($HostX, $HostY, $HostStride, $Dest, $DestX, $DestY,
                  $DestStride, $Width, $Size);



# ****************************************************
# Move Data from AGP memory to Frame Buffer
# ****************************************************

sub DataMove
{
        $HostX       = $_[0];
        $HostY       = $_[1];
        $HostStride  = $_[2];
        $Dest        = $_[3];
        $DestX       = $_[4];
        $DestY       = $_[5];
        $DestStride  = $_[6];
        $Width       = $_[7];
        $Size        = $_[8];

        $HostAddress = $HostX + ($HostY * $HostStride);
        $DestAddress = $DestX + ($DestY * $DestStride);
	
        printf "AGPREQSIZE 0x%x\n", $Size;                                         
        printf "HOSTADDRLOW 0x%x\n", $HostAddress;                                     
        printf "HOSTADDRHIGH 0x%x\n", (($HostStride << 14) | $Width);                                   
        printf "GRAPHICSADDR 0x%x\n", $DestAddress ;                                     
        printf "GRAPHICSSTRIDE 0x%x\n", $DestStride ;                                  
        printf "MOVECMD 0x%x\n", ($Dest << 3);                                    
}












