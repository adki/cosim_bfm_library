#!/bin/csh -f

if ( -e __init__.pyc  ) /bin/rm -f __init__.pyc
if ( -e cosim_bfm.pyc ) /bin/rm -f cosim_bfm.pyc

foreach F ( * )
    if ( -d $F ) then
    if ( -e $F/Clean.csh ) then
       ( cd $F; ./Clean.csh )
    endif
    endif
end
