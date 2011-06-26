#!/usr/bin/perl -w

# $Id$
# Original Author: John Cataldo aka jodiamonds at imap dot cc
# Modified by Yura Siamashka

# Put the text 'STACKTRACE;' at the beginning of each C++ function.
# This only recognizes functions which look like: text :: text (
# not_close_paren ) { So non-class functions will never be recognized.
# This is really a two line script, turned into a much larger
# program. =)

# All affected files are backed up to name~~ (similar to emacs backup file names).
# You can set BACKUP='' to turn this behavior off.

# This script will first delete all existing STACKTRACE mentions and
# replace them.  Ones with are on the same line are likely to be
# placed onto the next line.

use strict;
{#create a lexical scope, so these variables are accessible to the subroutines (but not global per se)

  ########################################
  ### CONFIGURATION VARIABLES ###
  my( $BACKUP ) = ''; #SET THIS TO '' STOP THIS SCRIPT FROM BACKING UP FILES

  ########################################
  ### CHECK THE ARGUMENTS ###
  #each and every argument should be a filename to do the replacements on, readable and writable
  my( @args ) = @ARGV;
  my( $filename );
  my( $test );
  my( $invalidArguments ) = 0;
  my( $fExists, $fReadable, $fWritable );
  foreach $filename ( @args )
    {
      $fExists = -f $filename;
      $fReadable = -r _;        #using _ means to use the stat() from the previous file stat(), which is better than checking it again
      $fWritable = -w _;

      warn "$filename doesn't exist!" unless $fExists;
      warn "$filename isn't readable by $0!" unless $fReadable;
      warn "$filename isn't writable by $0!" unless $fWritable;

      $invalidArguments = 1 unless ( $fExists && $fReadable && $fWritable );
    }
  die "\n\nAborting $0: One or more filenames aren't valid.  Usage: $0 filename filename ..." if $invalidArguments;


  foreach $filename( @args ) #process each file
    {
      print "Processing $filename\n";

      ########################################
      ### GET ENTIRE FILE AS ONE STRING ###
      #necessary for doing regex on the whole thing
      open( INPUT, "<", $filename ) or die "can't open $filename for input";
      my( @file ) = <INPUT>;	#the raw input
      my( $line );
      my( $blob ) = "";		#wholefile
      foreach $line (@file) {
	$blob .= $line;
      }
      close( INPUT );
      @file = (); #just frees up some memory

      my( $originalFile ) = $blob; #we save a copy of the file just to
                                   #see if it changed.  this isn't
                                   #particularly memory efficient, but
                                   #is the simplest way to see if
                                   #anything actually changed

      ### THE MEAT OF THE FILE: TWO REGEXs ###
      # Pretty much all of the actual changes happen on these two lines:

      # There's a couple weird things going on, like the \s* before
      # STACKTRACE, which are just done to improve readability
      # (i.e. try to put the new STACKTRACE in a place where it's most
      # legible, which I define as on a line by itself at the same
      # indentation as the NEXT line that's already in the function)

      #the ';' after STACKTRACE appears to be optional, but this script will put it in afterwards.
      #using /x at the end allows for commenting and whitespace!

      ##### Delete all existing "STACKTRACE" instances (but only in the exact places where we will put it back in!) #####
      #$blob =~ s/(\s*\w+\s*::\s*\w+[(][^)]*[)]\s*(?:[:][^{]*)?\s*[{])\s*STACKTRACE;?/$1/xg;

      $blob =~ s/\tSTACKTRACE;?\n//xg;
      ##### FIND THE PLACES STACKTRACE SHOULD SHOW UP, AND SHOVE IT IN THERE. #####
      $blob =~ s/(\s*\w+\s*::\s*\w+[(][^)]*[)]\s*(?:[:][^{]*)?\s*[{])(\s*)/${1}\n\tSTACKTRACE;${2}/mg;
      $blob =~ s/(\n(static\s+)?\w+\s+\w+[(][^)]*[)]\s*(?:[:][^{]*)?\s*[{])(\s*)/${1}\n\tSTACKTRACE;${3}/mg;
      #/mg: multiline, since we have the whole file here, and global, meaning replace not just the first instance

      #if the file changed, back it up and spit it out to disk
      if ( $blob ne $originalFile )
	{
	  if ( $BACKUP )
	    {
	      rename $filename, "${filename}~~" or warn "couldn't backup $filename to ${filename}~~\n";
	      print "\tBacked up $filename to ${filename}~~\n";
	    }

	  open( OUTPUT, ">", $filename ) or die "couldn't open $filename for output";
	  print OUTPUT $blob;
	  close OUTPUT;
	  print "\tWrote out new $filename.\n";
	}
    }
}
