clc
clear

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')

pkg load image

%script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_png_file='EAB_image.png'; %target file for animated gif
scaling_factor=1;
color_limit=4; %threshold to reject an image /255
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

AEB_mode_for_Photo('./Pictures/',target_png_file, color_limit) %processes red average

if not(scaling_factor==1)
    im=imread(target_png_file);
    im=imresize(im,scaling_factor,'nearest');
    imwrite(im,target_png_file);
end


disp('End of conversion, enjoy your fancy RGB fusion !')
