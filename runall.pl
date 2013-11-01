#!/usr/bin/perl 

$syssize="800";
$jobfile="job100.txt";

for ($a=-1; $a<=1.0; $a+=0.1 )
	{
	for ($b=-1; $b<=1.0; $b+=0.1 )
		{
		for ($g=-1; $g<=1.0; $g+=0.1 )
			{
			#print "$a $b $g\n"
			@args = ( "./sched_sim_04", "$a", "$b", "$g", "$syssize", "$jobfile");
 			system(@args);
			}
		}
	}

